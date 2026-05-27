#include "cvm++/compile.hpp"
#include "cvm++/runtime_options.hpp"
#include "cvm++/ui.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

cvm::RuntimeOptions g_opts;
bool show_ast_next = false;
bool show_disasm_next = false;

std::optional<cvm::BytecodeChunk> g_last_chunk;
cvm::VmSession g_repl_session;
bool g_use_repl_session = false;

int g_brace_depth = 0;
std::string g_multiline_buffer;

std::string trim(std::string s) {
    const auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    return s;
}

int count_brace_delta(const std::string& text) {
    int delta = 0;
    for (char ch : text) {
        if (ch == '{') {
            ++delta;
        } else if (ch == '}') {
            --delta;
        }
    }
    return delta;
}

bool is_repl_command(const std::string& segment) {
    const std::string s = trim(segment);
    if (s.empty()) {
        return false;
    }
    if (s[0] == ':') {
        return true;
    }
    static const char* kAliases[] = {"help",   "quit",  "exit",   "debug",
                                     "tokens", "ast",   "run",    "disasm",
                                     "clear",  "version"};
    for (const char* alias : kAliases) {
        const std::string a(alias);
        if (s == a || s.rfind(a + " ", 0) == 0) {
            return true;
        }
    }
    return false;
}

std::string normalize_command(std::string segment) {
    segment = trim(segment);
    if (segment.empty() || segment[0] == ':') {
        return segment;
    }
    static const char* kAliases[] = {"help",   "quit",  "exit",   "debug",
                                     "tokens", "ast",   "run",    "disasm",
                                     "clear",  "version"};
    for (const char* alias : kAliases) {
        const std::string a(alias);
        if (segment == a || segment.rfind(a + " ", 0) == 0) {
            return ":" + segment;
        }
    }
    return segment;
}

std::vector<std::string> split_repl_input(const std::string& line) {
    std::vector<std::string> segments;
    std::string current;

    auto flush = [&]() {
        const std::string piece = trim(current);
        if (!piece.empty()) {
            segments.push_back(piece);
        }
        current.clear();
    };

    for (char ch : line) {
        if (ch == ';') {
            current += ch;
            flush();
        } else if (ch == '\n') {
            flush();
        } else {
            current += ch;
        }
    }
    flush();
    return segments;
}

int print_frontend_result(cvm::FrontEndResult& result) {
    cvm::DiagnosticBag combined;
    if (!result.lex.ok()) {
        for (const auto& d : result.lex.diagnostics.all()) {
            combined.push(d);
        }
    } else {
        for (const auto& d : result.lex.diagnostics.all()) {
            if (d.severity == cvm::Severity::Warning) {
                combined.push(d);
            }
        }
        for (const auto& d : result.parse.diagnostics.all()) {
            combined.push(d);
        }
        if (result.parse.ok()) {
            for (const auto& d : result.compile.diagnostics.all()) {
                combined.push(d);
            }
        }
    }
    cvm::ui::print_diagnostics(combined, result.source);

    if (!result.lex.ok() || !result.parse.ok()) {
        return 1;
    }

    const std::size_t stmt_count =
        result.parse.program ? result.parse.program->statements.size() : 0;

    if (!g_opts.quiet) {
        cvm::ui::print_success("Parse succeeded (" + std::to_string(stmt_count) +
                               " top-level statement(s))");
    }

    if (!result.compile.ok()) {
        return 1;
    }

    g_last_chunk = result.compile.chunk;

    if (!g_opts.quiet) {
        cvm::ui::print_success("Bytecode compiled (" +
                               std::to_string(result.compile.chunk.size()) +
                               " bytes, " +
                               std::to_string(result.compile.chunk.names.size()) +
                               " name(s))");
    }

    const bool show_debug = g_opts.debug;
    if (show_debug) {
        cvm::ui::print_token_table(result.tokens);
    }
    if ((show_debug || show_ast_next) && result.parse.program) {
        cvm::ui::print_ast_tree(*result.parse.program);
    }
    if (show_debug || show_disasm_next) {
        cvm::ui::print_bytecode_table(result.compile.chunk);
    }
    show_ast_next = false;
    show_disasm_next = false;

    if (g_opts.compile_only) {
        if (!g_opts.quiet) {
            cvm::ui::print_info("Compile-only mode — VM skipped.");
        }
        return 0;
    }

    result.vm = cvm::execute(result.compile.chunk,
                           g_use_repl_session ? &g_repl_session : nullptr);
    for (const auto& d : result.vm.diagnostics.all()) {
        cvm::DiagnosticBag vm_only;
        vm_only.push(d);
        cvm::ui::print_diagnostics(vm_only, result.source);
    }

    if (!result.vm.ok()) {
        return 2;
    }

    if (!g_opts.quiet) {
        cvm::ui::print_success("VM execution completed");
    }
    cvm::ui::print_runtime_output(result.vm.output);
    return 0;
}

int run_source(const std::string& source) {
    if (source.empty()) {
        return 0;
    }
    cvm::FrontEndResult result = cvm::compile_frontend(source);
    return print_frontend_result(result);
}

void disasm_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        cvm::DiagnosticBag bag;
        cvm::Diagnostic d;
        d.phase = cvm::Phase::Repl;
        d.severity = cvm::Severity::Error;
        d.message = "could not open file '" + path + "'";
        d.hint = "usage: :disasm path/to/script.cvm";
        bag.push(std::move(d));
        cvm::ui::print_diagnostics(bag, "");
        return;
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    const bool prev_compile_only = g_opts.compile_only;
    const bool prev_quiet = g_opts.quiet;
    g_opts.compile_only = true;
    g_opts.quiet = true;
    show_disasm_next = true;

    run_source(buffer.str());

    g_opts.compile_only = prev_compile_only;
    g_opts.quiet = prev_quiet;
}

int run_file(const std::string& path, bool repl_mode = false) {
    g_use_repl_session = repl_mode;
    std::ifstream in(path);
    if (!in) {
        cvm::DiagnosticBag bag;
        cvm::Diagnostic d;
        d.phase = cvm::Phase::Repl;
        d.severity = cvm::Severity::Error;
        d.message = "could not open file '" + path + "'";
        d.hint = "check the path and permissions; usage: :run path/to/script.cvm";
        bag.push(std::move(d));
        cvm::ui::print_diagnostics(bag, "");
        return 1;
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    if (!g_opts.quiet) {
        cvm::ui::print_info("Running " + path);
    }
    return run_source(buffer.str());
}

void handle_command(const std::string& line) {
    const std::string cmd = trim(line);

    if (cmd == ":quit" || cmd == ":exit") {
        cvm::ui::print_info("Goodbye!");
        std::exit(0);
    }
    if (cmd == ":help") {
        cvm::ui::print_help();
        return;
    }
    if (cmd == ":version") {
        cvm::ui::print_version();
        return;
    }
    if (cmd == ":clear") {
        cvm::ui::clear_screen();
        return;
    }
    if (cmd == ":debug") {
        g_opts.debug = !g_opts.debug;
        cvm::ui::print_success(std::string("Debug mode ") +
                               (g_opts.debug ? "enabled" : "disabled"));
        return;
    }
    if (cmd == ":tokens") {
        g_opts.debug = true;
        cvm::ui::print_info("Token table enabled for the next input.");
        return;
    }
    if (cmd == ":ast") {
        show_ast_next = true;
        cvm::ui::print_info("AST tree enabled for the next input.");
        return;
    }
    if (cmd == ":disasm") {
        if (g_last_chunk) {
            cvm::ui::print_bytecode_table(*g_last_chunk);
        } else {
            cvm::DiagnosticBag bag;
            cvm::Diagnostic d;
            d.phase = cvm::Phase::Repl;
            d.severity = cvm::Severity::Error;
            d.message = "no bytecode to disassemble yet";
            d.hint = "run a script first, or use :disasm path/to/script.cvm";
            bag.push(std::move(d));
            cvm::ui::print_diagnostics(bag, "");
        }
        return;
    }
    if (cmd.rfind(":disasm ", 0) == 0) {
        const std::string path = trim(cmd.substr(8));
        if (path.empty()) {
            cvm::DiagnosticBag bag;
            cvm::Diagnostic d;
            d.phase = cvm::Phase::Repl;
            d.severity = cvm::Severity::Error;
            d.message = "missing file path for :disasm";
            d.hint = "usage: :disasm path/to/script.cvm";
            bag.push(std::move(d));
            cvm::ui::print_diagnostics(bag, "");
            return;
        }
        disasm_file(path);
        return;
    }
    if (cmd.rfind(":run ", 0) == 0) {
        const std::string path = trim(cmd.substr(5));
        if (path.empty()) {
            cvm::DiagnosticBag bag;
            cvm::Diagnostic d;
            d.phase = cvm::Phase::Repl;
            d.severity = cvm::Severity::Error;
            d.message = "missing file path for :run";
            d.hint = "usage: :run path/to/script.cvm  (or: run path/to/script.cvm)";
            bag.push(std::move(d));
            cvm::ui::print_diagnostics(bag, "");
            return;
        }
        run_file(path, true);
        return;
    }

    cvm::DiagnosticBag bag;
    cvm::Diagnostic d;
    d.phase = cvm::Phase::Repl;
    d.severity = cvm::Severity::Error;
    d.message = "unknown command '" + cmd + "'";
    d.hint = "type :help (or help) for available commands";
    bag.push(std::move(d));
    cvm::ui::print_diagnostics(bag, "");
}

void process_repl_line(const std::string& raw_line) {
    const std::vector<std::string> segments = split_repl_input(raw_line);
    if (segments.empty()) {
        return;
    }

    for (const std::string& segment : segments) {
        if (is_repl_command(segment)) {
            handle_command(normalize_command(segment));
        } else {
            run_source(segment);
        }
    }
}

void submit_multiline_buffer() {
    if (!g_multiline_buffer.empty()) {
        run_source(g_multiline_buffer);
    }
    g_multiline_buffer.clear();
    g_brace_depth = 0;
}

void handle_repl_input(const std::string& line) {
    if (g_brace_depth > 0) {
        g_multiline_buffer += line;
        g_multiline_buffer += '\n';
        g_brace_depth += count_brace_delta(line);
        if (g_brace_depth <= 0) {
            g_brace_depth = 0;
            submit_multiline_buffer();
        }
        return;
    }

    const int delta = count_brace_delta(line);
    if (delta > 0) {
        g_multiline_buffer = line;
        g_multiline_buffer += '\n';
        g_brace_depth = delta;
        return;
    }

    process_repl_line(line);
}

void run_repl() {
    g_use_repl_session = true;
    cvm::ui::print_welcome(g_opts.debug);

    for (;;) {
        if (g_brace_depth > 0) {
            cvm::ui::print_continuation_prompt(g_brace_depth);
        } else {
            cvm::ui::print_prompt();
        }

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            if (g_brace_depth > 0) {
                cvm::ui::print_info("Multiline input cancelled.");
                g_multiline_buffer.clear();
                g_brace_depth = 0;
            } else {
                cvm::ui::print_info("Goodbye!");
            }
            break;
        }

        line = trim(line);
        if (line.empty() && g_brace_depth == 0) {
            continue;
        }

        if (g_brace_depth == 0 && is_repl_command(line)) {
            handle_command(normalize_command(line));
            continue;
        }

        handle_repl_input(line);
    }
}

void print_cli_help() {
    std::cout << "CVM++ — stack VM and custom compiler\n\n";
    std::cout << "Usage:\n";
    std::cout << "  cvmpp [options]                  Interactive REPL\n";
    std::cout << "  cvmpp [options] script.cvm       Run a script file\n\n";
    std::cout << "Options:\n";
    std::cout << "  -d, --debug         Show tokens, AST, and bytecode\n";
    std::cout << "  -c, --compile-only  Compile without running the VM\n";
    std::cout << "  -q, --quiet         Only errors and program output\n";
    std::cout << "  -h, --help          Show this help\n";
    std::cout << "  -v, --version       Show version\n\n";
    std::cout << "Exit codes (file mode): 0 ok, 1 compile error, 2 runtime error\n";
}

bool parse_cli(int argc, char* argv[], std::string& script_path) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--debug" || arg == "-d") {
            g_opts.debug = true;
        } else if (arg == "--compile-only" || arg == "-c") {
            g_opts.compile_only = true;
        } else if (arg == "--quiet" || arg == "-q") {
            g_opts.quiet = true;
        } else if (arg == "--help" || arg == "-h") {
            print_cli_help();
            return false;
        } else if (arg == "--version" || arg == "-v") {
            cvm::ui::print_version();
            return false;
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << '\n';
            print_cli_help();
            return false;
        } else if (script_path.empty()) {
            script_path = arg;
        } else {
            std::cerr << "Unexpected argument: " << arg << '\n';
            print_cli_help();
            return false;
        }
    }
    return true;
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string script_path;
    if (!parse_cli(argc, argv, script_path)) {
        return 0;
    }

    if (!script_path.empty()) {
        g_use_repl_session = false;
        return run_file(script_path, false);
    }

    run_repl();
    return 0;
}
