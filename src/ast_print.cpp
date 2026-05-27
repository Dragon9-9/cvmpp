#include "cvm++/ui.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace cvm::ui {
namespace {

std::string colorize(const char* color, const std::string& text) {
    if (!stdout_is_tty()) {
        return text;
    }
    return std::string(color) + text + ansi::reset;
}

std::string expr_label(const Expr& expr) {
    if (dynamic_cast<const IntLiteralExpr*>(&expr)) {
        return "IntLiteral";
    }
    if (dynamic_cast<const BoolLiteralExpr*>(&expr)) {
        return "BoolLiteral";
    }
    if (dynamic_cast<const VariableExpr*>(&expr)) {
        return "Variable";
    }
    if (dynamic_cast<const InputExpr*>(&expr)) {
        return "Input";
    }
    if (dynamic_cast<const UnaryExpr*>(&expr)) {
        return "Unary";
    }
    if (dynamic_cast<const BinaryExpr*>(&expr)) {
        return "Binary";
    }
    if (dynamic_cast<const CallExpr*>(&expr)) {
        return "Call";
    }
    return "Expr";
}

std::string stmt_label(const Stmt& stmt) {
    if (dynamic_cast<const LetStmt*>(&stmt)) {
        return "Let";
    }
    if (dynamic_cast<const AssignStmt*>(&stmt)) {
        return "Assign";
    }
    if (dynamic_cast<const PrintStmt*>(&stmt)) {
        return "Print";
    }
    if (dynamic_cast<const ReturnStmt*>(&stmt)) {
        return "Return";
    }
    if (dynamic_cast<const IfStmt*>(&stmt)) {
        return "If";
    }
    if (dynamic_cast<const WhileStmt*>(&stmt)) {
        return "While";
    }
    if (dynamic_cast<const BlockStmt*>(&stmt)) {
        return "Block";
    }
    if (dynamic_cast<const ExprStmt*>(&stmt)) {
        return "ExprStmt";
    }
    return "Stmt";
}

std::string expr_detail(const Expr& expr) {
    if (const auto* lit = dynamic_cast<const IntLiteralExpr*>(&expr)) {
        return std::to_string(lit->value);
    }
    if (const auto* lit = dynamic_cast<const BoolLiteralExpr*>(&expr)) {
        return lit->value ? "true" : "false";
    }
    if (const auto* var = dynamic_cast<const VariableExpr*>(&expr)) {
        return var->name;
    }
    if (const auto* unary = dynamic_cast<const UnaryExpr*>(&expr)) {
        return std::string(unary_op_name(unary->op));
    }
    if (const auto* bin = dynamic_cast<const BinaryExpr*>(&expr)) {
        return std::string(bin_op_name(bin->op));
    }
    if (dynamic_cast<const InputExpr*>(&expr)) {
        return "stdin";
    }
    if (const auto* call = dynamic_cast<const CallExpr*>(&expr)) {
        return call->callee + "(" + std::to_string(call->arguments.size()) + " args)";
    }
    return "";
}

std::string stmt_detail(const Stmt& stmt) {
    if (const auto* let = dynamic_cast<const LetStmt*>(&stmt)) {
        return let->name;
    }
    if (const auto* assign = dynamic_cast<const AssignStmt*>(&stmt)) {
        return assign->name;
    }
    if (const auto* block = dynamic_cast<const BlockStmt*>(&stmt)) {
        return std::to_string(block->statements.size()) + " stmt(s)";
    }
    return "";
}

void print_tree_line(const std::string& prefix, const std::string& branch,
                     const std::string& label, const std::string& detail,
                     const SourceLoc& loc, const char* label_color) {
    std::ostringstream loc_ss;
    loc_ss << loc.to_string();

    std::cout << colorize(ansi::dim, prefix) << colorize(ansi::magenta, branch)
              << colorize(label_color, label);
    if (!detail.empty()) {
        std::cout << colorize(ansi::dim, " (") << colorize(ansi::cyan, detail)
                  << colorize(ansi::dim, ")");
    }
    std::cout << colorize(ansi::dim, " @ ") << colorize(ansi::dim, loc_ss.str())
              << '\n';
}

void print_expr(const Expr& expr, const std::string& prefix, bool is_last);
void print_stmt(const Stmt& stmt, const std::string& prefix, bool is_last);

void print_expr(const Expr& expr, const std::string& prefix, bool is_last) {
    const std::string branch = is_last ? "+-- " : "|-- ";
    const std::string child_prefix = prefix + (is_last ? "    " : "|   ");
    const char* color = ansi::cyan;

    print_tree_line(prefix, branch, expr_label(expr), expr_detail(expr), expr.loc,
                    color);

    if (const auto* unary = dynamic_cast<const UnaryExpr*>(&expr)) {
        if (unary->operand) {
            print_expr(*unary->operand, child_prefix, true);
        }
        return;
    }

    if (const auto* bin = dynamic_cast<const BinaryExpr*>(&expr)) {
        if (bin->left) {
            print_expr(*bin->left, child_prefix, false);
        }
        if (bin->right) {
            print_expr(*bin->right, child_prefix, true);
        }
        return;
    }

    if (const auto* call = dynamic_cast<const CallExpr*>(&expr)) {
        for (std::size_t i = 0; i < call->arguments.size(); ++i) {
            if (call->arguments[i]) {
                print_expr(*call->arguments[i], child_prefix,
                           i + 1 == call->arguments.size());
            }
        }
    }
}

void print_stmt(const Stmt& stmt, const std::string& prefix, bool is_last) {
    const std::string branch = is_last ? "+-- " : "|-- ";
    const std::string child_prefix = prefix + (is_last ? "    " : "|   ");
    const char* color = ansi::yellow;

    print_tree_line(prefix, branch, stmt_label(stmt), stmt_detail(stmt), stmt.loc,
                    color);

    if (const auto* let = dynamic_cast<const LetStmt*>(&stmt)) {
        if (let->initializer) {
            print_expr(*let->initializer, child_prefix, true);
        }
        return;
    }

    if (const auto* assign = dynamic_cast<const AssignStmt*>(&stmt)) {
        if (assign->value) {
            print_expr(*assign->value, child_prefix, true);
        }
        return;
    }

    if (const auto* ret = dynamic_cast<const ReturnStmt*>(&stmt)) {
        if (ret->value) {
            print_expr(*ret->value, child_prefix, true);
        }
        return;
    }

    if (const auto* print_s = dynamic_cast<const PrintStmt*>(&stmt)) {
        if (print_s->expression) {
            print_expr(*print_s->expression, child_prefix, true);
        }
        return;
    }

    if (const auto* expr_s = dynamic_cast<const ExprStmt*>(&stmt)) {
        if (expr_s->expression) {
            print_expr(*expr_s->expression, child_prefix, true);
        }
        return;
    }

    if (const auto* if_s = dynamic_cast<const IfStmt*>(&stmt)) {
        if (if_s->condition) {
            print_expr(*if_s->condition, child_prefix, false);
        }
        if (if_s->then_branch) {
            print_stmt(*if_s->then_branch, child_prefix, if_s->else_branch == nullptr);
        }
        if (if_s->else_branch) {
            print_stmt(*if_s->else_branch, child_prefix, true);
        }
        return;
    }

    if (const auto* while_s = dynamic_cast<const WhileStmt*>(&stmt)) {
        if (while_s->condition) {
            print_expr(*while_s->condition, child_prefix, false);
        }
        if (while_s->body) {
            print_stmt(*while_s->body, child_prefix, true);
        }
        return;
    }

    if (const auto* block = dynamic_cast<const BlockStmt*>(&stmt)) {
        for (std::size_t i = 0; i < block->statements.size(); ++i) {
            if (block->statements[i]) {
                print_stmt(*block->statements[i], child_prefix,
                           i + 1 == block->statements.size());
            }
        }
    }
}

}  // namespace

void print_ast_tree(const Program& program) {
    std::cout << '\n' << colorize(ansi::magenta, "  Abstract Syntax Tree")
              << '\n';
    std::cout << colorize(ansi::dim, "  ") << std::string(40, '-') << '\n';
    std::cout << colorize(ansi::bright_cyan, "  Program")
              << colorize(ansi::dim, " @ ") << program.loc.to_string() << '\n';

    for (std::size_t i = 0; i < program.functions.size(); ++i) {
        if (program.functions[i]) {
            const auto& fn = *program.functions[i];
            std::cout << colorize(ansi::yellow, "  +-- Fn ") << fn.name << " ("
                      << fn.parameters.size() << " params)\n";
            if (fn.body) {
                for (std::size_t j = 0; j < fn.body->statements.size(); ++j) {
                    if (fn.body->statements[j]) {
                        print_stmt(*fn.body->statements[j], "  |   ",
                                   j + 1 == fn.body->statements.size());
                    }
                }
            }
        }
    }

    for (std::size_t i = 0; i < program.statements.size(); ++i) {
        if (program.statements[i]) {
            print_stmt(*program.statements[i], "  ",
                       i + 1 == program.statements.size());
        }
    }
    std::cout << '\n';
}

}  // namespace cvm::ui
