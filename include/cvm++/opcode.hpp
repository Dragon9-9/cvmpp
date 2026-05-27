#pragma once

#include <cstdint>
#include <string_view>

namespace cvm {

enum class OpCode : std::uint8_t {
    PushInt = 0x01,
    PushBool = 0x02,
    Pop = 0x03,

    LoadVar = 0x10,
    StoreVar = 0x11,
    LoadLocal = 0x12,
    StoreLocal = 0x13,

    Add = 0x20,
    Sub = 0x21,
    Mul = 0x22,
    Div = 0x23,
    Eq = 0x24,
    Lt = 0x25,
    Gt = 0x27,
    Neg = 0x26,
    Ne = 0x28,
    Le = 0x29,
    Ge = 0x2A,

    Input = 0x30,
    Print = 0x31,

    Jump = 0x40,
    JumpIfFalse = 0x41,
    Call = 0x42,
    Return = 0x43,

    Halt = 0xFF,
};

std::string_view opcode_name(OpCode op);

constexpr std::size_t kJumpOperandSize = sizeof(std::uint32_t);
constexpr std::size_t kIntOperandSize = sizeof(std::int64_t);
constexpr std::size_t kNameIndexSize = sizeof(std::uint16_t);

}  // namespace cvm
