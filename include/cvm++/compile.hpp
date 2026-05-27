#pragma once

#include "cvm++/ast.hpp"
#include "cvm++/bytecode.hpp"
#include "cvm++/compiler.hpp"
#include "cvm++/diagnostic.hpp"
#include "cvm++/vm.hpp"
#include "cvm++/lexer.hpp"
#include "cvm++/parser.hpp"
#include "cvm++/token.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace cvm {

struct FrontEndResult {
    std::string source;
    LexResult lex;
    ParseResult parse;
    CompileResult compile;
    VmResult vm;
    std::vector<Token> tokens;  // retained for debug output after parse

    bool ok() const {
        return lex.ok() && parse.ok() && compile.ok() && vm.ok();
    }
};

FrontEndResult compile_frontend(std::string source);
FrontEndResult compile_and_run(std::string source, std::istream& input = std::cin,
                               std::ostream& output = std::cout);

}  // namespace cvm
