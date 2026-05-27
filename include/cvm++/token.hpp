#pragma once

#include "cvm++/source_loc.hpp"

#include <string>
#include <string_view>

namespace cvm {

enum class TokenType {
    Integer,
    Identifier,
    True,
    False,

    Let,
    Fn,
    Return,
    If,
    Else,
    While,
    Input,
    Print,

    Plus,
    Minus,
    Star,
    Slash,
    Bang,
    EqualEqual,
    BangEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Assign,

    LParen,
    RParen,
    LBrace,
    RBrace,
    Semicolon,
    Comma,

    Eof,
    Invalid,
};

std::string_view token_type_name(TokenType type);

struct Token {
    TokenType type{TokenType::Invalid};
    std::string lexeme;
    SourceLoc start;
    SourceLoc end;

    bool is_keyword() const;
    bool is_literal() const;
};

}  // namespace cvm
