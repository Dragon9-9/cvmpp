#pragma once

#include "cvm++/ast.hpp"
#include "cvm++/diagnostic.hpp"
#include "cvm++/token.hpp"

#include <memory>
#include <string>
#include <vector>

namespace cvm {

struct ParseResult {
    std::unique_ptr<Program> program;
    DiagnosticBag diagnostics;

    bool ok() const { return !diagnostics.has_errors() && program != nullptr; }
};

class Parser {
public:
    Parser(std::vector<Token> tokens, std::string source);

    ParseResult parse();

private:
    std::vector<Token> tokens_;
    std::string source_;
    std::size_t current_{0};
    DiagnosticBag bag_;
    bool panic_mode_{false};

    bool is_at_end() const;
    const Token& peek() const;
    const Token& peek_next() const;
    const Token& previous() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types);

    void error(const Token& token, const std::string& message,
               const std::string& hint = {});
    void synchronize();

    std::unique_ptr<Program> program();
    std::unique_ptr<FunctionDecl> function_declaration();
    StmtPtr declaration();
    StmtPtr statement();
    StmtPtr let_declaration();
    StmtPtr return_statement();
    StmtPtr expression_statement();
    StmtPtr assignment_statement();
    StmtPtr print_statement();
    StmtPtr if_statement();
    StmtPtr while_statement();
    StmtPtr block_statement();

    ExprPtr expression();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr primary();
    ExprPtr finish_call(const std::string& name, const SourceLoc& loc);

    StmtPtr finish_block(std::vector<StmtPtr> statements, const SourceLoc& start);

    void consume(TokenType type, const std::string& message,
                 const std::string& hint = {});
    bool consume_closing(TokenType open, TokenType close,
                         const std::string& what,
                         const std::string& hint = {});
};

}  // namespace cvm
