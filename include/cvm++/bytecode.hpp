#pragma once

#include "cvm++/opcode.hpp"
#include "cvm++/source_loc.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace cvm {

struct FunctionMeta {
    std::string name;
    std::uint32_t address{0};
    std::uint8_t arity{0};
};

struct BytecodeChunk {
    std::vector<std::uint8_t> code;
    std::vector<std::string> names;
    std::vector<FunctionMeta> functions;

    std::size_t size() const { return code.size(); }
    bool empty() const { return code.empty(); }
};

class BytecodeWriter {
public:
    explicit BytecodeWriter(BytecodeChunk& chunk);

    std::size_t current_offset() const;
    void emit(OpCode op);
    void emit_u8(std::uint8_t value);
    void emit_u16(std::uint16_t value);
    void emit_u32(std::uint32_t value);
    void emit_i64(std::int64_t value);

    std::uint16_t intern_name(const std::string& name);
    void patch_u32(std::size_t offset, std::uint32_t value);

private:
    BytecodeChunk& chunk_;
};

struct DisasmInstr {
    std::size_t offset{0};
    OpCode opcode{OpCode::Halt};
    std::string operands;
};

std::vector<DisasmInstr> disassemble(const BytecodeChunk& chunk);

}  // namespace cvm
