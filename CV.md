# CVM++ — Resume / CV bullets

Use these when listing the project on a resume, LinkedIn, or portfolio site.

## One-liner

**CVM++** — Built a C++17 scripting language with lexer, AST parser, bytecode compiler, and stack VM; includes functions, REPL, and structured multi-phase error reporting.

## Bullet points (pick 2–4)

- Designed and implemented an end-to-end **compiler + virtual machine** in C++17: lexical analysis, recursive-descent parsing, AST lowering, and stack bytecode execution.

- Defined a custom **instruction set** (stack operations, control-flow jumps, function call/return) with bytecode stored in `std::vector<uint8_t>` for efficient dispatch.

- Implemented **user-defined functions** with parameters, local slots, and recursive calls; added comparison operators and compile/runtime **failsafes** (divide-by-zero, stack bounds, type checks).

- Built a **terminal REPL and CLI runner** with ANSI-styled diagnostics, optional token/AST/bytecode debug views, and multiline script input.

- Applied modern C++ practices: `std::unique_ptr` for AST ownership, `std::variant` for typed runtime values, CMake/Makefile builds, and automated example verification.

## Skills to tag

`C++` · `Compilers` · `Interpreters` · `Virtual Machines` · `Data Structures` · `CMake` · `Systems Programming` · `Parsing` · `CLI Tools`

## Interview talking points

1. **Pipeline:** source → tokens → AST → bytecode → VM (draw on whiteboard).
2. **Why stack VM:** simple codegen, easy function calls with frames.
3. **Hardest bug:** function bodies at start of bytecode — fixed with entry `JUMP` to `main`.
4. **Error design:** phase-tagged diagnostics (lexer/parser/VM) without terminating the process on user errors.

## Documentation (GitHub)

- [Project & Build Guide PDF](docs/pdf/CVM++_01_Project_and_Build_Guide.pdf) — setup, glossary, construction steps
- [Architecture & Workflow PDF](docs/pdf/CVM++_02_Architecture_and_Workflow_Guide.pdf) — full runtime flow and opcode reference
