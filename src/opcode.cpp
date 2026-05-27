#include "cvm++/opcode.hpp"

namespace cvm {

std::string_view opcode_name(OpCode op) {
    switch (op) {
        case OpCode::PushInt:
            return "PUSH_INT";
        case OpCode::PushBool:
            return "PUSH_BOOL";
        case OpCode::Pop:
            return "POP";
        case OpCode::LoadVar:
            return "LOAD_VAR";
        case OpCode::StoreVar:
            return "STORE_VAR";
        case OpCode::Add:
            return "ADD";
        case OpCode::Sub:
            return "SUB";
        case OpCode::Mul:
            return "MUL";
        case OpCode::Div:
            return "DIV";
        case OpCode::Eq:
            return "EQ";
        case OpCode::Lt:
            return "LT";
        case OpCode::Gt:
            return "GT";
        case OpCode::Ne:
            return "NE";
        case OpCode::Le:
            return "LE";
        case OpCode::Ge:
            return "GE";
        case OpCode::Neg:
            return "NEG";
        case OpCode::LoadLocal:
            return "LOAD_LOCAL";
        case OpCode::StoreLocal:
            return "STORE_LOCAL";
        case OpCode::Call:
            return "CALL";
        case OpCode::Return:
            return "RETURN";
        case OpCode::Input:
            return "INPUT";
        case OpCode::Print:
            return "PRINT";
        case OpCode::Jump:
            return "JUMP";
        case OpCode::JumpIfFalse:
            return "JUMP_IF_FALSE";
        case OpCode::Halt:
            return "HALT";
    }
    return "UNKNOWN";
}

}  // namespace cvm
