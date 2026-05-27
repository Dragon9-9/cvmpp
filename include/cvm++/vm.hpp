#pragma once

#include "cvm++/bytecode.hpp"
#include "cvm++/diagnostic.hpp"
#include "cvm++/value.hpp"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace cvm {

struct VmSession {
    std::unordered_map<std::string, VmValue> variables;
};

struct VmResult {
    DiagnosticBag diagnostics;
    std::vector<std::string> output;

    bool ok() const { return !diagnostics.has_errors(); }
};

struct CallFrame {
    std::size_t return_ip{0};
    std::vector<VmValue> locals;
};

class VirtualMachine {
public:
    static constexpr std::size_t kMaxStackDepth = 65536;

    explicit VirtualMachine(const BytecodeChunk& chunk);

    VmResult run(std::istream& input, std::ostream& output, VmSession* session = nullptr);

private:
    const BytecodeChunk& chunk_;
    std::size_t ip_{0};
    std::vector<VmValue> stack_;
    std::vector<VmValue> globals_;
    std::vector<bool> global_init_;
    std::vector<CallFrame> frames_;
    VmSession* session_{nullptr};

    VmResult result_;
    std::istream* input_{nullptr};
    std::ostream* output_{nullptr};

    [[nodiscard]] bool ip_in_bounds() const;
    [[nodiscard]] bool ensure_bytes(std::size_t count);

    std::uint8_t read_u8();
    std::uint16_t read_u16();
    std::uint32_t read_u32();
    std::int64_t read_i64();

    void push(const VmValue& value);
    VmValue pop();

    VmResult fail(const std::string& message, const std::string& hint = {});

    void run_loop();
    void execute_opcode(OpCode op);

    VmValue read_input_value();
    void emit_output(const VmValue& value);

    const std::string& name_at(std::uint16_t index) const;
    void store_global(std::uint16_t index, const VmValue& value);
    VmValue load_global(std::uint16_t index);
    void store_local(std::uint8_t slot, const VmValue& value);
    VmValue load_local(std::uint8_t slot);

    VmValue binary_int(OpCode op, const VmValue& left, const VmValue& right);
    VmValue compare_eq(const VmValue& left, const VmValue& right);
};

VmResult execute(const BytecodeChunk& chunk, VmSession* session = nullptr,
                 std::istream& input = std::cin, std::ostream& output = std::cout);

}  // namespace cvm
