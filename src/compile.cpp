#include "cvm++/compile.hpp"

namespace cvm {

FrontEndResult compile_frontend(std::string source) {
    FrontEndResult result;
    result.source = source;

    Lexer lexer(result.source);
    result.lex = lexer.tokenize();
    if (!result.lex.ok()) {
        result.parse.diagnostics = result.lex.diagnostics;
        return result;
    }

    result.tokens = result.lex.tokens;
    Parser parser(std::move(result.lex.tokens), result.source);
    result.parse = parser.parse();

    for (const auto& d : result.lex.diagnostics.all()) {
        if (d.severity == Severity::Warning) {
            result.parse.diagnostics.push(d);
        }
    }

    if (!result.parse.ok() || !result.parse.program) {
        return result;
    }

    Compiler compiler(*result.parse.program);
    result.compile = compiler.compile();

    return result;
}

FrontEndResult compile_and_run(std::string source, std::istream& input,
                               std::ostream& output) {
    FrontEndResult result = compile_frontend(std::move(source));
    if (!result.compile.ok()) {
        return result;
    }
    result.vm = execute(result.compile.chunk, nullptr, input, output);
    return result;
}

}  // namespace cvm
