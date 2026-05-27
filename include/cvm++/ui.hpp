#pragma once

#include "cvm++/ast.hpp"
#include "cvm++/bytecode.hpp"
#include "cvm++/diagnostic.hpp"
#include "cvm++/token.hpp"

#include <string>
#include <vector>

namespace cvm::ui {

namespace ansi {
constexpr const char* reset = "\033[0m";
constexpr const char* bold = "\033[1m";
constexpr const char* dim = "\033[2m";
constexpr const char* red = "\033[31m";
constexpr const char* green = "\033[32m";
constexpr const char* yellow = "\033[33m";
constexpr const char* blue = "\033[34m";
constexpr const char* magenta = "\033[35m";
constexpr const char* cyan = "\033[36m";
constexpr const char* white = "\033[37m";
constexpr const char* bright_green = "\033[92m";
constexpr const char* bright_cyan = "\033[96m";
constexpr const char* bright_red = "\033[91m";
constexpr const char* bright_yellow = "\033[93m";
}  // namespace ansi

bool stdout_is_tty();

void print_logo();
void print_welcome(bool debug_mode);
void print_prompt();
void print_continuation_prompt(int brace_depth);
void print_success(const std::string& message);
void print_version();
void clear_screen();
void print_info(const std::string& message);

void print_diagnostics(const DiagnosticBag& bag, const std::string& source);
void print_token_table(const std::vector<Token>& tokens);
void print_ast_tree(const Program& program);
void print_bytecode_table(const BytecodeChunk& chunk);
void print_runtime_output(const std::vector<std::string>& lines);

void print_help();

}  // namespace cvm::ui
