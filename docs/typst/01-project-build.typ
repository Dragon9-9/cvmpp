#import "cvm-theme.typ": *

#cover(
  "Project and Build Guide",
  "Setup, structure, and language overview",
  "5-page presentation · github.com/Dragon9-9/cvmpp",
)
#slide-setup()

= What is CVM++?

A *scripting language* + full *toolchain* in C++17 — one binary `cvmpp`, no LLVM/Python.

#slidebox[
  #align(center)[
    #text(weight: "bold")[.cvm] #sym.arrow #text(weight: "bold")[Lexer] #sym.arrow
    #text(weight: "bold")[Parser] #sym.arrow #text(weight: "bold")[Compiler] #sym.arrow
    #text(weight: "bold")[Stack VM] #sym.arrow #text(weight: "bold")[Output]
  ]
  #v(0.2cm)
  #align(center)[#text(size: 9pt, fill: ink-muted)[
    tokens #sym.dot AST #sym.dot bytecode #sym.dot execution
  ]]
]

#tbl(
  ("Piece", "Role"),
  (
    [Language], [.cvm — variables, fn, if/while, print, input],
    [Lexer / Parser / Compiler], [Source to bytecode bytes],
    [VM], [Stack execution + safety checks],
    [CLI / REPL], [Files, debug, interactive mode],
  ),
)

= Build and run

#grid(
  columns: (1fr, 1fr),
  gutter: 12pt,
  [
    == Clone and build
    #codeblock[
      ```bash
      git clone https://github.com/Dragon9-9/cvmpp.git
      cd cvmpp && make
      make verify        # 11/11 tests
      ```
    ]
    == Run
    #codeblock[
      ```bash
      ./build/cvmpp examples/hello.cvm
      ./build/cvmpp -d examples/functions.cvm
      ./build/cvmpp          # REPL
      ```
    ]
  ],
  [
    == Flags
    #tbl(
      ("Flag", "Effect"),
      (
        [`-d`], [Tokens + AST + bytecode],
        [`-q`], [Quiet output],
        [`-c`], [Compile only],
      ),
    )
    *Exit:* `0` ok · `1` compile · `2` runtime
  ],
)

= Repository layout

#grid(
  columns: (1.1fr, 0.9fr),
  gutter: 10pt,
  codeblock[
    ```
    include/cvm++/   headers
    src/             lexer, parser,
                     compiler, vm, main
    examples/*.cvm   test scripts
    scripts/         verify.sh
    Makefile         make / make verify
    ```
  ],
  [
    == Built in 5 phases
    + *Phase 1* — lexer, tokens, diagnostics, UI
    + *Phase 2* — parser, AST
    + *Phase 3* — bytecode, compiler
    + *Phase 4* — stack VM, values
    + *Phase 5* — REPL, functions, CI

    #v(0.15cm)
    #text(size: 9pt)[Functions at start of bytecode; offset-0 `JUMP` skips to *main*.]
  ],
)

#pagebreak()

= CVM++ language

#codeblock[
  ```cvm
  fn factorial(n) {
    if (n <= 1) { return 1; }
    return n * factorial(n - 1);
  }
  print factorial(5);   // 120
  ```
]

#grid(
  columns: (1fr, 1fr),
  gutter: 10pt,
  tbl(
    ("Feature", "Syntax"),
    (
      [Types], [int, bool],
      [Vars], [`let x = …;` `x = …;`],
      [Functions], [`fn f(a) { }` `f(1,2)`],
      [Control], [`if`/`else` `while`],
      [Ops], [`+ - * / == != < > <= >=`],
      [I/O], [`print` `input`],
    ),
  ),
  [
    == REPL commands
    `help` `quit` `debug` `run file.cvm`
    `disasm` / `disasm file.cvm`

    #v(0.2cm)
    == Not supported
    strings, floats, arrays, `for`, modules
  ],
)

= Examples and verification

#tbl(
  ("Script", "Shows"),
  (
    [`hello.cvm`], [let, if, print],
    [`functions.cvm`], [recursive fn → 120],
    [`factorial.cvm`], [while loop],
    [`comparisons.cvm`], [`!=` `<=` `>=`],
    [`div_by_zero.cvm`], [runtime error exit 2],
  ),
)

#slidebox[
  #text(weight: "bold")[make verify — 11/11 PASSED]
  #text(size: 9pt)[
    hello · arithmetic · booleans · if\_else · factorial · multiline\_demo ·
    assignment · functions · comparisons · input\_demo · div\_by\_zero
  ]
]

#pagebreak()

= Project status

#tbl(
  ("Component", "Status", "Verified"),
  (
    [Lexer], [Tokens, overflow, comments], [✓],
    [Parser], [AST, if/while/fn], [✓],
    [Compiler], [Bytecode, jumps, calls], [✓],
    [Stack VM], [Arithmetic, frames, failsafes], [✓],
    [CLI / REPL], [Files, `-d`, multiline], [✓],
    [CI], [GitHub Actions on push], [✓],
  ),
)

#slidebox[
  *Personal project* — full interpreter pipeline in C++17.\
  Inspired by *Crafting Interpreters* (Robert Nystrom).\
  No external interpreter libraries.
]

= Implementation milestones

#tbl(
  ("Phase", "Deliverable", "Test"),
  (
    [1 Foundation], [Lexer + diagnostics + UI], [Token table prints],
    [2 Parser], [AST + recursive descent], [AST tree in `-d`],
    [3 Compiler], [Bytecode + disasm], [`:disasm` works],
    [4 VM], [Stack execution], [`print 1+2` → 3],
    [5 Product], [REPL, fn, CI, examples], [`make verify` 11/11],
  ),
)

= Key terms

#tbl(
  ("Term", "Meaning"),
  (
    [Token], [Lexer output unit (keyword, number, symbol)],
    [AST], [Syntax tree after parser],
    [Bytecode], [`vector<uint8_t>` VM instructions],
    [Opcode], [One-byte command: Add, Call, Jump…],
    [Stack], [Expression values during execution],
    [Call frame], [Return address + locals in a function],
    [VmSession], [REPL globals that persist between lines],
    [Diagnostic], [Error with phase, line, hint],
  ),
)

#v(0.3cm)
#align(center)[#text(size: 8pt, fill: ink-muted)[MIT License · github.com/Dragon9-9/cvmpp]]
