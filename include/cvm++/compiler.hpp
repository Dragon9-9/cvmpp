#pragma once

#include "cvm++/ast.hpp"
#include "cvm++/bytecode.hpp"
#include "cvm++/diagnostic.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace cvm {

struct CompileResult {
    BytecodeChunk chunk;
    DiagnosticBag diagnostics;

    bool ok() const { return !diagnostics.has_errors(); }
};

class Compiler {
public:
    explicit Compiler(const Program& program);

    CompileResult compile();

private:
    const Program& program_;
    BytecodeChunk chunk_;
    BytecodeWriter writer_;
    DiagnosticBag bag_;

    bool in_function_{false};
    std::unordered_map<std::string, std::uint8_t> locals_;
    std::uint8_t next_local_slot_{0};

    std::size_t emit_jump(OpCode op);
    void patch_jump(std::size_t operand_offset, std::size_t target);

    void error(const SourceLoc& loc, const std::string& message,
               const std::string& hint = {});

    void begin_locals();
    void end_locals();
    bool is_local(const std::string& name) const;
    std::uint8_t declare_local(const std::string& name);
    std::uint8_t local_slot(const std::string& name) const;

    void store_name(const std::string& name);
    void load_name(const std::string& name);

    void compile_program();
    void compile_function(const FunctionDecl& fn);
    void compile_statement(const Stmt& stmt);
    void compile_block(const BlockStmt& block);
    void compile_expression(const Expr& expr);

    void compile_literal(const IntLiteralExpr& expr);
    void compile_literal(const BoolLiteralExpr& expr);
    void compile_variable(const VariableExpr& expr);
    void compile_call(const CallExpr& expr);
    void compile_input(const InputExpr& expr);
    void compile_unary(const UnaryExpr& expr);
    void compile_binary(const BinaryExpr& expr);

    void compile_if(const IfStmt& stmt);
    void compile_while(const WhileStmt& stmt);
};

}  // namespace cvm
