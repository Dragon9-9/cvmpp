#include "cvm++/lexer.hpp"

#include <cctype>
#include <climits>
#include <cstdint>
#include <limits>
#include <unordered_map>

namespace cvm {
namespace {

const std::unordered_map<std::string, TokenType> kKeywords = {
    {"let", TokenType::Let},     {"fn", TokenType::Fn},
    {"return", TokenType::Return}, {"if", TokenType::If},
    {"else", TokenType::Else},   {"while", TokenType::While},
    {"input", TokenType::Input}, {"print", TokenType::Print},
    {"true", TokenType::True},   {"false", TokenType::False},
};

constexpr std::int64_t kIntMax =
    static_cast<std::int64_t>(std::numeric_limits<std::int64_t>::max());
bool is_identifier_start(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool is_identifier_part(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

}  // namespace

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

bool Lexer::at_end() const { return current_ >= source_.size(); }

char Lexer::peek() const {
    if (at_end()) {
        return '\0';
    }
    return source_[current_];
}

char Lexer::peek_next() const {
    if (current_ + 1 >= source_.size()) {
        return '\0';
    }
    return source_[current_ + 1];
}

char Lexer::advance() {
    if (at_end()) {
        return '\0';
    }
    char c = source_[current_++];
    if (c == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    return c;
}

bool Lexer::match(char expected) {
    if (at_end() || source_[current_] != expected) {
        return false;
    }
    advance();
    return true;
}

SourceLoc Lexer::loc_at(std::size_t index) const {
    SourceLoc loc;
    loc.line = 1;
    loc.column = 1;
    for (std::size_t i = 0; i < index && i < source_.size(); ++i) {
        if (source_[i] == '\n') {
            ++loc.line;
            loc.column = 1;
        } else {
            ++loc.column;
        }
    }
    return loc;
}

void Lexer::error_at(std::size_t index, const std::string& message,
                     const std::string& hint) {
    Diagnostic d;
    d.phase = Phase::Lexer;
    d.severity = Severity::Error;
    d.message = message;
    d.loc = loc_at(index);
    d.hint = hint;
    bag_.push(std::move(d));
}

void Lexer::warn_at(std::size_t index, const std::string& message,
                    const std::string& hint) {
    Diagnostic d;
    d.phase = Phase::Lexer;
    d.severity = Severity::Warning;
    d.message = message;
    d.loc = loc_at(index);
    d.hint = hint;
    bag_.push(std::move(d));
}

void Lexer::skip_whitespace_and_comments() {
    for (;;) {
        char c = peek();
        if (c == ' ' || c == '\r' || c == '\t' || c == '\n') {
            advance();
            continue;
        }
        if (c == '/' && peek_next() == '/') {
            while (!at_end() && peek() != '\n') {
                advance();
            }
            continue;
        }
        break;
    }
}

Token Lexer::make_token(TokenType type, std::size_t lex_start,
                        std::size_t lex_end) {
    Token t;
    t.type = type;
    t.lexeme = source_.substr(lex_start, lex_end - lex_start);
    t.start = loc_at(lex_start);
    t.end = loc_at(lex_end > 0 ? lex_end - 1 : lex_start);
    return t;
}

void Lexer::integer(std::vector<Token>& out) {
    const std::size_t lex_start = current_;
    std::int64_t value = 0;
    bool overflow = false;

    while (std::isdigit(static_cast<unsigned char>(peek()))) {
        int digit = peek() - '0';
        if (value > (kIntMax - digit) / 10) {
            overflow = true;
        } else {
            value = value * 10 + digit;
        }
        advance();
    }

    const std::size_t lex_end = current_;
    const std::string lexeme = source_.substr(lex_start, lex_end - lex_start);

    if (overflow) {
        error_at(lex_start,
                 "integer literal '" + lexeme +
                     "' overflows the maximum representable value (" +
                     std::to_string(kIntMax) + ")",
                 "use a smaller literal or split the computation across steps");
        out.push_back(make_token(TokenType::Invalid, lex_start, lex_end));
        return;
    }

    if (lexeme.size() > 1 && lexeme[0] == '0') {
        warn_at(lex_start, "integer literal has leading zeros",
                "leading zeros are allowed but may be confusing");
    }

    out.push_back(make_token(TokenType::Integer, lex_start, lex_end));
}

void Lexer::identifier_or_keyword(std::vector<Token>& out) {
    const std::size_t lex_start = current_;
    while (is_identifier_part(peek())) {
        advance();
    }
    const std::size_t lex_end = current_;
    const std::string text = source_.substr(lex_start, lex_end - lex_start);

    auto it = kKeywords.find(text);
    if (it != kKeywords.end()) {
        out.push_back(make_token(it->second, lex_start, lex_end));
    } else {
        out.push_back(make_token(TokenType::Identifier, lex_start, lex_end));
    }
}

void Lexer::scan_token(std::vector<Token>& out) {
    skip_whitespace_and_comments();
    start_ = current_;
    if (at_end()) {
        return;
    }

    char c = advance();

    if (c == '\0') {
        error_at(start_, "null byte (\\0) in source",
                "remove embedded null characters from the script file");
        out.push_back(make_token(TokenType::Invalid, start_, current_));
        return;
    }

    switch (c) {
        case '(':
            out.push_back(make_token(TokenType::LParen, start_, current_));
            return;
        case ')':
            out.push_back(make_token(TokenType::RParen, start_, current_));
            return;
        case '{':
            out.push_back(make_token(TokenType::LBrace, start_, current_));
            return;
        case '}':
            out.push_back(make_token(TokenType::RBrace, start_, current_));
            return;
        case ';':
            out.push_back(make_token(TokenType::Semicolon, start_, current_));
            return;
        case '+':
            out.push_back(make_token(TokenType::Plus, start_, current_));
            return;
        case '-':
            out.push_back(make_token(TokenType::Minus, start_, current_));
            return;
        case '*':
            out.push_back(make_token(TokenType::Star, start_, current_));
            return;
        case '/':
            out.push_back(make_token(TokenType::Slash, start_, current_));
            return;
        case '!':
            if (match('=')) {
                out.push_back(make_token(TokenType::BangEqual, start_, current_));
            } else {
                out.push_back(make_token(TokenType::Bang, start_, current_));
            }
            return;
        case '<':
            if (match('=')) {
                out.push_back(make_token(TokenType::LessEqual, start_, current_));
            } else {
                out.push_back(make_token(TokenType::Less, start_, current_));
            }
            return;
        case '>':
            if (match('=')) {
                out.push_back(make_token(TokenType::GreaterEqual, start_, current_));
            } else {
                out.push_back(make_token(TokenType::Greater, start_, current_));
            }
            return;
        case ',':
            out.push_back(make_token(TokenType::Comma, start_, current_));
            return;
        case '=':
            if (match('=')) {
                out.push_back(make_token(TokenType::EqualEqual, start_, current_));
            } else {
                out.push_back(make_token(TokenType::Assign, start_, current_));
            }
            return;
        default:
            break;
    }

    if (std::isdigit(static_cast<unsigned char>(c))) {
        current_ = start_;
        integer(out);
        return;
    }

    if (is_identifier_start(c)) {
        current_ = start_;
        identifier_or_keyword(out);
        return;
    }

    std::string display;
    if (std::iscntrl(static_cast<unsigned char>(c))) {
        display = "\\x" + std::to_string(static_cast<unsigned char>(c));
    } else {
        display = std::string(1, c);
    }

    error_at(start_, "unrecognized symbol '" + display + "'",
             "check for typos or unsupported characters in this script");
    out.push_back(make_token(TokenType::Invalid, start_, current_));
}

LexResult Lexer::tokenize() {
    std::vector<Token> tokens;
    line_ = 1;
    column_ = 1;
    current_ = 0;
    bag_.clear();

    while (!at_end()) {
        scan_token(tokens);
    }

    Token eof;
    eof.type = TokenType::Eof;
    eof.lexeme = "";
    eof.start = loc_at(source_.size());
    eof.end = eof.start;
    tokens.push_back(eof);

    LexResult result;
    result.tokens = std::move(tokens);
    result.diagnostics = std::move(bag_);
    return result;
}

}  // namespace cvm
