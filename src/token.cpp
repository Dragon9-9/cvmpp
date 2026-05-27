#include "cvm++/token.hpp"

namespace cvm {

std::string_view token_type_name(TokenType type) {
    switch (type) {
        case TokenType::Integer:
            return "Integer";
        case TokenType::Identifier:
            return "Identifier";
        case TokenType::True:
            return "True";
        case TokenType::False:
            return "False";
        case TokenType::Let:
            return "Let";
        case TokenType::Fn:
            return "Fn";
        case TokenType::Return:
            return "Return";
        case TokenType::If:
            return "If";
        case TokenType::Else:
            return "Else";
        case TokenType::While:
            return "While";
        case TokenType::Input:
            return "Input";
        case TokenType::Print:
            return "Print";
        case TokenType::Plus:
            return "Plus";
        case TokenType::Minus:
            return "Minus";
        case TokenType::Star:
            return "Star";
        case TokenType::Slash:
            return "Slash";
        case TokenType::Bang:
            return "Bang";
        case TokenType::EqualEqual:
            return "EqualEqual";
        case TokenType::BangEqual:
            return "BangEqual";
        case TokenType::Less:
            return "Less";
        case TokenType::LessEqual:
            return "LessEqual";
        case TokenType::Greater:
            return "Greater";
        case TokenType::GreaterEqual:
            return "GreaterEqual";
        case TokenType::Assign:
            return "Assign";
        case TokenType::LParen:
            return "LParen";
        case TokenType::RParen:
            return "RParen";
        case TokenType::LBrace:
            return "LBrace";
        case TokenType::RBrace:
            return "RBrace";
        case TokenType::Semicolon:
            return "Semicolon";
        case TokenType::Comma:
            return "Comma";
        case TokenType::Eof:
            return "Eof";
        case TokenType::Invalid:
            return "Invalid";
    }
    return "Unknown";
}

bool Token::is_keyword() const {
    switch (type) {
        case TokenType::Let:
        case TokenType::Fn:
        case TokenType::Return:
        case TokenType::If:
        case TokenType::Else:
        case TokenType::While:
        case TokenType::Input:
        case TokenType::Print:
        case TokenType::True:
        case TokenType::False:
            return true;
        default:
            return false;
    }
}

bool Token::is_literal() const {
    return type == TokenType::Integer || type == TokenType::True ||
           type == TokenType::False;
}

}  // namespace cvm
