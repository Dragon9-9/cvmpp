#pragma once

#include "cvm++/diagnostic.hpp"
#include "cvm++/token.hpp"

#include <string>
#include <vector>

namespace cvm {

struct LexResult {
    std::vector<Token> tokens;
    DiagnosticBag diagnostics;

    bool ok() const { return !diagnostics.has_errors(); }
};

class Lexer {
public:
    explicit Lexer(std::string source);

    LexResult tokenize();

private:
    std::string source_;
    std::size_t start_{0};
    std::size_t current_{0};
    std::size_t line_{1};
    std::size_t column_{1};
    DiagnosticBag bag_;

    bool at_end() const;
    char peek() const;
    char peek_next() const;
    char advance();
    bool match(char expected);
    void skip_whitespace_and_comments();

    SourceLoc loc_at(std::size_t index) const;
    void error_at(std::size_t index, const std::string& message,
                    const std::string& hint = {});
    void warn_at(std::size_t index, const std::string& message,
                 const std::string& hint = {});

    void scan_token(std::vector<Token>& out);
    void integer(std::vector<Token>& out);
    void identifier_or_keyword(std::vector<Token>& out);

    Token make_token(TokenType type, std::size_t lex_start, std::size_t lex_end);
};

}  // namespace cvm
