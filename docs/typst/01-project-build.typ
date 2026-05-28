#import "cvm-theme.typ": *
#import "cvm-figures.typ": ast-tree, fig-call-protocol, fig-compile-runtime, fig-jump-patch, fig-while-flow, fig-phase-timeline

#doc-setup(header-label: [Project and Build Guide])
#doc-title([Project and Build])

CVM++ is a programming language and interpreter shipped as a single program called `cvmpp`. Source
code lives in `.cvm` files. The interpreter reads a file, turns it into bytecode, and runs that
bytecode on a small stack virtual machine. Everything is implemented in C++17 without parser
generators or LLVM.

= Pipeline and runtime

== End-to-end flow

When we run `cvmpp examples/hello.cvm`:

+ The *driver* (`main.cpp`) reads the file from disk.
+ `compile_frontend()` in `compile.cpp` runs the *lexer*, *parser*, and *compiler*.
+ The result is a `BytecodeChunk` — a byte array plus metadata (global names, function table).
+ Unless we pass `-c` (compile only), the *virtual machine* in `vm.cpp` executes those bytes.
+ Output goes to stdout; errors print with a phase name and a caret under the source line.

The AST exists only while compiling. At runtime the VM never sees the tree — only the bytecode.

#flow-diagram()

#fig-compile-runtime()

#tbl(
  ("Stage", "Input", "Output", "Source files"),
  (
    [Lexer], [characters], [`vector<Token>`], [`lexer.cpp`, `token.cpp`],
    [Parser], [tokens], [`Program` (AST)], [`parser.cpp`, `ast.hpp`],
    [Compiler], [AST], [`BytecodeChunk`], [`compiler.cpp`, `bytecode.cpp`],
    [VM], [bytecode], [values / errors], [`vm.cpp`, `value.cpp`],
  ),
)

== CLI modes and debug

*File mode.* With a path argument, `main` reads the file, compiles once, and runs it.

*REPL.* With no arguments, each line is compiled and executed in a `VmSession` that keeps globals
between lines (`let x = 1` on one line is visible on the next). Commands: `:help`, `:disasm path`,
`:quit`.

*Flags.*

#tbl(
  ("Flag", "Effect"),
  (
    [`-d`], [Print tokens, AST, and bytecode — then run],
    [`-c`], [Compile only; no VM],
    [`-q`], [Quiet run (no debug banners)],
  ),
)

*Using `-d` to localize bugs.*

+ *Tokens* — confirms the lexer sees keywords and operators.
+ *AST* — confirms parse structure (`ast_print.cpp`).
+ *Bytecode* — confirms jump targets and literals (`disassemble()`).

Wrong tokens → lexer. Wrong AST → parser. Wrong run with good AST → compare bytecode to section 5
and `vm.cpp`.

= Scope and design

== Goals

+ One repository and one binary: lex, parse, compile, execute.
+ Language subset: integers, booleans, variables, `fn`, `if`/`while`, `print`, `input`, recursion.
+ Diagnostics tagged by phase (`Lexer`, `Parser`, `Compiler`, `Vm`, `Repl`) with source location.
+ Eleven `.cvm` examples and `make verify` (CI runs the same checks).

== Scope

#tbl(
  ("In v1", "Not in v1"),
  (
    [Hand-written lexer, parser, compiler, stack VM], [Strings, floats, arrays],
    [REPL with persistent globals], [Modules / imports],
    [`fn`, `CALL`, `RETURN`, jump patching], [`for`, `break`, `continue`],
    [Entry `JUMP` at offset 0 → main], [Separate compiler / VM binaries],
    [`-d`, `-c`, disassembler], [Yacc/Lex, LLVM],
    [11 tests + `verify.sh` + CI], [Large standard library],
  ),
)

== Design choices

#tbl(
  ("Area", "Decision", "Reason"),
  (
    [Types], [`int64` and `bool`], [Simple VM checks],
    [AST], [`unique_ptr` tree], [Clear ownership, one compile pass],
    [Bytecode], [`vector<uint8_t>` + name pool], [Compact, easy to disassemble],
    [Functions], [`FunctionMeta` + `CALL` by index], [Address and arity at compile time],
    [Layout], [`JUMP` at byte 0 → main], [`fn` bodies emitted before main code],
    [REPL], [Global map in `VmSession`], [Variables persist across lines],
    [Safety], [Stack 65536; 1M steps per run], [Overflow and infinite-loop guards],
  ),
)

= Build phases

Each phase ended with something runnable from the terminal.

#fig-phase-timeline()

#tbl(
  ("Phase", "Focus", "Check"),
  (
    [1], [Lexer + diagnostics], [`let x = 42;` → six tokens],
    [2], [Parser + AST], [`cvmpp -d hello.cvm` → syntax tree],
    [3], [Compiler], [`cvmpp -c` → bytecode listing],
    [4], [Virtual machine], [`print 1 + 2;` → `3`],
    [5], [Functions, REPL, release], [`make verify` → 11 passed],
  ),
)

#phase-block(
  "1",
  [Lexer and diagnostics],
  [
    The lexer turns text into tokens (type, lexeme, source location). `SourceLoc`, `Diagnostic`, and
    `DiagnosticBag` tag errors with `Phase::Lexer`. `Lexer::tokenize()` handles keywords, `//`
    comments, multi-char operators, and integer overflow. `ui.cpp` prints errors with a caret.
  ],
  [
    `source_loc`, `diagnostic`, `lexer`, `token`, `ui`. For `let x = 42;`: `Let`, `Identifier`,
    `Assign`, `Integer`, `Semicolon`, `Eof`.
  ],
  [
    Token dump for a small input file.
  ],
)

#phase-block(
  "2",
  [Parser and AST],
  [
    The parser builds a `Program` tree. `ast_print` wired to `-d` separates parse bugs from
    compiler bugs.
  ],
  [
    `Expr` / `Stmt` in `ast.hpp`. `Program` has `functions[]` and `statements[]`. Recursive-descent
    precedence. `synchronize()` recovers after errors.
  ],
  [
    `./build/cvmpp -d examples/hello.cvm` — tokens then indented AST.
  ],
)

#phase-block(
  "3",
  [Bytecode compiler],
  [
    The compiler walks the AST into `BytecodeChunk::code`. Control flow becomes jump instructions
    with patched offsets. This phase used the disassembler; the VM came in phase 4.
  ],
  [
    `opcode.hpp`, `BytecodeWriter` (`emit`, `patch_u32`, `intern_name`). `compile_if` / `compile_while`
    emit placeholders then patch. `fn` / `CALL` added in phase 5.
  ],
  [
    `cvmpp -c examples/arithmetic.cvm` — bytecode only. `-d` also runs the VM.
  ],
)

#phase-block(
  "4",
  [Virtual machine],
  [
    The VM dispatch loop runs until `Halt` or error. First end-to-end target: `print 1 + 2;` → `3`.
  ],
  [
    `VmValue` (`int` / `bool`). Binary ops pop *right* then *left*. Uninitialized globals are errors.
    `compile_frontend()` does not execute; `main` calls `execute()`. Guards: ÷0, stack, types, step limit.
  ],
  [
    Disasm shows `PushInt 1`, `PushInt 2`, `Add`, `Print`. Run prints `3`, exit `0`.
  ],
)

#phase-block(
  "5",
  [Functions, REPL, and release],
  [
    User functions, recursion, `!=`/`<=`/`>=`, REPL, examples, CI. Fixed bytecode layout bug when
    `fn` bodies sit at low addresses.
  ],
  [
    `CALL` / `RETURN`, locals, `Ne`/`Le`/`Ge`, `VmSession`, flags `-d`/`-q`/`-c`, eleven `.cvm`
    files, `scripts/verify.sh`, GitHub Actions.
  ],
  [
    `make verify` — 11 passed. `functions.cvm` prints `120`.
  ],
)

== Bytecode layout and entry jump

The VM starts at offset 0. Top-level `fn` bodies are emitted *before* main (lower addresses). Without
a skip, byte 0 would enter a function with no call frame → `LOAD_LOCAL outside function`.

`compile_program()` (`compiler.cpp`):

+ Emit `JUMP` + placeholder at the start.
+ Compile all functions.
+ Patch `JUMP` to main entry.
+ Compile main statements.
+ Emit `Halt`.

#bytecode-layout()

= Language reference

== Syntax

#tbl(
  ("Feature", "Form", "Notes"),
  (
    [Literals], [`42`, `true`, `false`], [64-bit integers and booleans],
    [Variable], [`let x = expr;` then `x = expr;`], [Globals in files; REPL persists],
    [Function], [`fn name(a, b) { ... }`], [Top-level; `return expr;`],
    [Call], [`name(arg1, arg2);`], [Arity checked at compile time],
    [If / else], [`if (cond) { ... } else { ... }`], [`cond` must be `bool`],
    [While], [`while (cond) { ... }`], [Condition re-checked each iteration],
    [Print], [`print expr;`], [Value + newline],
    [Input], [`let x = input;`], [One stdin line as integer],
  ),
)

== Operator precedence

Tightest operators bind first:

#tbl(
  ("Level (tight → loose)", "Operators", "Example"),
  (
    [Unary], [`-expr`], [`-5`],
    [Multiply / divide], [`*` `/`], [`8 / 2` → `4`],
    [Add / subtract], [`+` `-`], [`1 + 2 * 3` → `7`],
    [Comparison], [`<` `>` `<=` `>=`], [`x < 10`],
    [Equality], [`==` `!=`], [`a == b`],
  ),
)

== Control flow and calls

*If / else:* condition → `JumpIfFalse` (placeholder) → `then` → optional `Jump` over `else` → patch targets.

#fig-jump-patch()

*While:* loop start → condition → `JumpIfFalse` to exit → body → `Jump` back → patch exit.

#fig-while-flow()

*Call:* arguments on stack → `Call` (index, arity) → callee locals → `Return` to caller.

#fig-call-protocol()

== Abstract syntax tree

`Program` = `functions[]` + `statements[]` (main, after entry jump). `cvmpp -d` prints:

#ast-tree(
  "Program\n|-- Let (x)\n|   +-- IntLiteral (42)\n|-- Let (flag)\n|   +-- BoolLiteral (true)\n|-- Print\n|   +-- Variable (x)\n+-- If\n    |-- Binary (<)\n    |   |-- Variable (x)\n    |   +-- IntLiteral (100)\n    +-- Block\n        +-- Print\n            +-- Binary (+)\n                |-- Variable (x)\n                +-- IntLiteral (8)"
)

*Expression nodes:* literals, `Variable`, `Unary`, `Binary`, `Call`, `Input`.

*Statement nodes:* `Let`, `Assign`, `Print`, `Return`, `If`, `While`, `Block`, `ExprStmt` (emits `Pop`).

== Opcodes

One opcode byte + fixed operands (`opcode.hpp`). Binary ops pop right, then left.

#tbl(
  ("Opcode", "Hex", "Operands", "Meaning"),
  (
    [`PushInt`], [0x01], [i64], [Integer literal],
    [`PushBool`], [0x02], [u8], [Boolean],
    [`Pop`], [0x03], [—], [Discard stack top],
    [`LoadVar` / `StoreVar`], [0x10 / 0x11], [u16], [Global],
    [`LoadLocal` / `StoreLocal`], [0x12 / 0x13], [u8], [Local in frame],
    [`Add`–`Div`], [0x20–0x23], [—], [Integer math; `/` checks ÷0],
    [`Eq` `Ne` `Lt` `Gt` `Neg` `Le` `Ge`], [0x24–0x2A], [—], [Compare / negate],
    [`Input` / `Print`], [0x30 / 0x31], [—], [Stdin line / print + newline],
    [`Jump` / `JumpIfFalse`], [0x40 / 0x41], [u32], [Branch],
    [`Call` / `Return`], [0x42 / 0x43], [u16, u8 / —], [Function call / return],
    [`Halt`], [0xFF], [—], [Stop],
  ),
)

= Example programs

Scripts live in `examples/`. `make verify` checks every exit code (ten scripts → `0`,
`div_by_zero.cvm` → `2`).

#tbl(
  ("Script", "Exit", "Topic"),
  (
    [`hello.cvm`], [0], [Variables, `print`, `if`],
    [`arithmetic.cvm`], [0], [Operators],
    [`if_else.cvm`], [0], [`else`],
    [`factorial.cvm`], [0], [`while`],
    [`functions.cvm`], [0], [Recursion, `CALL`],
    [`input_demo.cvm`], [0], [`input`],
    [`div_by_zero.cvm`], [2], [Runtime error],
    [`booleans.cvm`], [0], [Booleans],
    [`assignment.cvm`], [0], [Reassignment],
    [`comparisons.cvm`], [0], [`!=`, `<=`, `>=`],
    [`multiline_demo.cvm`], [0], [Multi-line blocks],
  ),
)

== Worked examples

#worked-example(
  [Variables, print, and if],
  [hello.cvm],
  [Bind globals, print, and branch on a condition.],
  ```cvm
let x = 42;
let flag = true;
print x;
if (x < 100) {
  print x + 8;
}
  ```,
  [
    + `let x = 42;` — global `x` = `42` (`StoreVar` after pushing the literal).
    + `let flag = true;` — boolean global (unused later; checks bool parsing).
    + `print x;` → first line `42`.
    + `if (x < 100)` — `42 < 100` is `true`; then-branch runs.
    + `print x + 8;` → `42 + 8` = `50` on the second line.
  ],
  [./build/cvmpp -q examples/hello.cvm],
  [42\
50],
)

#worked-example(
  [Arithmetic and comparisons],
  [arithmetic.cvm],
  [`a = 20`, `b = 6`; each `print` applies one opcode to the stack.],
  ```cvm
let a = 20;
let b = 6;
print a + b;
print a - b;
print a * b;
print a / b;
print a == 20;
print a < b;
  ```,
  [
    + `a + b` → `26` (`Add` pops `6`, then `20`).
    + `a - b` → `14` (right operand popped first).
    + `a * b` → `120`; `a / b` → `3` (integer division).
    + `a == 20` → `true`; `a < b` → `false`.
  ],
  [./build/cvmpp -q examples/arithmetic.cvm],
  [26\
14\
120\
3\
true\
false],
)

#worked-example(
  [If / else branches],
  [if_else.cvm],
  [`n = 17` so `n < 10` is false — only the else branch runs.],
  ```cvm
let n = 17;
if (n < 10) {
  print 1;
} else {
  print 2;
}
  ```,
  [
    + `JumpIfFalse` skips the then-block when the condition is `false`.
    + `print 2` in the else branch → output `2`.
  ],
  [./build/cvmpp -q examples/if_else.cvm],
  [2],
)

#worked-example(
  [While loop (factorial)],
  [factorial.cvm],
  [Iterative `5!` with a `while` loop.],
  ```cvm
let n = 5;
let result = 1;
while (n > 0) {
  result = result * n;
  n = n - 1;
}
print result;
  ```,
  [
    + Loop: while `n > 0`, multiply `result` by `n`, then decrement `n`.
    + When `n` reaches `0`, exit and print `result`.
  ],
  [./build/cvmpp -q examples/factorial.cvm],
  [120],
)

#tbl(
  ("Iteration", "`n` after", "`result`"),
  (
    [1], [4], [5],
    [2], [3], [20],
    [3], [2], [60],
    [4], [1], [120],
    [5], [0], [120],
  ),
)

#worked-example(
  [Recursive functions],
  [functions.cvm],
  [Recursive `factorial`; function bytecode before main; entry `JUMP` at offset 0.],
  ```cvm
fn factorial(n) {
  if (n <= 1) {
    return 1;
  }
  return n * factorial(n - 1);
}

let x = factorial(5);
print x;
  ```,
  [
    + `factorial(5)` → `Call` pushes a frame; `n` is local slot 0.
    + Base case `n <= 1` → `return 1`.
    + Else `n * factorial(n - 1)` — inner call completes before multiply.
    + Chain `5→4→…→1`, then multiply back → `120`.
  ],
  [./build/cvmpp -q examples/functions.cvm],
  [120],
)

#ast-tree(
  "Program\n|-- Function (factorial)\n|   |-- Param (n)\n|   +-- Block ...\n|-- Let (x)\n|   +-- Call (factorial)\n|       +-- IntLiteral (5)\n+-- Print\n    +-- Variable (x)"
)

#worked-example(
  [Input from stdin],
  [input_demo.cvm],
  [Read stdin, print double.],
  ```cvm
let n = input;
print n * 2;
  ```,
  [
    + `let n = input;` reads one line as an integer.
    + `print n * 2` — with `echo 21 | cvmpp ...`, output is `42`.
  ],
  [echo 21 \| ./build/cvmpp -q examples/input_demo.cvm],
  [42],
)

#worked-example(
  [Divide by zero],
  [div_by_zero.cvm],
  [Compiles; VM errors at `Div` with divisor zero.],
  ```cvm
let a = 10;
let b = 0;
print a / b;
  ```,
  [
    + `Div` sees `10` and `0` on the stack.
    + VM prints `Phase::Vm` error; exit `2` (compile errors use `1`).
    + `make verify` requires exit `2` for this file.
  ],
  [./build/cvmpp -q examples/div_by_zero.cvm],
  [VM error at 0:0: divide by zero\
hint: check the divisor before division; 0 is not allowed],
  exit: [2],
)

= Repository and commands

== Layout

Headers: `include/cvm++/`. Implementation: `src/`. One binary links everything.

#tbl(
  ("Part", "Role", "Files"),
  (
    [Diagnostics], [Errors with carets], [`source_loc`, `diagnostic`, `ui`],
    [Lexer], [Tokenize], [`lexer`, `token`],
    [Parser], [Build AST], [`parser`, `ast`, `ast_print`],
    [Compiler], [AST → bytecode], [`compiler`, `bytecode`, `opcode`],
    [VM], [Execute], [`vm`, `value`, `compile`],
    [Driver], [CLI, REPL], [`main`, `runtime_options`],
    [Tests], [Regression], [`examples/`, `scripts/verify.sh`],
    [CI], [Automated verify], [`.github/workflows/ci.yml`],
  ),
)

== Build and verify

#tbl(
  ("Command", "Effect"),
  (
    [`make`], [Build `build/cvmpp` (`clang++`, `-Iinclude`)],
    [`make cmake`], [CMake Release build],
    [`make verify`], [Run `scripts/verify.sh` on all examples],
    [`make clean`], [Remove `build/cvmpp`],
  ),
)

`scripts/verify.sh` checks stdout and exit codes. CI runs `make verify` on push.

== Exit codes and VM guards

#tbl(
  ("Code", "When"),
  (
    [0], [Success],
    [1], [Lexer, parser, or compiler error],
    [2], [VM runtime guard (÷0, bad type, unread variable, …)],
  ),
)

VM faults (exit `2`): division by zero; stack overflow/underflow (max 65536); unread global; type
mismatch; invalid `RETURN` / `LOAD_LOCAL`; more than 1M instructions per run.

== Daily commands

#tbl(
  ("Command", "Use"),
  (
    [`make`], [Build],
    [`make verify`], [Run all examples],
    [`cvmpp -d file.cvm`], [Debug dump, then run],
    [`cvmpp -c file.cvm`], [Compile only],
    [`cvmpp -q file.cvm`], [Quiet run],
    [`cvmpp`], [REPL],
  ),
)
