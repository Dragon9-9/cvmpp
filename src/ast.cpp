#include "cvm++/ast.hpp"

namespace cvm {

std::string_view bin_op_name(BinOp op) {
    switch (op) {
        case BinOp::Add:
            return "+";
        case BinOp::Sub:
            return "-";
        case BinOp::Mul:
            return "*";
        case BinOp::Div:
            return "/";
        case BinOp::Eq:
            return "==";
        case BinOp::Lt:
            return "<";
        case BinOp::Gt:
            return ">";
        case BinOp::Ne:
            return "!=";
        case BinOp::Le:
            return "<=";
        case BinOp::Ge:
            return ">=";
    }
    return "?";
}

std::string_view unary_op_name(UnaryOp op) {
    switch (op) {
        case UnaryOp::Neg:
            return "-";
    }
    return "?";
}

}  // namespace cvm
