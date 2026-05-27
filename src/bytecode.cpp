#include "cvm++/bytecode.hpp"

#include <cstring>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace cvm {
namespace {

std::uint16_t read_u16(const std::uint8_t* p) {
    std::uint16_t v = 0;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

std::uint32_t read_u32(const std::uint8_t* p) {
    std::uint32_t v = 0;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

std::int64_t read_i64(const std::uint8_t* p) {
    std::int64_t v = 0;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

}  // namespace

BytecodeWriter::BytecodeWriter(BytecodeChunk& chunk) : chunk_(chunk) {}

std::size_t BytecodeWriter::current_offset() const { return chunk_.code.size(); }

void BytecodeWriter::emit(OpCode op) {
    chunk_.code.push_back(static_cast<std::uint8_t>(op));
}

void BytecodeWriter::emit_u8(std::uint8_t value) {
    chunk_.code.push_back(value);
}

void BytecodeWriter::emit_u16(std::uint16_t value) {
    const auto b0 = static_cast<std::uint8_t>(value & 0xFF);
    const auto b1 = static_cast<std::uint8_t>((value >> 8) & 0xFF);
    chunk_.code.push_back(b0);
    chunk_.code.push_back(b1);
}

void BytecodeWriter::emit_u32(std::uint32_t value) {
    const auto b0 = static_cast<std::uint8_t>(value & 0xFF);
    const auto b1 = static_cast<std::uint8_t>((value >> 8) & 0xFF);
    const auto b2 = static_cast<std::uint8_t>((value >> 16) & 0xFF);
    const auto b3 = static_cast<std::uint8_t>((value >> 24) & 0xFF);
    chunk_.code.push_back(b0);
    chunk_.code.push_back(b1);
    chunk_.code.push_back(b2);
    chunk_.code.push_back(b3);
}

void BytecodeWriter::emit_i64(std::int64_t value) {
    const auto* bytes = reinterpret_cast<const std::uint8_t*>(&value);
    for (std::size_t i = 0; i < sizeof(value); ++i) {
        chunk_.code.push_back(bytes[i]);
    }
}

std::uint16_t BytecodeWriter::intern_name(const std::string& name) {
    for (std::size_t i = 0; i < chunk_.names.size(); ++i) {
        if (chunk_.names[i] == name) {
            return static_cast<std::uint16_t>(i);
        }
    }
    if (chunk_.names.size() >= static_cast<std::size_t>(UINT16_MAX) + 1) {
        throw std::runtime_error("too many distinct variable names");
    }
    chunk_.names.push_back(name);
    return static_cast<std::uint16_t>(chunk_.names.size() - 1);
}

void BytecodeWriter::patch_u32(std::size_t offset, std::uint32_t value) {
    if (offset + kJumpOperandSize > chunk_.code.size()) {
        throw std::runtime_error("patch offset out of range");
    }
    auto* bytes = chunk_.code.data() + offset;
    bytes[0] = static_cast<std::uint8_t>(value & 0xFF);
    bytes[1] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
    bytes[2] = static_cast<std::uint8_t>((value >> 16) & 0xFF);
    bytes[3] = static_cast<std::uint8_t>((value >> 24) & 0xFF);
}

std::vector<DisasmInstr> disassemble(const BytecodeChunk& chunk) {
    std::vector<DisasmInstr> out;
    std::size_t ip = 0;

    while (ip < chunk.code.size()) {
        DisasmInstr row;
        row.offset = ip;
        const auto op = static_cast<OpCode>(chunk.code[ip++]);
        row.opcode = op;

        std::ostringstream operands;
        switch (op) {
            case OpCode::PushInt: {
                if (ip + kIntOperandSize > chunk.code.size()) {
                    operands << "<truncated>";
                    break;
                }
                operands << read_i64(chunk.code.data() + ip);
                ip += kIntOperandSize;
                break;
            }
            case OpCode::PushBool: {
                if (ip >= chunk.code.size()) {
                    operands << "<truncated>";
                    break;
                }
                operands << (chunk.code[ip++] ? "true" : "false");
                break;
            }
            case OpCode::Call: {
                if (ip + kNameIndexSize + 1 > chunk.code.size()) {
                    operands << "<truncated>";
                    break;
                }
                const std::uint16_t idx = read_u16(chunk.code.data() + ip);
                ip += kNameIndexSize;
                const std::uint8_t argc = chunk.code[ip++];
                operands << '#' << idx << " argc=" << static_cast<int>(argc);
                if (idx < chunk.functions.size()) {
                    operands << " (" << chunk.functions[idx].name << '@'
                              << chunk.functions[idx].address << ')';
                }
                break;
            }
            case OpCode::LoadLocal:
            case OpCode::StoreLocal: {
                if (ip >= chunk.code.size()) {
                    operands << "<truncated>";
                    break;
                }
                operands << "slot=" << static_cast<int>(chunk.code[ip++]);
                break;
            }
            case OpCode::LoadVar:
            case OpCode::StoreVar: {
                if (ip + kNameIndexSize > chunk.code.size()) {
                    operands << "<truncated>";
                    break;
                }
                const std::uint16_t idx = read_u16(chunk.code.data() + ip);
                ip += kNameIndexSize;
                operands << '#' << idx;
                if (idx < chunk.names.size()) {
                    operands << " (" << chunk.names[idx] << ')';
                }
                break;
            }
            case OpCode::Jump:
            case OpCode::JumpIfFalse: {
                if (ip + kJumpOperandSize > chunk.code.size()) {
                    operands << "<truncated>";
                    break;
                }
                operands << '@' << read_u32(chunk.code.data() + ip);
                ip += kJumpOperandSize;
                break;
            }
            case OpCode::Pop:
            case OpCode::Add:
            case OpCode::Sub:
            case OpCode::Mul:
            case OpCode::Div:
            case OpCode::Eq:
            case OpCode::Ne:
            case OpCode::Lt:
            case OpCode::Gt:
            case OpCode::Le:
            case OpCode::Ge:
            case OpCode::Return:
            case OpCode::Neg:
            case OpCode::Input:
            case OpCode::Print:
            case OpCode::Halt:
                break;
        }

        row.operands = operands.str();
        out.push_back(std::move(row));
    }

    return out;
}

}  // namespace cvm
