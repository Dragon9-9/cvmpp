#include "cvm++/parser.hpp"

#include <cstdlib>
#include <utility>

namespace cvm {
namespace {

bool is_invalid(const Token& t) { return t.type == TokenType::Invalid; }

}  // namespace

Parser::Parser(std::vector<Token> tokens, std::string source)
    : tokens_(std::move(tokens)), source_(std::move(source)) {}

bool Parser::is_at_end() const { return peek().type == TokenType::Eof; }

const Token& Parser::peek() const { return tokens_[current_]; }

const Token& Parser::peek_next() const {
    if (current_ + 1 >= tokens_.size()) {
        return tokens_.back();
    }
    return tokens_[current_ + 1];
}

const Token& Parser::previous() const { return tokens_[current_ - 1]; }

const Token& Parser::advance() {
    if (!is_at_end()) {
        ++current_;
    }
    return previous();
}

bool Parser::check(TokenType type) const {
    if (is_at_end()) {
        return false;
    }
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (!check(type)) {
        return false;
    }
    advance();
    return true;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

void Parser::error(const Token& token, const std::string& message,
                   const std::string& hint) {
    if (panic_mode_) {
        return;
    }
    panic_mode_ = true;

    Diagnostic d;
    d.phase = Phase::Parser;
    d.severity = Severity::Error;
    d.message = message;
    d.loc = token.start;
    d.hint = hint;
    bag_.push(std::move(d));
}

void Parser::synchronize() {
    panic_mode_ = false;
    while (!is_at_end()) {
        if (previous().type == TokenType::Semicolon) {
            return;
        }
        switch (peek().type) {
            case TokenType::Let:
            case TokenType::Fn:
            case TokenType::Return:
            case TokenType::If:
            case TokenType::While:
            case TokenType::Print:
            case TokenType::LBrace:
            case TokenType::RBrace:
                return;
            default:
                break;
        }
        advance();
    }
}

void Parser::consume(TokenType type, const std::string& message,
                     const std::string& hint) {
    if (check(type)) {
        advance();
        return;
    }

    if (is_at_end()) {
        error(peek(), "unexpected end of file: " + message,
              hint.empty() ? "add the missing token or complete the statement"
                           : hint);
    } else if (is_invalid(peek())) {
        error(peek(), "cannot continue parsing after invalid token",
              "fix lexer errors before parsing");
        advance();
    } else {
        error(peek(), message, hint);
    }
}

bool Parser::consume_closing(TokenType open, TokenType close,
                             const std::string& what,
                             const std::string& hint) {
    if (match(close)) {
        return true;
    }

    if (is_at_end()) {
        error(peek(), "unexpected end of file while looking for closing " + what,
              "add a matching '" + std::string(1, close == TokenType::RParen ? ')' : '}') +
                  "' to balance the opening delimiter");
        return false;
    }

    if (check(open)) {
        error(peek(), "unbalanced " + what + ": extra opening delimiter",
              "remove the extra '" +
                  std::string(1, open == TokenType::LParen ? '(' : '{') + "'");
        return false;
    }

    error(peek(), "expected closing " + what + " before '" + peek().lexeme + "'",
          hint.empty() ? "check for missing or extra parentheses/braces" : hint);
    return false;
}

ParseResult Parser::parse() {
    ParseResult result;
    result.program = program();
    result.diagnostics = std::move(bag_);
    if (!result.program && !result.diagnostics.has_errors()) {
        result.program = std::make_unique<Program>();
    }
    return result;
}

std::unique_ptr<Program> Parser::program() {
    auto prog = std::make_unique<Program>();
    prog->loc = peek().start;

    while (!is_at_end()) {
        if (is_invalid(peek())) {
            error(peek(), "cannot parse invalid token '" + peek().lexeme + "'",
                  "fix lexical errors first");
            advance();
            synchronize();
            continue;
        }

        if (check(TokenType::Fn)) {
            prog->functions.push_back(function_declaration());
        } else if (check(TokenType::Let)) {
            prog->statements.push_back(declaration());
        } else {
            prog->statements.push_back(statement());
        }

        if (panic_mode_) {
            synchronize();
        }
    }

    return prog;
}

StmtPtr Parser::declaration() { return let_declaration(); }

std::unique_ptr<FunctionDecl> Parser::function_declaration() {
    const SourceLoc start = advance().start;  // fn

    if (!check(TokenType::Identifier)) {
        error(peek(), "expected function name after 'fn'",
              "use: fn name(a, b) { ... }");
        return nullptr;
    }
    const Token name_tok = advance();

    consume(TokenType::LParen, "expected '(' after function name",
            "declare parameters: fn add(a, b) { ... }");

    std::vector<std::string> params;
    if (!check(TokenType::RParen)) {
        do {
            if (!check(TokenType::Identifier)) {
                error(peek(), "expected parameter name",
                      "use identifiers for parameters");
                break;
            }
            params.push_back(advance().lexeme);
        } while (match(TokenType::Comma));
    }

    if (!consume_closing(TokenType::LParen, TokenType::RParen, "parenthesis",
                         "close parameter list with ')'")) {
        synchronize();
    }

    StmtPtr body_stmt = statement();
    auto* block = dynamic_cast<BlockStmt*>(body_stmt.get());
    if (!block) {
        error(name_tok, "function body must be a block { ... }",
              "wrap the body in curly braces");
        return nullptr;
    }

    auto fn = std::make_unique<FunctionDecl>();
    fn->loc = start;
    fn->name = name_tok.lexeme;
    fn->parameters = std::move(params);
    fn->body = std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(body_stmt.release()));
    return fn;
}

StmtPtr Parser::let_declaration() {
    const Token let_tok = advance();  // let
    const SourceLoc start = let_tok.start;

    if (!check(TokenType::Identifier)) {
        if (is_at_end()) {
            error(peek(), "unexpected end of file after 'let'",
                  "provide a variable name, e.g. let x = 0;");
        } else {
            error(peek(), "expected variable name after 'let', got '" +
                              peek().lexeme + "'",
                  "identifiers must start with a letter or underscore");
        }
        synchronize();
        return nullptr;
    }

    const Token name_tok = advance();
    consume(TokenType::Assign, "expected '=' after variable name in let declaration",
            "use the form: let name = expression;");

    ExprPtr init = expression();
    if (!init) {
        error(peek(), "missing initializer in let declaration",
              "provide a value after '=', e.g. let x = 10;");
    }

    consume(TokenType::Semicolon,
            "expected ';' after let declaration",
            "terminate the declaration with a semicolon");

    auto stmt = std::make_unique<LetStmt>();
    stmt->loc = start;
    stmt->name = name_tok.lexeme;
    stmt->initializer = std::move(init);
    return stmt;
}

StmtPtr Parser::statement() {
    if (match(TokenType::Return)) {
        return return_statement();
    }
    if (match(TokenType::Print)) {
        return print_statement();
    }
    if (match(TokenType::If)) {
        return if_statement();
    }
    if (match(TokenType::While)) {
        return while_statement();
    }
    if (match(TokenType::LBrace)) {
        return block_statement();
    }
    if (check(TokenType::Identifier) && current_ + 1 < tokens_.size() &&
        tokens_[current_ + 1].type == TokenType::Assign) {
        return assignment_statement();
    }
    return expression_statement();
}

StmtPtr Parser::expression_statement() {
    const SourceLoc start = peek().start;
    ExprPtr expr = expression();
    if (!expr) {
        if (is_at_end()) {
            error(peek(), "unexpected end of file in expression",
                  "complete the expression or remove incomplete code");
        }
        synchronize();
        return nullptr;
    }

    if (check(TokenType::Assign)) {
        error(peek(), "invalid assignment target",
              "only variables can be assigned; use 'let name = value' or "
              "'identifier = value', not a literal or expression");
        advance();
        expression();
    }

    consume(TokenType::Semicolon, "expected ';' after expression",
            "terminate the statement with a semicolon");

    auto stmt = std::make_unique<ExprStmt>();
    stmt->loc = start;
    stmt->expression = std::move(expr);
    return stmt;
}

StmtPtr Parser::assignment_statement() {
    const Token name_tok = advance();
    advance();  // =

    ExprPtr value = expression();
    if (!value) {
        error(peek(), "missing expression on right-hand side of assignment",
              "provide a value after '=', e.g. x = 10;");
    }

    consume(TokenType::Semicolon, "expected ';' after assignment",
            "terminate the assignment with a semicolon");

    auto stmt = std::make_unique<AssignStmt>();
    stmt->loc = name_tok.start;
    stmt->name = name_tok.lexeme;
    stmt->value = std::move(value);
    return stmt;
}

StmtPtr Parser::return_statement() {
    const SourceLoc start = previous().start;
    ExprPtr value = expression();
    if (!value) {
        error(peek(), "missing expression after 'return'",
              "use: return expression;");
    }
    consume(TokenType::Semicolon, "expected ';' after return",
            "terminate return with a semicolon");
    auto stmt = std::make_unique<ReturnStmt>();
    stmt->loc = start;
    stmt->value = std::move(value);
    return stmt;
}

StmtPtr Parser::print_statement() {
    const SourceLoc start = previous().start;
    ExprPtr expr = expression();
    if (!expr) {
        error(peek(), "missing expression after 'print'",
              "provide a value to print, e.g. print x;");
    }
    consume(TokenType::Semicolon, "expected ';' after print statement",
            "terminate with a semicolon");

    auto stmt = std::make_unique<PrintStmt>();
    stmt->loc = start;
    stmt->expression = std::move(expr);
    return stmt;
}

StmtPtr Parser::if_statement() {
    const SourceLoc start = previous().start;

    consume(TokenType::LParen, "expected '(' after 'if'",
            "wrap the condition in parentheses: if (condition) { ... }");
    ExprPtr cond = expression();
    if (!cond) {
        error(peek(), "missing condition in if statement",
              "provide a boolean expression inside parentheses");
    }

    if (!consume_closing(TokenType::LParen, TokenType::RParen, "parenthesis",
                         "close the condition with ')'")) {
        synchronize();
    }

    StmtPtr then_branch = statement();
    StmtPtr else_branch = nullptr;
    if (match(TokenType::Else)) {
        else_branch = statement();
    }

    auto stmt = std::make_unique<IfStmt>();
    stmt->loc = start;
    stmt->condition = std::move(cond);
    stmt->then_branch = std::move(then_branch);
    stmt->else_branch = std::move(else_branch);
    return stmt;
}

StmtPtr Parser::while_statement() {
    const SourceLoc start = previous().start;

    consume(TokenType::LParen, "expected '(' after 'while'",
            "wrap the condition in parentheses: while (condition) { ... }");
    ExprPtr cond = expression();
    if (!cond) {
        error(peek(), "missing condition in while loop",
              "provide a boolean expression inside parentheses");
    }

    if (!consume_closing(TokenType::LParen, TokenType::RParen, "parenthesis",
                         "close the condition with ')'")) {
        synchronize();
    }

    StmtPtr body = statement();

    auto stmt = std::make_unique<WhileStmt>();
    stmt->loc = start;
    stmt->condition = std::move(cond);
    stmt->body = std::move(body);
    return stmt;
}

StmtPtr Parser::block_statement() {
    const SourceLoc start = previous().start;
    std::vector<StmtPtr> statements;

    while (!check(TokenType::RBrace) && !is_at_end()) {
        if (is_invalid(peek())) {
            error(peek(), "cannot parse invalid token inside block",
                  "fix lexical errors first");
            advance();
            synchronize();
            continue;
        }

        if (check(TokenType::Let)) {
            statements.push_back(declaration());
        } else {
            statements.push_back(statement());
        }

        if (panic_mode_) {
            synchronize();
        }
    }

    if (!match(TokenType::RBrace)) {
        if (is_at_end()) {
            error(peek(), "unexpected end of file while parsing block",
                  "add a closing '}' for the block");
        } else if (check(TokenType::RParen)) {
            error(peek(), "unbalanced delimiters: found ')' where '}' was expected",
                  "blocks use curly braces { }, conditions use ( )");
        } else {
            error(peek(), "expected '}' to close block",
                  "ensure every '{' has a matching '}'");
        }
    }

    return finish_block(std::move(statements), start);
}

StmtPtr Parser::finish_block(std::vector<StmtPtr> statements, const SourceLoc& start) {
    auto block = std::make_unique<BlockStmt>();
    block->loc = start;
    block->statements = std::move(statements);
    return block;
}

ExprPtr Parser::expression() { return equality(); }

ExprPtr Parser::equality() {
    ExprPtr left = comparison();
    if (!left) {
        return nullptr;
    }

    while (match({TokenType::EqualEqual, TokenType::BangEqual})) {
        const Token op_tok = previous();
        ExprPtr right = comparison();
        if (!right) {
            error(peek(), "missing right operand after '" + op_tok.lexeme + "'",
                  "provide a value to compare");
            break;
        }
        auto node = std::make_unique<BinaryExpr>();
        node->loc = op_tok.start;
        node->op = op_tok.type == TokenType::EqualEqual ? BinOp::Eq : BinOp::Ne;
        node->left = std::move(left);
        node->right = std::move(right);
        left = std::move(node);
    }
    return left;
}

ExprPtr Parser::comparison() {
    ExprPtr left = term();
    if (!left) {
        return nullptr;
    }

    while (match({TokenType::Less, TokenType::Greater, TokenType::LessEqual,
                  TokenType::GreaterEqual})) {
        const Token op_tok = previous();
        ExprPtr right = term();
        if (!right) {
            error(peek(), "missing right operand after '" + op_tok.lexeme + "'",
                  "provide a value to compare");
            break;
        }
        auto node = std::make_unique<BinaryExpr>();
        node->loc = op_tok.start;
        switch (op_tok.type) {
            case TokenType::Less:
                node->op = BinOp::Lt;
                break;
            case TokenType::Greater:
                node->op = BinOp::Gt;
                break;
            case TokenType::LessEqual:
                node->op = BinOp::Le;
                break;
            case TokenType::GreaterEqual:
                node->op = BinOp::Ge;
                break;
            default:
                node->op = BinOp::Lt;
                break;
        }
        node->left = std::move(left);
        node->right = std::move(right);
        left = std::move(node);
    }
    return left;
}

ExprPtr Parser::term() {
    ExprPtr left = factor();
    if (!left) {
        return nullptr;
    }

    while (match({TokenType::Plus, TokenType::Minus})) {
        const Token op_tok = previous();
        ExprPtr right = factor();
        if (!right) {
            error(peek(), "missing right operand after '" + op_tok.lexeme + "'",
                  "provide a number or expression after the operator");
            break;
        }
        auto node = std::make_unique<BinaryExpr>();
        node->loc = op_tok.start;
        node->op = op_tok.type == TokenType::Plus ? BinOp::Add : BinOp::Sub;
        node->left = std::move(left);
        node->right = std::move(right);
        left = std::move(node);
    }
    return left;
}

ExprPtr Parser::factor() {
    ExprPtr left = unary();
    if (!left) {
        return nullptr;
    }

    while (match({TokenType::Star, TokenType::Slash})) {
        const Token op_tok = previous();
        ExprPtr right = unary();
        if (!right) {
            error(peek(), "missing right operand after '" + op_tok.lexeme + "'",
                  "provide a number or expression after the operator");
            break;
        }
        auto node = std::make_unique<BinaryExpr>();
        node->loc = op_tok.start;
        node->op = op_tok.type == TokenType::Star ? BinOp::Mul : BinOp::Div;
        node->left = std::move(left);
        node->right = std::move(right);
        left = std::move(node);
    }
    return left;
}

ExprPtr Parser::unary() {
    if (match(TokenType::Minus)) {
        const SourceLoc op_loc = previous().start;
        ExprPtr operand = unary();
        if (!operand) {
            error(peek(), "missing operand after unary '-'",
                  "provide a number or expression, e.g. -x or -(a + b)");
            return nullptr;
        }
        auto node = std::make_unique<UnaryExpr>();
        node->loc = op_loc;
        node->op = UnaryOp::Neg;
        node->operand = std::move(operand);
        return node;
    }
    return primary();
}

ExprPtr Parser::primary() {
    if (is_at_end()) {
        error(peek(), "unexpected end of file in expression",
              "complete the expression or add a value");
        return nullptr;
    }

    if (is_invalid(peek())) {
        error(peek(), "invalid token in expression", "fix lexical errors first");
        advance();
        return nullptr;
    }

    if (match(TokenType::Integer)) {
        auto node = std::make_unique<IntLiteralExpr>();
        node->loc = previous().start;
        node->value = std::strtoll(previous().lexeme.c_str(), nullptr, 10);
        return node;
    }

    if (match(TokenType::True)) {
        auto node = std::make_unique<BoolLiteralExpr>();
        node->loc = previous().start;
        node->value = true;
        return node;
    }

    if (match(TokenType::False)) {
        auto node = std::make_unique<BoolLiteralExpr>();
        node->loc = previous().start;
        node->value = false;
        return node;
    }

    if (match(TokenType::Input)) {
        auto node = std::make_unique<InputExpr>();
        node->loc = previous().start;
        return node;
    }

    if (check(TokenType::Identifier) && peek_next().type == TokenType::LParen) {
        const Token name_tok = advance();
        return finish_call(name_tok.lexeme, name_tok.start);
    }

    if (match(TokenType::Identifier)) {
        auto node = std::make_unique<VariableExpr>();
        node->loc = previous().start;
        node->name = previous().lexeme;
        return node;
    }

    if (match(TokenType::LParen)) {
        ExprPtr inner = expression();
        if (!inner) {
            error(peek(), "missing expression inside parentheses",
                  "add a value or sub-expression between ( and )");
        }
        if (!consume_closing(TokenType::LParen, TokenType::RParen, "parenthesis",
                             "close the grouped expression with ')'")) {
            return inner;
        }
        return inner;
    }

    if (check(TokenType::Plus) || check(TokenType::Star) || check(TokenType::Slash) ||
        check(TokenType::EqualEqual) || check(TokenType::Less) ||
        check(TokenType::Greater)) {
        const Token op = peek();
        error(op, "missing left operand before '" + op.lexeme + "'",
              "unary '+' is not supported; provide a value before the operator");
        advance();
        return nullptr;
    }

    error(peek(), "expected expression, found '" + peek().lexeme + "'",
          "use a number, true/false, identifier, input, or parenthesized expression");
    return nullptr;
}

ExprPtr Parser::finish_call(const std::string& name, const SourceLoc& loc) {
    consume(TokenType::LParen, "expected '(' after function name in call",
            "call syntax: name(arg1, arg2)");

    std::vector<ExprPtr> args;
    if (!check(TokenType::RParen)) {
        do {
            ExprPtr arg = expression();
            if (!arg) {
                error(peek(), "missing function argument expression",
                      "provide a value for each argument");
                break;
            }
            args.push_back(std::move(arg));
        } while (match(TokenType::Comma));
    }

    if (!consume_closing(TokenType::LParen, TokenType::RParen, "parenthesis",
                         "close argument list with ')'")) {
        synchronize();
    }

    auto node = std::make_unique<CallExpr>();
    node->loc = loc;
    node->callee = name;
    node->arguments = std::move(args);
    return node;
}

}  // namespace cvm
