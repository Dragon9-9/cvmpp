#include "cvm++/vm.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <limits>
#include <sstream>

namespace cvm {
namespace {

std::string trim_copy(std::string s) {
    const auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
    return s;
}

}  // namespace

VirtualMachine::VirtualMachine(const BytecodeChunk& chunk) : chunk_(chunk) {}

const std::string& VirtualMachine::name_at(std::uint16_t index) const {
    static const std::string kEmpty;
    if (index >= chunk_.names.size()) {
        return kEmpty;
    }
    return chunk_.names[index];
}

void VirtualMachine::store_global(std::uint16_t index, const VmValue& value) {
    if (session_ != nullptr) {
        const std::string& name = name_at(index);
        if (name.empty()) {
            fail("invalid variable index " + std::to_string(index),
                 "bytecode name pool may be corrupt");
            return;
        }
        session_->variables[name] = value;
        return;
    }

    if (index >= globals_.size()) {
        globals_.resize(index + 1);
        global_init_.resize(index + 1, false);
    }
    globals_[index] = value;
    global_init_[index] = true;
}

VmValue VirtualMachine::load_global(std::uint16_t index) {
    if (session_ != nullptr) {
        const std::string& name = name_at(index);
        if (name.empty()) {
            fail("invalid variable index " + std::to_string(index),
                 "bytecode name pool may be corrupt");
            return std::int64_t{0};
        }
        const auto it = session_->variables.find(name);
        if (it == session_->variables.end()) {
            fail("read of uninitialized variable '" + name + "'",
                 "assign with let or = before use");
            return std::int64_t{0};
        }
        return it->second;
    }

    if (index >= globals_.size() || !global_init_[index]) {
        std::string name = "#" + std::to_string(index);
        if (index < chunk_.names.size()) {
            name = "'" + chunk_.names[index] + "'";
        }
        fail("read of uninitialized variable " + name,
             "assign with let or = before use");
        return std::int64_t{0};
    }
    return globals_[index];
}

bool VirtualMachine::ip_in_bounds() const { return ip_ < chunk_.code.size(); }

bool VirtualMachine::ensure_bytes(std::size_t count) {
    if (ip_ + count > chunk_.code.size()) {
        fail("instruction pointer overran bytecode (truncated instruction at offset " +
                 std::to_string(ip_) + ")",
             "recompile the program; bytecode may be corrupt or incomplete");
        return false;
    }
    return true;
}

std::uint8_t VirtualMachine::read_u8() {
    if (!ensure_bytes(1)) {
        return 0;
    }
    return chunk_.code[ip_++];
}

std::uint16_t VirtualMachine::read_u16() {
    if (!ensure_bytes(kNameIndexSize)) {
        return 0;
    }
    std::uint16_t v = 0;
    std::memcpy(&v, chunk_.code.data() + ip_, sizeof(v));
    ip_ += kNameIndexSize;
    return v;
}

std::uint32_t VirtualMachine::read_u32() {
    if (!ensure_bytes(kJumpOperandSize)) {
        return 0;
    }
    std::uint32_t v = 0;
    std::memcpy(&v, chunk_.code.data() + ip_, sizeof(v));
    ip_ += kJumpOperandSize;
    return v;
}

std::int64_t VirtualMachine::read_i64() {
    if (!ensure_bytes(kIntOperandSize)) {
        return 0;
    }
    std::int64_t v = 0;
    std::memcpy(&v, chunk_.code.data() + ip_, sizeof(v));
    ip_ += kIntOperandSize;
    return v;
}

VmResult VirtualMachine::fail(const std::string& message, const std::string& hint) {
    Diagnostic d;
    d.phase = Phase::Vm;
    d.severity = Severity::Error;
    d.message = message;
    d.loc.line = 0;
    d.loc.column = 0;
    if (ip_ > 0) {
        d.message += " [ip=" + std::to_string(ip_ - 1) + "]";
    }
    d.hint = hint;
    result_.diagnostics.push(std::move(d));
    return result_;
}

void VirtualMachine::push(const VmValue& value) {
    if (stack_.size() >= kMaxStackDepth) {
        fail("stack overflow (maximum depth " + std::to_string(kMaxStackDepth) + ")",
             "simplify expressions or reduce recursion depth");
        return;
    }
    stack_.push_back(value);
}

VmValue VirtualMachine::pop() {
    if (stack_.empty()) {
        fail("stack underflow (pop from empty stack)",
             "compiler/bytecode mismatch — too many pops for pushes");
        return std::int64_t{0};
    }
    VmValue v = stack_.back();
    stack_.pop_back();
    return v;
}

VmValue VirtualMachine::read_input_value() {
    if (!input_ || !(*input_)) {
        fail("input stream unavailable while executing INPUT",
             "provide input on stdin or run interactively");
        return std::int64_t{0};
    }

    std::string line;
    if (!std::getline(*input_, line)) {
        fail("unexpected end of input while executing INPUT",
             "provide a value when the program requests input");
        return std::int64_t{0};
    }

    line = trim_copy(line);
    if (line == "true") {
        return true;
    }
    if (line == "false") {
        return false;
    }

    try {
        std::size_t consumed = 0;
        const long long parsed = std::stoll(line, &consumed);
        if (consumed != line.size()) {
            fail("invalid input '" + line + "'",
                 "enter an integer, or the literals true/false");
            return std::int64_t{0};
        }
        if (parsed < std::numeric_limits<std::int64_t>::min() ||
            parsed > std::numeric_limits<std::int64_t>::max()) {
            fail("input integer out of range",
                 "enter a value that fits in a 64-bit signed integer");
            return std::int64_t{0};
        }
        return static_cast<std::int64_t>(parsed);
    } catch (const std::exception&) {
        fail("invalid input '" + line + "'",
             "enter an integer, or the literals true/false");
        return std::int64_t{0};
    }
}

void VirtualMachine::emit_output(const VmValue& value) {
    result_.output.push_back(value_to_string(value));
}

VmValue VirtualMachine::binary_int(OpCode op, const VmValue& left,
                                   const VmValue& right) {
    if (!is_int(left) || !is_int(right)) {
        fail("type mismatch: arithmetic requires two integers (got " +
                 value_type_name(left) + " and " + value_type_name(right) + ")",
             "use integer operands or cast logic; booleans cannot mix with + - * /");
        return std::int64_t{0};
    }

    const std::int64_t a = as_int(left);
    const std::int64_t b = as_int(right);

    switch (op) {
        case OpCode::Add:
            return a + b;
        case OpCode::Sub:
            return a - b;
        case OpCode::Mul:
            return a * b;
        case OpCode::Div:
            if (b == 0) {
                fail("divide by zero",
                     "check the divisor before division; 0 is not allowed");
                return std::int64_t{0};
            }
            return a / b;
        default:
            fail("internal VM error: invalid arithmetic opcode",
                 "report this opcode dispatch bug");
            return std::int64_t{0};
    }
}

VmValue VirtualMachine::compare_eq(const VmValue& left, const VmValue& right) {
    if (is_int(left) && is_int(right)) {
        return as_int(left) == as_int(right);
    }
    if (is_bool(left) && is_bool(right)) {
        return as_bool(left) == as_bool(right);
    }
    fail("type mismatch: == requires operands of the same type (got " +
             value_type_name(left) + " and " + value_type_name(right) + ")",
         "compare integers to integers or booleans to booleans");
    return false;
}

void VirtualMachine::execute_opcode(OpCode op) {
    switch (op) {
        case OpCode::PushInt: {
            push(read_i64());
            break;
        }
        case OpCode::PushBool: {
            push(read_u8() != 0);
            break;
        }
        case OpCode::Pop: {
            (void)pop();
            break;
        }
        case OpCode::LoadVar: {
            const std::uint16_t index = read_u16();
            const VmValue value = load_global(index);
            if (!result_.ok()) {
                return;
            }
            push(value);
            break;
        }
        case OpCode::StoreVar: {
            const std::uint16_t index = read_u16();
            const VmValue value = pop();
            if (!result_.ok()) {
                return;
            }
            store_global(index, value);
            break;
        }
        case OpCode::LoadLocal: {
            const std::uint8_t slot = read_u8();
            const VmValue value = load_local(slot);
            if (!result_.ok()) {
                return;
            }
            push(value);
            break;
        }
        case OpCode::StoreLocal: {
            const std::uint8_t slot = read_u8();
            const VmValue value = pop();
            if (!result_.ok()) {
                return;
            }
            store_local(slot, value);
            break;
        }
        case OpCode::Add:
        case OpCode::Sub:
        case OpCode::Mul:
        case OpCode::Div: {
            const VmValue right = pop();
            const VmValue left = pop();
            if (!result_.ok()) {
                return;
            }
            const VmValue computed = binary_int(op, left, right);
            if (!result_.ok()) {
                return;
            }
            push(computed);
            break;
        }
        case OpCode::Eq: {
            const VmValue right = pop();
            const VmValue left = pop();
            if (!result_.ok()) {
                return;
            }
            const VmValue computed = compare_eq(left, right);
            if (!result_.ok()) {
                return;
            }
            push(computed);
            break;
        }
        case OpCode::Ne: {
            const VmValue right = pop();
            const VmValue left = pop();
            if (!result_.ok()) {
                return;
            }
            const VmValue computed = compare_eq(left, right);
            if (!result_.ok()) {
                return;
            }
            push(!as_bool(computed));
            break;
        }
        case OpCode::Lt: {
            const VmValue right = pop();
            const VmValue left = pop();
            if (!result_.ok()) {
                return;
            }
            if (!is_int(left) || !is_int(right)) {
                fail("type mismatch: < requires two integers (got " +
                         value_type_name(left) + " and " + value_type_name(right) +
                         ")",
                     "use integer operands for ordering comparisons");
                return;
            }
            push(as_int(left) < as_int(right));
            break;
        }
        case OpCode::Gt: {
            const VmValue right = pop();
            const VmValue left = pop();
            if (!result_.ok()) {
                return;
            }
            if (!is_int(left) || !is_int(right)) {
                fail("type mismatch: > requires two integers (got " +
                         value_type_name(left) + " and " + value_type_name(right) +
                         ")",
                     "use integer operands for ordering comparisons");
                return;
            }
            push(as_int(left) > as_int(right));
            break;
        }
        case OpCode::Le: {
            const VmValue right = pop();
            const VmValue left = pop();
            if (!result_.ok()) {
                return;
            }
            if (!is_int(left) || !is_int(right)) {
                fail("type mismatch: <= requires two integers",
                     "use integer operands for ordering comparisons");
                return;
            }
            push(as_int(left) <= as_int(right));
            break;
        }
        case OpCode::Ge: {
            const VmValue right = pop();
            const VmValue left = pop();
            if (!result_.ok()) {
                return;
            }
            if (!is_int(left) || !is_int(right)) {
                fail("type mismatch: >= requires two integers",
                     "use integer operands for ordering comparisons");
                return;
            }
            push(as_int(left) >= as_int(right));
            break;
        }
        case OpCode::Call: {
            const std::uint16_t fn_index = read_u16();
            const std::uint8_t argc = read_u8();
            if (fn_index >= chunk_.functions.size()) {
                fail("call to unknown function index " + std::to_string(fn_index),
                     "recompile the program");
                return;
            }
            if (stack_.size() < argc) {
                fail("stack underflow while preparing function call",
                     "provide all required arguments");
                return;
            }
            const FunctionMeta& meta = chunk_.functions[fn_index];
            if (argc != meta.arity) {
                fail("argument count mismatch in CALL",
                     "recompile; bytecode may be corrupt");
                return;
            }
            std::vector<VmValue> args(argc);
            for (int i = static_cast<int>(argc) - 1; i >= 0; --i) {
                args[static_cast<std::size_t>(i)] = pop();
            }
            CallFrame frame;
            frame.return_ip = ip_;
            frame.locals.resize(meta.arity);
            for (std::uint8_t i = 0; i < argc; ++i) {
                frame.locals[i] = args[i];
            }
            frames_.push_back(std::move(frame));
            if (meta.address >= chunk_.code.size()) {
                fail("function entry address out of range",
                     "recompile the program");
                return;
            }
            ip_ = meta.address;
            break;
        }
        case OpCode::Return: {
            VmValue ret = pop();
            if (!result_.ok()) {
                return;
            }
            if (frames_.empty()) {
                fail("return outside of a function call",
                     "use return only inside fn bodies");
                return;
            }
            CallFrame frame = std::move(frames_.back());
            frames_.pop_back();
            ip_ = frame.return_ip;
            push(ret);
            break;
        }
        case OpCode::Neg: {
            const VmValue operand = pop();
            if (!result_.ok()) {
                return;
            }
            if (!is_int(operand)) {
                fail("type mismatch: unary '-' requires an integer (got " +
                         value_type_name(operand) + ")",
                     "negate numeric values only");
                return;
            }
            push(-as_int(operand));
            break;
        }
        case OpCode::Input: {
            push(read_input_value());
            break;
        }
        case OpCode::Print: {
            const VmValue value = pop();
            if (!result_.ok()) {
                return;
            }
            emit_output(value);
            break;
        }
        case OpCode::Jump: {
            const std::uint32_t target = read_u32();
            if (target >= chunk_.code.size()) {
                fail("jump target " + std::to_string(target) +
                         " is outside bytecode (size " +
                         std::to_string(chunk_.code.size()) + ")",
                     "recompile; jump offset may be corrupt");
                return;
            }
            ip_ = target;
            break;
        }
        case OpCode::JumpIfFalse: {
            const std::uint32_t target = read_u32();
            const VmValue cond = pop();
            if (!result_.ok()) {
                return;
            }
            if (!is_bool(cond)) {
                fail("type mismatch: conditional jump requires a boolean (got " +
                         value_type_name(cond) + ")",
                     "use == or < comparisons in if/while conditions");
                return;
            }
            if (target >= chunk_.code.size()) {
                fail("jump target " + std::to_string(target) +
                         " is outside bytecode (size " +
                         std::to_string(chunk_.code.size()) + ")",
                     "recompile; jump offset may be corrupt");
                return;
            }
            if (!as_bool(cond)) {
                ip_ = target;
            }
            break;
        }
        case OpCode::Halt:
            ip_ = chunk_.code.size();
            break;
        default:
            fail("unknown opcode 0x" +
                     std::to_string(static_cast<unsigned>(static_cast<std::uint8_t>(op))),
                 "bytecode may be corrupt; recompile the program");
            break;
    }
}

void VirtualMachine::run_loop() {
    constexpr std::size_t kMaxSteps = 1'000'000;
    std::size_t steps = 0;

    while (ip_in_bounds() && result_.ok()) {
        if (++steps > kMaxSteps) {
            fail("execution step limit exceeded (possible infinite loop)",
                 "check while loops and jump targets");
            return;
        }

        const auto opcode = static_cast<OpCode>(read_u8());
        if (!result_.ok()) {
            return;
        }

        execute_opcode(opcode);
    }

    if (result_.ok() && ip_ < chunk_.code.size()) {
        fail("execution ended before HALT opcode",
             "bytecode may be truncated");
    }
}

VmResult VirtualMachine::run(std::istream& input, std::ostream& output,
                             VmSession* session) {
    result_ = VmResult{};
    input_ = &input;
    output_ = &output;
    session_ = session;

    if (chunk_.code.empty()) {
        fail("cannot execute empty bytecode program", "compile a non-empty script");
        return result_;
    }

    ip_ = 0;
    stack_.clear();
    frames_.clear();
    if (session_ == nullptr) {
        globals_.clear();
        global_init_.clear();
    }

    run_loop();
    return result_;
}

void VirtualMachine::store_local(std::uint8_t slot, const VmValue& value) {
    if (frames_.empty()) {
        fail("STORE_LOCAL outside of function",
             "internal VM error — report this case");
        return;
    }
    auto& locals = frames_.back().locals;
    if (slot >= locals.size()) {
        locals.resize(slot + 1);
    }
    locals[slot] = value;
}

VmValue VirtualMachine::load_local(std::uint8_t slot) {
    if (frames_.empty()) {
        fail("LOAD_LOCAL outside of function",
             "internal VM error — report this case");
        return std::int64_t{0};
    }
    const auto& locals = frames_.back().locals;
    if (slot >= locals.size()) {
        fail("read of uninitialized local slot " + std::to_string(slot),
             "assign to the local before use");
        return std::int64_t{0};
    }
    return locals[slot];
}

VmResult execute(const BytecodeChunk& chunk, VmSession* session, std::istream& input,
                 std::ostream& output) {
    VirtualMachine vm(chunk);
    return vm.run(input, output, session);
}

}  // namespace cvm
