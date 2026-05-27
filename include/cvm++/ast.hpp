#pragma once

#include "cvm++/source_loc.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace cvm {

enum class BinOp { Add, Sub, Mul, Div, Eq, Ne, Lt, Gt, Le, Ge };
enum class UnaryOp { Neg };

std::string_view bin_op_name(BinOp op);
std::string_view unary_op_name(UnaryOp op);

struct Expr;
struct Stmt;
struct Program;

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

struct Expr {
    SourceLoc loc;
    virtual ~Expr() = default;
};

struct Stmt {
    SourceLoc loc;
    virtual ~Stmt() = default;
};

struct IntLiteralExpr : Expr {
    std::int64_t value{0};
};

struct BoolLiteralExpr : Expr {
    bool value{false};
};

struct VariableExpr : Expr {
    std::string name;
};

struct InputExpr : Expr {};

struct CallExpr : Expr {
    std::string callee;
    std::vector<ExprPtr> arguments;
};

struct UnaryExpr : Expr {
    UnaryOp op{UnaryOp::Neg};
    ExprPtr operand;
};

struct BinaryExpr : Expr {
    BinOp op{BinOp::Add};
    ExprPtr left;
    ExprPtr right;
};

struct ExprStmt : Stmt {
    ExprPtr expression;
};

struct LetStmt : Stmt {
    std::string name;
    ExprPtr initializer;
};

struct AssignStmt : Stmt {
    std::string name;
    ExprPtr value;
};

struct PrintStmt : Stmt {
    ExprPtr expression;
};

struct ReturnStmt : Stmt {
    ExprPtr value;
};

struct IfStmt : Stmt {
    ExprPtr condition;
    StmtPtr then_branch;
    StmtPtr else_branch;
};

struct WhileStmt : Stmt {
    ExprPtr condition;
    StmtPtr body;
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;
};

struct FunctionDecl {
    std::string name;
    std::vector<std::string> parameters;
    std::unique_ptr<BlockStmt> body;
    SourceLoc loc;
};

struct Program {
    std::vector<std::unique_ptr<FunctionDecl>> functions;
    std::vector<StmtPtr> statements;
    SourceLoc loc;
};

}  // namespace cvm
