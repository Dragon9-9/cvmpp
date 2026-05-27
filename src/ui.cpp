#include "cvm++/ui.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unistd.h>

namespace cvm::ui {
namespace {

std::string colorize(const char* color, const std::string& text) {
    if (!stdout_is_tty()) {
        return text;
    }
    return std::string(color) + text + ansi::reset;
}

std::size_t max_token_type_width(const std::vector<Token>& tokens) {
    std::size_t w = std::string("Type").size();
    for (const auto& t : tokens) {
        w = std::max(w, token_type_name(t.type).size());
    }
    return w;
}

std::size_t max_lexeme_width(const std::vector<Token>& tokens) {
    std::size_t w = std::string("Lexeme").size();
    for (const auto& t : tokens) {
        w = std::max(w, t.lexeme.size());
    }
    return w;
}

void print_horizontal_rule(std::size_t width) {
    std::cout << colorize(ansi::dim, std::string(width, '-')) << '\n';
}

}  // namespace

bool stdout_is_tty() {
#ifdef _WIN32
    return false;
#else
    return isatty(STDOUT_FILENO) != 0;
#endif
}

void print_logo() {
    const char* logo = R"(
   ██████╗██╗   ██╗███╗   ███╗+++
  ██╔════╝██║   ██║████╗ ████║
  ██║     ██║   ██║██╔████╔██║
  ██║     ╚██╗ ██╔╝██║╚██╔╝██║
  ╚██████╗ ╚████╔╝ ██║ ╚═╝ ██║
   ╚═════╝  ╚═══╝  ╚═╝     ╚═╝
)";
    std::cout << colorize(ansi::bright_cyan, logo) << '\n';
    std::cout << colorize(ansi::dim,
                          "  Stack VM & Compiler  ·  Personal Project  ·  v1.0\n")
              << '\n';
}

void print_welcome(bool debug_mode) {
    print_logo();
    std::cout << colorize(ansi::white, "  Welcome to CVM++ REPL")
              << (debug_mode ? colorize(ansi::bright_yellow, "  [debug ON]") : "")
              << "\n\n";
    std::cout << colorize(ansi::dim,
                          "  :help for commands  ·  :quit to exit  ·  { } opens multiline input\n\n");
}

void print_prompt() {
    std::cout << colorize(ansi::bright_green, "cvm++") << colorize(ansi::dim, " > ");
    std::cout.flush();
}

void print_continuation_prompt(int brace_depth) {
    std::cout << colorize(ansi::bright_green, "cvm++")
              << colorize(ansi::yellow, " {" + std::to_string(brace_depth) + "}")
              << colorize(ansi::dim, " > ");
    std::cout.flush();
}

void print_version() {
    std::cout << colorize(ansi::bright_cyan, "CVM++") << colorize(ansi::dim, " v1.0.0")
              << "  — scripting language, bytecode compiler, stack VM\n";
}

void clear_screen() {
    if (stdout_is_tty()) {
        std::cout << "\033[2J\033[H";
    }
}

void print_success(const std::string& message) {
    std::cout << colorize(ansi::cyan, "✓ ") << message << '\n';
}

void print_info(const std::string& message) {
    std::cout << colorize(ansi::blue, "ℹ ") << message << '\n';
}

void print_diagnostics(const DiagnosticBag& bag, const std::string& source) {
    for (const auto& d : bag.all()) {
        const char* color = ansi::bright_yellow;
        const char* icon = "⚠";
        if (d.severity == Severity::Error) {
            color = ansi::bright_red;
            icon = "✗";
        } else if (d.severity == Severity::Note) {
            color = ansi::dim;
            icon = "·";
        }

        std::ostringstream header;
        header << icon << ' ' << colorize(color, phase_label(d.phase)) << ' '
               << colorize(color, severity_label(d.severity)) << " at "
               << d.loc.to_string() << ": " << d.message;

        std::cout << header.str() << '\n';

        if (!d.hint.empty()) {
            std::cout << colorize(ansi::dim, "    hint: ") << d.hint << '\n';
        }

        if (!source.empty() && d.loc.line > 0) {
            std::size_t line_idx = 0;
            std::size_t current_line = 1;
            std::string line_text;
            while (line_idx < source.size() && current_line <= d.loc.line) {
                if (source[line_idx] == '\n') {
                    if (current_line == d.loc.line) {
                        break;
                    }
                    line_text.clear();
                    ++current_line;
                } else {
                    if (current_line == d.loc.line) {
                        line_text += source[line_idx];
                    }
                }
                ++line_idx;
            }

            if (!line_text.empty()) {
                std::cout << colorize(ansi::dim, "    | ") << line_text << '\n';
                std::cout << colorize(ansi::dim, "    | ");
                std::size_t col = d.loc.column > 0 ? d.loc.column - 1 : 0;
                for (std::size_t i = 0; i < col; ++i) {
                    std::cout << ' ';
                }
                std::cout << colorize(color, "^") << '\n';
            }
        }
    }
}

void print_token_table(const std::vector<Token>& tokens) {
    const std::size_t idx_w = 4;
    const std::size_t type_w = max_token_type_width(tokens);
    const std::size_t lex_w = std::max<std::size_t>(6, max_lexeme_width(tokens));
    const std::size_t loc_w = 11;

    const std::size_t total_w =
        idx_w + type_w + lex_w + loc_w + 9;

    std::cout << '\n'
              << colorize(ansi::magenta, "  ╭─ Token stream ")
              << colorize(ansi::dim, std::string(total_w > 18 ? total_w - 18 : 0, '-'))
              << colorize(ansi::magenta, "╮\n");

    auto header_cell = [](const char* color, std::size_t w,
                          const std::string& text) {
        std::cout << colorize(color, "  │ ")
                  << std::left << std::setw(static_cast<int>(w)) << text;
    };

    header_cell(ansi::bold, idx_w, "#");
    header_cell(ansi::bold, type_w, "Type");
    header_cell(ansi::bold, lex_w, "Lexeme");
    header_cell(ansi::bold, loc_w, "Location");
    std::cout << colorize(ansi::magenta, "│\n");

    print_horizontal_rule(total_w);

    std::size_t index = 0;
    for (const auto& t : tokens) {
        std::string lex_display = t.lexeme;
        if (t.type == TokenType::Eof) {
            lex_display = "<eof>";
        } else if (lex_display.empty() && t.is_literal()) {
            lex_display = t.type == TokenType::True ? "true" : "false";
        }

        const char* type_color = ansi::white;
        if (t.type == TokenType::Invalid) {
            type_color = ansi::bright_red;
        } else if (t.is_keyword()) {
            type_color = ansi::yellow;
        } else if (t.is_literal()) {
            type_color = ansi::cyan;
        }

        std::ostringstream loc;
        loc << t.start.to_string();

        const std::string type_name{token_type_name(t.type)};
        std::cout << colorize(ansi::magenta, "  │ ") << std::right
                  << std::setw(static_cast<int>(idx_w)) << index << std::left
                  << ' ';
        std::cout << colorize(type_color, type_name);
        if (type_name.size() < type_w) {
            std::cout << std::string(type_w - type_name.size(), ' ');
        }
        std::cout << ' ';
        std::cout << std::setw(static_cast<int>(lex_w))
                  << (lex_display.empty() ? "·" : lex_display) << ' ';
        std::cout << std::setw(static_cast<int>(loc_w)) << loc.str();
        std::cout << colorize(ansi::magenta, "│\n");
        ++index;
    }

    std::cout << colorize(ansi::magenta, "  ╰")
              << colorize(ansi::dim, std::string(total_w > 2 ? total_w - 2 : 0, '-'))
              << colorize(ansi::magenta, "╯\n\n");
}

void print_bytecode_table(const BytecodeChunk& chunk) {
    const auto instrs = disassemble(chunk);

    std::size_t offset_w = 6;
    std::size_t opcode_w = 14;
    std::size_t operand_w = 24;
    for (const auto& row : instrs) {
        offset_w = std::max(offset_w, std::to_string(row.offset).size());
        opcode_w = std::max(opcode_w, opcode_name(row.opcode).size());
        operand_w = std::max(operand_w, row.operands.size());
    }

    const std::size_t total_w = offset_w + opcode_w + operand_w + 9;

    std::cout << '\n'
              << colorize(ansi::blue, "  ╭─ Bytecode ")
              << colorize(ansi::dim, std::string(total_w > 14 ? total_w - 14 : 0, '-'))
              << colorize(ansi::blue, "╮\n");

    auto header = [&](const char* color, std::size_t w, const std::string& text) {
        std::cout << colorize(color, "  │ ") << std::left
                  << std::setw(static_cast<int>(w)) << text;
    };

    header(ansi::bold, offset_w, "Offset");
    header(ansi::bold, opcode_w, "Opcode");
    header(ansi::bold, operand_w, "Operands");
    std::cout << colorize(ansi::blue, "│\n");
    print_horizontal_rule(total_w);

    for (const auto& row : instrs) {
        const std::string op_name{opcode_name(row.opcode)};
        std::ostringstream off;
        off << row.offset;

        std::cout << colorize(ansi::blue, "  │ ") << std::right
                  << std::setw(static_cast<int>(offset_w)) << off.str() << std::left
                  << ' ';
        std::cout << colorize(ansi::yellow, op_name);
        if (op_name.size() < opcode_w) {
            std::cout << std::string(opcode_w - op_name.size(), ' ');
        }
        std::cout << ' ';
        std::cout << std::setw(static_cast<int>(operand_w))
                  << (row.operands.empty() ? "·" : row.operands);
        std::cout << colorize(ansi::blue, "│\n");
    }

    if (!chunk.names.empty()) {
        std::cout << colorize(ansi::blue, "  │ ") << colorize(ansi::dim, "Name pool: ");
        for (std::size_t i = 0; i < chunk.names.size(); ++i) {
            if (i > 0) {
                std::cout << colorize(ansi::dim, ", ");
            }
            std::cout << '#' << i << '=' << chunk.names[i];
        }
        std::cout << '\n';
    }

    std::cout << colorize(ansi::blue, "  ╰")
              << colorize(ansi::dim, std::string(total_w > 2 ? total_w - 2 : 0, '-'))
              << colorize(ansi::blue, "╯\n\n");
}

void print_runtime_output(const std::vector<std::string>& lines) {
    if (lines.empty()) {
        return;
    }
    std::cout << colorize(ansi::bright_cyan, "\n  Program output\n");
    for (const auto& line : lines) {
        std::cout << colorize(ansi::cyan, "  » ") << line << '\n';
    }
    std::cout << '\n';
}

void print_help() {
    std::cout << colorize(ansi::bold, "\n  CVM++ REPL commands\n\n");
    std::cout << colorize(ansi::green, "  :help") << " / help          Show this help\n";
    std::cout << colorize(ansi::green, "  :quit") << " / quit          Exit (Ctrl+D)\n";
    std::cout << colorize(ansi::green, "  :debug") << " / debug         Toggle token / AST / bytecode\n";
    std::cout << colorize(ansi::green, "  :run <f>") << " / run <f>       Run a .cvm file\n";
    std::cout << colorize(ansi::green, "  :disasm") << " / disasm <f>   Show bytecode (last run or file)\n";
    std::cout << colorize(ansi::green, "  :clear") << "                 Clear the screen\n";
    std::cout << colorize(ansi::green, "  :version") << "               Show version\n\n";
    std::cout << colorize(ansi::dim, "  Multiline: type blocks with { } — prompt shows {n} until closed\n");
    std::cout << colorize(ansi::dim,
                          "  One line:  let x = 42; print x; run examples/hello.cvm\n\n");
    std::cout << colorize(ansi::dim, "  CLI:  cvmpp script.cvm   cvmpp -d -q script.cvm   cvmpp --help\n\n");
}

}  // namespace cvm::ui
