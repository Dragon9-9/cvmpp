# CVM++

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C++-17-00599C?logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![CI](https://github.com/Dragon9-9/cvmpp/actions/workflows/ci.yml/badge.svg)](https://github.com/Dragon9-9/cvmpp/actions/workflows/ci.yml)

A **from-scratch scripting language**, **bytecode compiler**, and **stack virtual machine** in C++17 — with a polished terminal REPL and full documentation.

```
  .cvm source  →  Lexer  →  Parser  →  Compiler  →  Stack VM  →  output
                  tokens      AST       bytecode
```

> **New here?** Read the [**Project & Build Guide (PDF)**](docs/pdf/CVM++_01_Project_and_Build_Guide.pdf) for setup, glossary, and how the project is structured.  
> **How does it work?** See the [**Architecture & Workflow Guide (PDF)**](docs/pdf/CVM++_02_Architecture_and_Workflow_Guide.pdf).

---

## Features

- Full pipeline: lexer → recursive-descent parser → AST → bytecode (`std::vector<uint8_t>`) → stack VM
- Language: integers, booleans, `let`, functions (`fn` / `return`), `if` / `while`, `print`, `input`
- Operators: `+ - * / == != < > <= >=`
- Structured diagnostics (lexer / parser / compiler / VM) with source locations
- REPL with multiline blocks, debug disassembly, persistent globals
- CLI: `cvmpp script.cvm` with `-d` `-q` `-c` flags
- CI via GitHub Actions · `make verify` regression suite

---

## Quick start

```bash
git clone https://github.com/Dragon9-9/cvmpp.git
cd cvmpp
make
make verify
./build/cvmpp examples/hello.cvm
./build/cvmpp -d examples/functions.cvm
./build/cvmpp    # REPL
```

---

## Example

```cvm
fn factorial(n) {
  if (n <= 1) {
    return 1;
  }
  return n * factorial(n - 1);
}
print factorial(5);   // 120
```

---

## Documentation

| Resource | Description |
|----------|-------------|
| [docs/pdf/01 — Project & Build Guide](docs/pdf/CVM++_01_Project_and_Build_Guide.pdf) | Clone, build, repo map, construction steps, **glossary** |
| [docs/pdf/02 — Architecture & Workflow](docs/pdf/CVM++_02_Architecture_and_Workflow_Guide.pdf) | End-to-end flow, opcodes, AST, traces |
| [docs/README.md](docs/README.md) | Doc index and PDF rebuild instructions |
| [CV.md](CV.md) | Resume bullet points |

---

## Project layout

```
include/cvm++/   Headers (lexer, parser, ast, compiler, vm, …)
src/             Implementations
examples/        Sample .cvm programs
scripts/         verify.sh, build-docs.sh
docs/            Guides (Markdown + PDF)
```

---

## REPL commands

| Command | Action |
|---------|--------|
| `help` / `:help` | Command list |
| `run file.cvm` | Execute a script |
| `disasm` / `:disasm [file]` | Show bytecode |
| `debug` | Toggle token / AST / bytecode views |
| `quit` | Exit |

---

## Build options

| Command | Description |
|---------|-------------|
| `make` | Build `build/cvmpp` |
| `make verify` | Run all example scripts |
| `make clean` | Remove binary |
| `cmake -B build && cmake --build build` | CMake build |

**Exit codes:** `0` success · `1` compile error · `2` runtime error

---

## Examples

| File | Shows |
|------|-------|
| `examples/hello.cvm` | Basics |
| `examples/functions.cvm` | Recursion |
| `examples/factorial.cvm` | `while` |
| `examples/comparisons.cvm` | `!=` `<=` `>=` |
| `examples/div_by_zero.cvm` | Runtime error handling |

---

## Tech stack

C++17 · CMake · Makefile · ANSI terminal UI · Stack bytecode VM

Inspired by [*Crafting Interpreters*](https://craftinginterpreters.com/) (Robert Nystrom).

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).

---

## License

[MIT](LICENSE) — free to use, modify, and distribute with attribution.
