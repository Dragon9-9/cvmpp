#include "cvm++/compiler.hpp"

namespace cvm {
namespace {

OpCode binop_opcode(BinOp op) {
    switch (op) {
        case BinOp::Add:
            return OpCode::Add;
        case BinOp::Sub:
            return OpCode::Sub;
        case BinOp::Mul:
            return OpCode::Mul;
        case BinOp::Div:
            return OpCode::Div;
        case BinOp::Eq:
            return OpCode::Eq;
        case BinOp::Ne:
            return OpCode::Ne;
        case BinOp::Lt:
            return OpCode::Lt;
        case BinOp::Gt:
            return OpCode::Gt;
        case BinOp::Le:
            return OpCode::Le;
        case BinOp::Ge:
            return OpCode::Ge;
    }
    return OpCode::Add;
}

}  // namespace

Compiler::Compiler(const Program& program)
    : program_(program), writer_(chunk_) {}

void Compiler::error(const SourceLoc& loc, const std::string& message,
                     const std::string& hint) {
    Diagnostic d;
    d.phase = Phase::Compiler;
    d.severity = Severity::Error;
    d.message = message;
    d.loc = loc;
    d.hint = hint;
    bag_.push(std::move(d));
}

std::size_t Compiler::emit_jump(OpCode op) {
    writer_.emit(op);
    const std::size_t operand_offset = writer_.current_offset();
    writer_.emit_u32(0);
    return operand_offset;
}

void Compiler::patch_jump(std::size_t operand_offset, std::size_t target) {
    writer_.patch_u32(operand_offset, static_cast<std::uint32_t>(target));
}

void Compiler::begin_locals() {
    in_function_ = true;
    locals_.clear();
    next_local_slot_ = 0;
}

void Compiler::end_locals() {
    in_function_ = false;
    locals_.clear();
    next_local_slot_ = 0;
}

bool Compiler::is_local(const std::string& name) const {
    return in_function_ && locals_.find(name) != locals_.end();
}

std::uint8_t Compiler::declare_local(const std::string& name) {
    const auto it = locals_.find(name);
    if (it != locals_.end()) {
        return it->second;
    }
    const std::uint8_t slot = next_local_slot_++;
    locals_[name] = slot;
    return slot;
}

std::uint8_t Compiler::local_slot(const std::string& name) const {
    return locals_.at(name);
}

void Compiler::store_name(const std::string& name) {
    if (is_local(name)) {
        writer_.emit(OpCode::StoreLocal);
        writer_.emit_u8(local_slot(name));
        return;
    }
    const std::uint16_t index = writer_.intern_name(name);
    writer_.emit(OpCode::StoreVar);
    writer_.emit_u16(index);
}

void Compiler::load_name(const std::string& name) {
    if (is_local(name)) {
        writer_.emit(OpCode::LoadLocal);
        writer_.emit_u8(local_slot(name));
        return;
    }
    const std::uint16_t index = writer_.intern_name(name);
    writer_.emit(OpCode::LoadVar);
    writer_.emit_u16(index);
}

void Compiler::compile_literal(const IntLiteralExpr& expr) {
    writer_.emit(OpCode::PushInt);
    writer_.emit_i64(expr.value);
}

void Compiler::compile_literal(const BoolLiteralExpr& expr) {
    writer_.emit(OpCode::PushBool);
    writer_.emit_u8(expr.value ? 1 : 0);
}

void Compiler::compile_variable(const VariableExpr& expr) {
    load_name(expr.name);
}

void Compiler::compile_call(const CallExpr& expr) {
    std::uint16_t fn_index = 0;
    bool found = false;
    for (std::size_t i = 0; i < chunk_.functions.size(); ++i) {
        if (chunk_.functions[i].name == expr.callee) {
            fn_index = static_cast<std::uint16_t>(i);
            found = true;
            break;
        }
    }
    if (!found) {
        error(expr.loc, "call to undefined function '" + expr.callee + "'",
              "define the function with fn before calling it");
        return;
    }

    if (expr.arguments.size() > 255) {
        error(expr.loc, "too many arguments in function call",
              "split into smaller calls");
        return;
    }

    const FunctionMeta& meta = chunk_.functions[fn_index];
    if (expr.arguments.size() != meta.arity) {
        error(expr.loc,
              "function '" + expr.callee + "' expects " +
                  std::to_string(meta.arity) + " argument(s), got " +
                  std::to_string(expr.arguments.size()),
              "match the parameter count in the fn declaration");
        return;
    }

    for (const auto& arg : expr.arguments) {
        if (arg) {
            compile_expression(*arg);
        }
    }

    writer_.emit(OpCode::Call);
    writer_.emit_u16(fn_index);
    writer_.emit_u8(static_cast<std::uint8_t>(expr.arguments.size()));
}

void Compiler::compile_input(const InputExpr& expr) {
    (void)expr;
    writer_.emit(OpCode::Input);
}

void Compiler::compile_unary(const UnaryExpr& expr) {
    if (!expr.operand) {
        return;
    }
    compile_expression(*expr.operand);
    if (expr.op == UnaryOp::Neg) {
        writer_.emit(OpCode::Neg);
    }
}

void Compiler::compile_binary(const BinaryExpr& expr) {
    if (!expr.left || !expr.right) {
        return;
    }
    compile_expression(*expr.left);
    compile_expression(*expr.right);
    writer_.emit(binop_opcode(expr.op));
}

void Compiler::compile_expression(const Expr& expr) {
    if (const auto* lit = dynamic_cast<const IntLiteralExpr*>(&expr)) {
        compile_literal(*lit);
        return;
    }
    if (const auto* lit = dynamic_cast<const BoolLiteralExpr*>(&expr)) {
        compile_literal(*lit);
        return;
    }
    if (const auto* var = dynamic_cast<const VariableExpr*>(&expr)) {
        compile_variable(*var);
        return;
    }
    if (const auto* call = dynamic_cast<const CallExpr*>(&expr)) {
        compile_call(*call);
        return;
    }
    if (const auto* input = dynamic_cast<const InputExpr*>(&expr)) {
        compile_input(*input);
        return;
    }
    if (const auto* unary = dynamic_cast<const UnaryExpr*>(&expr)) {
        compile_unary(*unary);
        return;
    }
    if (const auto* binary = dynamic_cast<const BinaryExpr*>(&expr)) {
        compile_binary(*binary);
        return;
    }
    error(expr.loc, "unsupported expression in compiler",
          "this expression form is not yet lowered to bytecode");
}

void Compiler::compile_block(const BlockStmt& block) {
    for (const auto& stmt : block.statements) {
        if (stmt) {
            compile_statement(*stmt);
        }
    }
}

void Compiler::compile_if(const IfStmt& stmt) {
    if (!stmt.condition) {
        return;
    }
    compile_expression(*stmt.condition);
    const std::size_t jump_false_operand = emit_jump(OpCode::JumpIfFalse);

    if (stmt.then_branch) {
        compile_statement(*stmt.then_branch);
    }

    std::size_t jump_end_operand = 0;
    if (stmt.else_branch) {
        jump_end_operand = emit_jump(OpCode::Jump);
    }

    patch_jump(jump_false_operand, writer_.current_offset());

    if (stmt.else_branch) {
        compile_statement(*stmt.else_branch);
        patch_jump(jump_end_operand, writer_.current_offset());
    }
}

void Compiler::compile_while(const WhileStmt& stmt) {
    if (!stmt.condition) {
        return;
    }
    const std::size_t loop_start = writer_.current_offset();
    compile_expression(*stmt.condition);
    const std::size_t exit_jump_operand = emit_jump(OpCode::JumpIfFalse);

    if (stmt.body) {
        compile_statement(*stmt.body);
    }

    writer_.emit(OpCode::Jump);
    writer_.emit_u32(static_cast<std::uint32_t>(loop_start));

    patch_jump(exit_jump_operand, writer_.current_offset());
}

void Compiler::compile_statement(const Stmt& stmt) {
    if (const auto* let = dynamic_cast<const LetStmt*>(&stmt)) {
        if (!let->initializer) {
            return;
        }
        compile_expression(*let->initializer);
        if (in_function_) {
            declare_local(let->name);
        }
        store_name(let->name);
        return;
    }

    if (const auto* assign = dynamic_cast<const AssignStmt*>(&stmt)) {
        if (!assign->value) {
            return;
        }
        compile_expression(*assign->value);
        store_name(assign->name);
        return;
    }

    if (const auto* ret = dynamic_cast<const ReturnStmt*>(&stmt)) {
        if (!ret->value) {
            error(ret->loc, "return requires a value", "use: return expression;");
            return;
        }
        compile_expression(*ret->value);
        writer_.emit(OpCode::Return);
        return;
    }

    if (const auto* print_s = dynamic_cast<const PrintStmt*>(&stmt)) {
        if (!print_s->expression) {
            return;
        }
        compile_expression(*print_s->expression);
        writer_.emit(OpCode::Print);
        return;
    }

    if (const auto* expr_s = dynamic_cast<const ExprStmt*>(&stmt)) {
        if (!expr_s->expression) {
            return;
        }
        compile_expression(*expr_s->expression);
        writer_.emit(OpCode::Pop);
        return;
    }

    if (const auto* if_s = dynamic_cast<const IfStmt*>(&stmt)) {
        compile_if(*if_s);
        return;
    }

    if (const auto* while_s = dynamic_cast<const WhileStmt*>(&stmt)) {
        compile_while(*while_s);
        return;
    }

    if (const auto* block = dynamic_cast<const BlockStmt*>(&stmt)) {
        compile_block(*block);
        return;
    }

    error(stmt.loc, "unsupported statement in compiler",
          "this statement form is not yet lowered to bytecode");
}

void Compiler::compile_function(const FunctionDecl& fn) {
    if (!fn.body) {
        error(fn.loc, "function '" + fn.name + "' has no body",
              "add a block body: fn f() { ... }");
        return;
    }

    FunctionMeta meta;
    meta.name = fn.name;
    meta.address = static_cast<std::uint32_t>(writer_.current_offset());
    meta.arity = static_cast<std::uint8_t>(fn.parameters.size());
    chunk_.functions.push_back(meta);

    begin_locals();
    for (const auto& param : fn.parameters) {
        declare_local(param);
    }

    compile_block(*fn.body);
    end_locals();
}

void Compiler::compile_program() {
    const std::size_t skip_functions = emit_jump(OpCode::Jump);

    for (const auto& fn : program_.functions) {
        if (fn) {
            compile_function(*fn);
        }
    }

    const std::size_t main_entry = writer_.current_offset();
    patch_jump(skip_functions, main_entry);

    for (const auto& stmt : program_.statements) {
        if (stmt) {
            compile_statement(*stmt);
        }
    }
    writer_.emit(OpCode::Halt);
}

CompileResult Compiler::compile() {
    CompileResult result;
    try {
        compile_program();
        result.chunk = std::move(chunk_);
    } catch (const std::exception& ex) {
        error(program_.loc, std::string("compiler internal failure: ") + ex.what(),
              "reduce program size or report a bug");
    }
    result.diagnostics = std::move(bag_);
    return result;
}

}  // namespace cvm
