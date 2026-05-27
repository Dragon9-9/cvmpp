# ⚙️ CVM++ — Custom Scripting Language, Bytecode Compiler & Stack VM

**Personal systems project** · C++17 · Recursive-descent parser · Stack bytecode VM  
**Author:** Satish Kumar Munda ([@Dragon9-9](https://github.com/Dragon9-9))

---

## 📋 System Summary

CVM++ is an end-to-end interpreter toolchain: you write `.cvm` scripts, the program **tokenizes** source, **parses** into an AST, **compiles** to compact stack bytecode (`std::vector<uint8_t>`), and **executes** on a custom virtual machine with structured diagnostics and a terminal REPL.

```
  .cvm source  →  Lexer  →  Parser  →  Compiler  →  Stack VM  →  stdout
                  tokens      AST       bytecode
```

---

## 📊 Current Project Status

| Component | Description | Verification |
|-----------|-------------|:------------:|
| **Lexer** | Keywords, identifiers, integers, operators (`==`, `!=`, `<=`, `>=`), `//` comments, overflow checks | ✅ |
| **Parser** | Recursive-descent AST; `let`, `if`/`else`, `while`, `fn`/`return`, calls | ✅ |
| **Compiler** | AST → bytecode; jump patching; function table; entry `JUMP` over `main` | ✅ |
| **Stack VM** | Arithmetic, comparisons, globals, call frames, `CALL`/`RETURN` | ✅ |
| **CLI / REPL** | File runner, `-d`/`-q`/`-c`, multiline `{ }`, `:disasm` | ✅ |
| **CI** | GitHub Actions — `make` + `make verify` on push | ✅ |

**Total:** **11/11** example scripts passing (`make verify`)

---

## 🏗️ Design Highlights

### Phase 1 — Lexer & diagnostics

- Hand-written scanner → `Token` stream with `SourceLoc`
- Integer overflow, invalid characters, null-byte rejection
- Phase-tagged errors: `LEXER`, `PARSER`, `COMPILER`, `VM`

### Phase 2 — Parser & AST

- `Program`: top-level `fn` declarations + main statements
- Expressions: literals, variables, unary `-`, binary ops, `input`, calls
- Panic-mode recovery on syntax errors

### Phase 3 — Bytecode compiler

- Opcodes: `PUSH_*`, `LOAD/STORE_VAR`, `LOAD/STORE_LOCAL`, `ADD`…`GE`, `JUMP*`, `CALL`, `RETURN`, `PRINT`, `HALT`
- Name interning pool; forward jump patching for `if` / `while`
- Functions emitted first; **offset-0 `JUMP`** skips to `main`

### Phase 4 — Stack virtual machine

- `std::variant<int64_t, bool>` values; max stack depth 65536
- Runtime guards: divide-by-zero, stack under/overflow, uninitialized vars, step limit (1M)
- REPL: persistent globals via `VmSession`

### Phase 5 — Tooling

- `cvmpp script.cvm` · debug tables (`-d`) · bytecode disasm (`:disasm`)
- Example suite under `examples/` · `scripts/verify.sh`

---

## 📁 Repository Structure

```
cvmpp/
├── include/cvm++/          # Public headers
│   ├── lexer.hpp           # Character scanner
│   ├── parser.hpp          # Token → AST
│   ├── ast.hpp             # Expression / statement nodes
│   ├── compiler.hpp        # AST → bytecode
│   ├── opcode.hpp          # Instruction set
│   ├── bytecode.hpp        # Byte buffer + disassembler
│   ├── vm.hpp              # Stack machine + call frames
│   └── compile.hpp         # Front-end orchestration
├── src/                    # Implementations (.cpp)
├── examples/               # Sample .cvm programs
├── docs/
│   ├── pdf/                # Full PDF guides (build + architecture)
│   └── guides/             # Markdown sources for PDFs
├── scripts/
│   ├── verify.sh           # Regression tests (11 scripts)
│   └── build-docs.sh       # Regenerate PDFs (pandoc)
├── .github/workflows/ci.yml
├── Makefile
└── CMakeLists.txt
```

---

## 🏃 Quickstart

```bash
# Clone
git clone https://github.com/Dragon9-9/cvmpp.git
cd cvmpp

# Build
make

# Run
./build/cvmpp examples/hello.cvm
./build/cvmpp examples/functions.cvm    # recursive factorial → 120
./build/cvmpp -d examples/hello.cvm     # tokens + AST + bytecode

# REPL
./build/cvmpp

# Verify all examples
make verify
```

**CMake (optional):**

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
```

**Exit codes:** `0` ok · `1` compile error · `2` runtime error

---

## 📊 Verification Results

```
hello.cvm           PASSED  (let, if, print)
arithmetic.cvm      PASSED  (+ - * / == < >)
booleans.cvm        PASSED  (true / false)
if_else.cvm         PASSED  (else branch)
factorial.cvm       PASSED  (while loop)
multiline_demo.cvm  PASSED  (block sum)
assignment.cvm      PASSED  (reassignment)
functions.cvm       PASSED  (fn / return / recursion)
comparisons.cvm     PASSED  (!= <= >=)
input_demo.cvm      PASSED  (stdin input)
div_by_zero.cvm     PASSED  (runtime error, exit 2)

TOTAL               11/11 PASSED
```

---

## 📖 Documentation

| Guide | Contents |
|-------|----------|
| [Project & Build (PDF)](docs/pdf/CVM++_01_Project_and_Build_Guide.pdf) | Setup, repo map, construction steps, glossary |
| [Architecture & Workflow (PDF)](docs/pdf/CVM++_02_Architecture_and_Workflow_Guide.pdf) | Pipeline, opcodes, AST, execution traces |

---

## 📜 License

MIT — see [LICENSE](LICENSE).
