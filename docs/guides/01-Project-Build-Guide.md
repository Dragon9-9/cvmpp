---
title: "CVM++ Project & Build Guide"
subtitle: "From Zero to Running - Setup, Structure, Definitions, and Step-by-Step Construction"
author: "CVM++ Documentation"
date: "May 2026"
---

# About This Document

This guide is written for **anyone** opening the CVM++ repository: students, reviewers, interviewers, or future you. It explains **what the project is**, **how to build and run it**, **how the repository is organized**, the **recommended order for building each part**, and a **complete glossary** of technical terms used throughout the codebase.

**Companion document:** *CVM++ Architecture & Workflow Guide* (PDF) — explains how code flows through the system at runtime.

---

# 1. What Is CVM++?

**CVM++** is a small **scripting language** plus a **toolchain** written in **C++17**:

| Piece | Role |
|-------|------|
| **Language** | `.cvm` text files with variables, math, logic, functions, `if`/`while`, `print`, `input` |
| **Lexer** | Turns source text into **tokens** |
| **Parser** | Turns tokens into an **AST** (tree of meaning) |
| **Compiler** | Turns AST into **bytecode** (`std::vector<uint8_t>`) |
| **Virtual Machine (VM)** | Executes bytecode on a **stack** |
| **CLI / REPL** | Terminal program `cvmpp` to run scripts interactively |

You are **not** using LLVM, Python, or an off-the-shelf interpreter library. The pipeline is implemented directly in this repository.

---

# 2. Prerequisites

## 2.1 Required software

| Tool | Minimum | Purpose |
|------|---------|---------|
| **C++ compiler** | GCC 9+, Clang 10+, or MSVC 2019+ | Build `cvmpp` |
| **Make** | Any | `make` / `make verify` |
| **CMake** | 3.16+ | Optional alternative build |
| **Git** | Any | Clone and version control |

## 2.2 Optional (documentation)

| Tool | Purpose |
|------|---------|
| **Pandoc** + **LaTeX** (TeX Live / MacTeX) | Regenerate PDF guides: `./scripts/build-docs.sh` |

## 2.3 Knowledge assumed

- Basic command line (`cd`, `ls`, running executables)
- Very basic C++ (classes, `std::vector`, `std::string`) if you plan to modify the code

No prior compiler-course background is required to **run** the project.

---

# 3. Quick Start (Clone → Build → Run)

## 3.1 Clone the repository

```bash
git clone https://github.com/Dragon9-9/cvmpp.git
cd cvmpp
```

## 3.2 Build

**Option A — Make (recommended):**

```bash
make
```

Produces: `build/cvmpp`

**Option B — CMake:**

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

On Windows, use **WSL** or **MSVC** with the same commands inside a developer shell.

## 3.3 Run a program

```bash
./build/cvmpp examples/hello.cvm
./build/cvmpp -d examples/functions.cvm    # debug: tokens, AST, bytecode
./build/cvmpp -q examples/factorial.cvm    # quiet output only
./build/cvmpp                              # interactive REPL
```

## 3.4 Verify everything works

```bash
make verify
```

Expected: `11 passed, 0 failed`.

---

# 4. Repository Structure

Every top-level path and its purpose:

```
cvmpp/
├── README.md                 Project overview (start here on GitHub)
├── LICENSE                   MIT license
├── CV.md                     Resume bullet points (optional for readers)
├── CONTRIBUTING.md           How to contribute / report issues
├── Makefile                  make, make verify, make clean
├── CMakeLists.txt            CMake build definition
├── .gitignore                Files Git should ignore
├── .github/workflows/ci.yml  GitHub Actions: build + verify
│
├── include/cvm++/           Public C++ headers (API of each module)
├── src/                     Implementations (.cpp)
├── examples/                Sample .cvm programs
├── scripts/
│   ├── verify.sh            Automated example tests
│   └── build-docs.sh        Build PDF documentation
└── docs/
    ├── README.md            Documentation index
    ├── guides/              Markdown sources for PDFs
    ├── pdf/                 Generated PDFs (committed for GitHub)
    └── pandoc/              PDF styling defaults
```

## 4.1 Source modules (`src/` ↔ `include/cvm++/`)

| File(s) | Module | Responsibility |
|---------|--------|----------------|
| `lexer.cpp` / `lexer.hpp` | Lexer | Characters → tokens |
| `token.cpp` / `token.hpp` | Token | Token types and names |
| `parser.cpp` / `parser.hpp` | Parser | Tokens → AST |
| `ast.cpp`, `ast.hpp`, `ast_print.cpp` | AST | Tree nodes + debug printer |
| `compiler.cpp` / `compiler.hpp` | Compiler | AST → bytecode |
| `opcode.cpp` / `opcode.hpp` | Opcode | Instruction enum + names |
| `bytecode.cpp` / `bytecode.hpp` | Bytecode | Byte buffer, disassembler |
| `vm.cpp` / `vm.hpp` | VM | Execute bytecode |
| `value.cpp` / `value.hpp` | Value | Runtime `int` / `bool` |
| `compile.cpp` / `compile.hpp` | Frontend | Wires lexer→parser→compiler→VM |
| `diagnostic.cpp` / `diagnostic.hpp` | Diagnostics | Errors by phase |
| `ui.cpp` / `ui.hpp` | UI | Colors, tables, help text |
| `main.cpp` | CLI | REPL, file runner, flags |

The **executable** `cvmpp` links all of the above into one binary.

---

# 5. Step-by-Step: How This Project Was Built

This section is the **construction roadmap** — the order used to implement a working interpreter. If you rebuild or extend CVM++, follow this sequence.

## Step 1 — Foundation

**Goal:** Locations, errors, tokens, and a minimal `main`.

1. `source_loc.hpp` — line/column for error messages  
2. `diagnostic.hpp` — `Phase`, `Severity`, `DiagnosticBag`  
3. `token.hpp` — `TokenType`, `Token`  
4. `lexer.hpp` — scan identifiers, numbers, operators, keywords  
5. `ui.hpp` — colored printing  
6. `main.cpp` — read a line, tokenize, print token table  

**Definition checkpoint:** A **token** is one classified chunk of source (e.g. keyword `let`, number `42`).

## Step 2 — Parser and AST

**Goal:** Validate grammar and build a tree.

1. `ast.hpp` — `Expr`, `Stmt`, `Program`, literals, `if`, `while`, `let`, `print`  
2. `parser.hpp` — recursive-descent parsing  
3. `ast_print.cpp` — print tree for debugging  

**Checkpoint:** **AST** = Abstract Syntax Tree; it stores *structure*, not text.

## Step 3 — Bytecode and compiler

**Goal:** Lower AST to portable bytes.

1. `opcode.hpp` — instruction set (`PUSH_INT`, `ADD`, `JUMP`, …)  
2. `bytecode.hpp` — `BytecodeChunk`, `BytecodeWriter`, disassembler  
3. `compiler.hpp` — walk AST, emit opcodes, patch jump targets  

**Checkpoint:** **Bytecode** = numeric instructions the VM understands.

## Step 4 — Virtual machine

**Goal:** Run programs.

1. `value.hpp` — `std::variant<int64_t, bool>`  
2. `vm.hpp` — stack, globals, instruction loop, failsafes  
3. `compile.hpp` — `compile_frontend()`, `execute()`  

**Checkpoint:** **VM** = program that simulates a CPU for your bytecode.

## Step 5 — Product polish

**Goal:** Usable tool for demos and portfolio.

1. REPL with `:help`, `:run`, `:disasm`, multiline `{ }`  
2. File mode: `cvmpp script.cvm`, flags `-d -q -c`  
3. `examples/*.cvm` + `scripts/verify.sh`  
4. Functions: `fn`, `return`, `CALL` / `RETURN`, locals  
5. CI on GitHub Actions  

---

# 6. Build System Details

## 6.1 Makefile targets

| Target | Command | Effect |
|--------|---------|--------|
| `all` (default) | `make` | Build `build/cvmpp` |
| `verify` | `make verify` | Run all example scripts |
| `clean` | `make clean` | Remove binary |
| `repl` | `make repl` | Start REPL |
| `cmake` | `make cmake` | Configure + build via CMake |

## 6.2 Compiler flags

- `-std=c++17` — language standard  
- `-Wall -Wextra -Wpedantic` — strict warnings  
- `-Iinclude` — header search path  

## 6.3 Exit codes (file mode)

| Code | Meaning |
|------|---------|
| `0` | Success |
| `1` | Lexer, parser, or compiler error |
| `2` | Runtime VM error (e.g. divide by zero) |

---

# 7. CVM++ Language Reference (Short)

```cvm
let x = 10;
x = x + 1;
print x;

if (x < 20) {
  print true;
} else {
  print false;
}

while (x > 0) {
  x = x - 1;
}

fn add(a, b) {
  return a + b;
}
print add(3, 4);

let name = input;   // reads one line from stdin
```

| Construct | Syntax |
|-----------|--------|
| Integers | `0`, `42`, `1000` |
| Booleans | `true`, `false` |
| Declare | `let name = expr;` |
| Assign | `name = expr;` |
| Function | `fn name(p1, p2) { ... }` |
| Return | `return expr;` |
| Call | `name(arg1, arg2);` |
| Operators | `+ - * / == != < > <= >=` |
| Unary minus | `-x` |
| Comment | `// to end of line` |

**Not supported:** strings, floats, arrays, `for`, classes, modules.

---

# 8. Complete Glossary

Terms are listed alphabetically. Every term appears somewhere in the project or PDF workflow guide.

| Term | Definition |
|------|------------|
| **Abstract Syntax Tree (AST)** | Tree data structure representing program structure after parsing. Nodes: expressions and statements. |
| **Arity** | Number of parameters a function accepts. |
| **Bytecode** | Sequence of bytes (`std::vector<uint8_t>`) encoding VM instructions. |
| **BytecodeChunk** | Struct holding `code`, `names` (variable pool), and `functions` metadata. |
| **Call frame** | VM record of return address + local variables during a function call. |
| **CLI** | Command-line interface; running `cvmpp` with a file path. |
| **Compiler** | Module that translates AST to bytecode (not a separate executable). |
| **Control flow** | `if`, `else`, `while`, and jumps in bytecode. |
| **Diagnostic** | Structured error/warning with phase, location, message, hint. |
| **Disassembler** | Tool that prints human-readable opcode listing from bytecode. |
| **Front end** | Lexer + parser + compiler together (`compile_frontend`). |
| **Function table** | `BytecodeChunk::functions` — name, entry address, arity. |
| **Global variable** | Variable stored outside functions; persists in REPL via `VmSession`. |
| **Identifier** | Name for variable or function (`x`, `factorial`). |
| **Instruction pointer (IP)** | Index into bytecode array; advances as VM runs. |
| **ISA** | Instruction Set Architecture — list of opcodes the VM supports. |
| **Jump patching** | Compiler writes placeholder offset, fills real target after label is known. |
| **Lexer** | Scanner converting source text to tokens. |
| **Literal** | Fixed value in source: integer or boolean. |
| **Local variable** | Slot inside a function (`LOAD_LOCAL` / `STORE_LOCAL`). |
| **Name pool** | `chunk.names` — interned variable names referenced by index. |
| **Opcode** | One-byte instruction tag (e.g. `Add`, `Call`). |
| **Operand** | Extra bytes after opcode (e.g. jump offset, name index). |
| **Parser** | Builds AST from token stream; reports syntax errors. |
| **Phase** | Stage tag on diagnostics: LEXER, PARSER, COMPILER, VM, REPL. |
| **REPL** | Read-Eval-Print Loop; interactive `cvmpp` without script file. |
| **Recursive descent** | Parsing style: one function per grammar rule. |
| **Runtime** | VM execution phase (after compile succeeds). |
| **Source location** | Line and column (`SourceLoc`) for error caret display. |
| **Stack** | LIFO structure holding intermediate expression values in VM. |
| **Stack machine** | VM design: operations consume/produce stack slots. |
| **Token** | Lexeme + type + source range (e.g. `Integer "42"`). |
| **Type (runtime)** | Only `int64_t` or `bool` in `VmValue` variant. |
| **Virtual Machine (VM)** | Interpreter loop executing bytecode. |
| **VmSession** | REPL-only map keeping global variables between lines. |

---

# 9. Examples Catalog

| File | Demonstrates |
|------|----------------|
| `hello.cvm` | `let`, `if`, `print` |
| `arithmetic.cvm` | Operators |
| `booleans.cvm` | `true` / `false` |
| `if_else.cvm` | Branches |
| `factorial.cvm` | `while` loop |
| `functions.cvm` | Recursive `fn` / `return` |
| `comparisons.cvm` | `!=` `<=` `>=` |
| `assignment.cvm` | Reassignment |
| `multiline_demo.cvm` | Block summation |
| `input_demo.cvm` | stdin `input` |
| `div_by_zero.cvm` | Runtime error (exit 2) |

---

# 10. Troubleshooting

| Problem | Solution |
|---------|----------|
| `cvmpp: command not found` | Run from repo root; use `./build/cvmpp` or `make` first |
| Build fails on C++ | Install Xcode CLI tools (macOS) or `build-essential` (Linux) |
| `make verify` fails one script | Run `./build/cvmpp -d examples/<file>.cvm` for diagnostics |
| PDF build fails | Install `pandoc` and MacTeX/TeX Live; run `./scripts/build-docs.sh` |
| REPL variables “lost” | File runs reset VM; REPL keeps `VmSession` globals |

---

# 11. Publishing to GitHub (Checklist)

1. Create empty repo on GitHub (e.g. `cvmpp`).  
2. `git init && git add . && git commit -m "Initial public release"`  
3. `git remote add origin git@github.com:Dragon9-9/cvmpp.git`  
4. `git push -u origin main`  
5. Enable **Actions** tab — CI should pass on push.  
6. Pin **README** and link both PDFs under `docs/pdf/`.  

---

# 12. Further Reading

- *Crafting Interpreters* — Robert Nystrom (conceptual background)  
- **Architecture & Workflow Guide (PDF)** — byte-level execution trace in this repo  
- Source entry: `src/compile.cpp` → `compile_frontend()`  

---

*CVM++ — MIT License — Documentation generated for public repository release.*
