#import "cvm-theme.typ": *
#import "cvm-figures.typ": (
  fig-pipeline, fig-compile-runtime, fig-driver-flow, fig-pipeline-stops,
  fig-program-shape, fig-chunk-parts, fig-bytecode-layout, fig-while-flow,
  fig-call-protocol, fig-opcode-map,
)

#doc-setup-plain(header-label: [Architecture and Workflow])
#doc-title([Architecture and Workflow], tight: true)

This document is the implementation-level workflow reference for CVM++. It describes how source code
moves through Driver, Lexer, Parser, Compiler, and VM, and maps each stage to its primary source
files.

= Workflow Overview

#fig-pipeline()

#tbl-tight(
  ("Stage", "Input", "Output", "Primary file(s)"),
  (
    [Driver], [CLI flags and source path], [source text or diagnostics], [`main.cpp`],
    [Lexer], [source text], [token stream], [`lexer.cpp`],
    [Parser], [token stream], [`Program` AST], [`parser.cpp`, `ast.hpp`],
    [Compiler], [AST], [`BytecodeChunk`], [`compiler.cpp`, `bytecode.cpp`],
    [VM], [`BytecodeChunk`], [stdout and status], [`vm.cpp`],
  ),
)

`compile_frontend()` in `compile.cpp` orchestrates Lexer → Parser → Compiler. If compile succeeds,
`main.cpp` calls `execute()` in `vm.cpp` unless compile-only mode is enabled.

#fig-compile-runtime()

#tbl-tight(
  ("CLI mode", "Behavior"),
  (
    [`cvmpp file.cvm`], [Compile then run],
    [`-c`], [Compile only; VM does not run],
    [`-d`], [Print tokens, AST, and bytecode before running],
    [`-q`], [Hide informational banners],
  ),
)

= Driver Stage

#fig-driver-flow()

Responsibilities:

+ Parse flags and select file mode or REPL mode.
+ Read source into memory.
+ Run `compile_frontend()`.
+ Handle compile diagnostics and exit with code `1` on failure.
+ Run debug dumps (`-d`) and VM execution (unless `-c`).

When any compile stage fails, later stages are skipped:

#fig-pipeline-stops()

= Core Artifacts

== Program AST

Parser output root is `Program`:

#fig-program-shape()

#tbl-tight(
  ("Field", "Meaning", "Consumed by"),
  (
    [`functions[]`], [Top-level function declarations], [Compiler function metadata and code emission],
    [`statements[]`], [Top-level executable statements], [Compiler main-script emission],
  ),
)

== Bytecode chunk

Compiler output is `BytecodeChunk`:

#fig-chunk-parts()
#fig-bytecode-layout()

#tbl-tight(
  ("Chunk component", "Role"),
  (
    [`code[]`], [Opcode bytes plus encoded operands],
    [`names[]`], [Interned global-name pool referenced by slot],
    [`functions[]`], [Function metadata (entry offset, arity, identity)],
  ),
)

= Stage Reference

== Lexer

`Lexer::tokenize()` scans source left-to-right and emits token records with:

+ token type,
+ lexeme,
+ source location (line, column).

#tbl-tight(
  ("Token class", "Examples", "Purpose"),
  (
    [Keywords], [`let`, `fn`, `if`, `while`, `return`, `print`], [Language control and declarations],
    [Identifiers], [variable/function names], [Symbol references],
    [Literals], [integers and booleans], [Immediate values],
    [Operators], [`+`, `-`, `*`, `<`, `<=`, `=`], [Expression and assignment semantics],
    [Delimiters], [`(` `)` `{` `}` `;` `,`], [Grammar structure],
    [Terminator], [`Eof`], [End-of-input marker],
  ),
)

== Parser

Parser builds AST nodes using recursive descent with precedence-aware expression parsing. Parse
diagnostics stop the pipeline before compilation.

Generic AST shape (not tied to one file):

#cvm-sample-tight("Program\n|-- functions[]\n|   +-- FunctionDecl(name, params, body)\n+-- statements[]\n    +-- Let / Print / If / While / Return nodes")

== Compiler

Compiler lowers AST nodes into bytecode and metadata. It does not execute program logic.

#tbl-tight(
  ("Source form", "Lowering pattern (typical)"),
  (
    [Variable declaration], [Evaluate expression → `STORE_VAR`],
    [Print statement], [Evaluate expression → `PRINT`],
    [Conditional], [Condition eval → `JUMP_IF_FALSE` around block],
    [Loop], [Header label → condition → exit jump → body → back jump],
    [Function call], [Evaluate args → `CALL` with function index and arity],
    [Return], [Evaluate expression → `RETURN`],
  ),
)

#fig-while-flow()

== Virtual machine

VM executes a fetch-decode-execute loop over `code[]` until `HALT` or runtime fault.

#tbl-tight(
  ("VM state", "Runtime role"),
  (
    [`ip_`], [Instruction pointer into `code[]`],
    [`stack_`], [Operand stack for expression evaluation],
    [`globals_`], [Global storage indexed by name slots],
    [`frames_`], [Call frames containing locals and return addresses],
  ),
)

#fig-call-protocol()

Generic opcode flow (stack view):

#tbl-tight(
  ("IP", "Opcode", "Stack before", "Stack after", "Notes"),
  (
    [n], [`LOAD_VAR a`], [[ ]], [[value(a)]], [Push variable value],
    [n+1], [`LOAD_VAR b`], [[value(a)]], [[value(a), value(b)]], [Push second operand],
    [n+2], [`ADD`], [[value(a), value(b)]], [[value(a+b)]], [Pop two, push result],
    [n+3], [`PRINT`], [[value(a+b)]], [[ ]], [Pop and print],
    [n+4], [`HALT`], [[ ]], [[ ]], [Stop execution],
  ),
)

= Function and Recursion Workflow

Function execution follows this runtime pattern:

1. Caller evaluates arguments and pushes them.
2. VM executes `CALL` and creates a new frame.
3. Callee reads parameters from frame-local slots.
4. Callee executes until `RETURN`.
5. VM restores caller frame and resumes at return address.

#tbl-tight(
  ("Frame field", "Meaning"),
  (
    [Local slot], [Parameter/local value storage for the active frame],
    [Return address], [Instruction offset to continue after `RETURN`],
    [Frame depth], [Current call nesting level],
  ),
)

Generic call-stack ladder:

#tbl-tight(
  ("Moment", "Top frame", "Caller frame", "Event"),
  (
    [Before `CALL`], [none], [current], [Caller has evaluated arguments],
    [After `CALL`], [callee], [caller], [New frame created, locals initialized],
    [During callee], [callee], [caller], [Callee executes bytecode],
    [At `RETURN`], [callee], [caller], [Return value prepared],
    [After `RETURN`], [caller], [previous caller], [Callee frame popped, caller resumes],
  ),
)

= Diagnostics and Exit Behavior

#tbl-tight(
  ("Symptom", "Likely stage", "Primary file to inspect"),
  (
    [Malformed tokenization], [Lexer], [`lexer.cpp`],
    [Syntax error], [Parser], [`parser.cpp`],
    [Incorrect jump layout / opcode sequence], [Compiler], [`compiler.cpp`],
    [Wrong runtime value / stack behavior], [VM], [`vm.cpp`],
  ),
)

Front-end early-stop rule:

```cpp
result.lex = lexer.tokenize();
if (!result.lex.ok()) return result;
result.parse = parser.parse();
if (!result.parse.ok()) return result;
result.compile = compiler.compile();
return result;
```

#tbl-tight(
  ("Exit code", "Meaning"),
  (
    [0], [Success (`HALT` reached)],
    [1], [Compile-time failure (Lexer, Parser, or Compiler)],
    [2], [Runtime VM failure],
  ),
)

= Opcode Reference

#fig-opcode-map()
