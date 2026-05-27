#import "cvm-theme.typ": *

#cover(
  "Architecture and Workflow Guide",
  "How cvmpp runs your program internally",
  "5-page presentation · github.com/Dragon9-9/cvmpp",
)
#slide-setup()

= Runtime overview

```bash
./build/cvmpp examples/hello.cvm
```

#slidebox[
  #align(center)[
    #text(weight: "bold")[main.cpp]
    #sym.arrow #text(weight: "bold")[compile\_frontend()]
    #sym.arrow #text(weight: "bold")[VirtualMachine::run()]
  ]
  #v(0.25cm)
  #grid(
    columns: (1fr, 1fr, 1fr, 1fr),
    gutter: 6pt,
    box(width: 100%, inset: 6pt, fill: accent.lighten(92%), stroke: 0.4pt + accent)[
      #align(center)[#text(weight: "bold", size: 8pt)[Lexer]]
      #text(size: 7.5pt, fill: ink-muted)[string→tokens]
    ],
    box(width: 100%, inset: 6pt, fill: accent.lighten(92%), stroke: 0.4pt + accent)[
      #align(center)[#text(weight: "bold", size: 8pt)[Parser]]
      #text(size: 7.5pt, fill: ink-muted)[tokens→AST]
    ],
    box(width: 100%, inset: 6pt, fill: accent.lighten(92%), stroke: 0.4pt + accent)[
      #align(center)[#text(weight: "bold", size: 8pt)[Compiler]]
      #text(size: 7.5pt, fill: ink-muted)[AST→bytes]
    ],
    box(width: 100%, inset: 6pt, fill: accent.lighten(92%), stroke: 0.4pt + accent)[
      #align(center)[#text(weight: "bold", size: 8pt)[VM]]
      #text(size: 7.5pt, fill: ink-muted)[bytes→output]
    ],
  )
]

#tbl(
  ("Stage", "Input", "Output"),
  (
    [Lexer], [Source text], [vector of Token],
    [Parser], [Tokens], [Program AST],
    [Compiler], [AST], [BytecodeChunk],
    [VM], [Bytecode], [Printed lines + errors],
  ),
)

= Front end (`compile.cpp`)

#codeblock[
  ```cpp
  compile_frontend(source):
    1. Lexer::tokenize()
    2. Parser::parse()
    3. Compiler::compile()
  // main.cpp then: execute(chunk)
  ```
]

#grid(
  columns: (1fr, 1fr),
  gutter: 10pt,
  [
    == Parser output
    ```
    Program
     ├─ functions[]  (fn defs)
     └─ statements[] (main script)
  ```
    Precedence: eq → compare → add/sub → mul/div → unary minus
  ],
  [
    == Bytecode layout
    #tbl(
      ("Offset", "Content"),
      (
        [0], [`JUMP` → main],
        […], [function bodies],
        [main], [top-level code],
        [end], [`HALT`],
      ),
    )
    Without entry `JUMP` → crash on `LOAD_LOCAL`.
  ],
)

#pagebreak()

= Compiler control flow

#tbl(
  ("Construct", "Bytecode idea"),
  (
    [`if` / `else`], [`JUMP_IF_FALSE`, patch else; `JUMP` end],
    [`while`], [loop label, cond, body, jump back],
    [`let` / `=`], [eval expr; `STORE_VAR` or `STORE_LOCAL`],
    [`fn` / call], [metadata table; `CALL` index + argc],
    [`return`], [`RETURN` restores frame + value],
  ),
)

== Name pool
Globals: intern name → *u16* index (`LOAD_VAR` / `STORE_VAR`).\
Locals: parameter slots → *u8* (`LOAD_LOCAL` / `STORE_LOCAL`).

= Virtual machine

#grid(
  columns: (1fr, 1fr),
  gutter: 10pt,
  tbl(
    ("State", "Role"),
    (
      [`ip_`], [Index into bytecode],
      [`stack_`], [Operand stack],
      [`globals_`], [File-mode variables],
      [`frames_`], [Call stack],
      [`session_`], [REPL name→value map],
    ),
  ),
  [
    == Failsafes
    + Stack max 65,536
    + Step limit 1,000,000
    + Divide by zero → exit 2
    + Uninitialized var load
    + Stack under/overflow
    + Bad jump / truncated bytecode

    #v(0.15cm)
    *Types:* only `int64_t` and `bool`
  ],
)

= Opcode reference

#text(size: 9pt)[Stack: binary ops pop *b* (top) then *a*.]

#tbl(
  ("Op", "Hex", "Effect"),
  (
    [`PUSH_INT`], [01], [push i64],
    [`LOAD/STORE_VAR`], [10/11], [global u16 idx],
    [`LOAD/STORE_LOCAL`], [12/13], [local u8 slot],
    [`ADD`…`GE`], [20–2A], [math / compare],
    [`INPUT`/`PRINT`], [30/31], [stdin / output],
    [`JUMP`/`JIF_FALSE`], [40/41], [u32 offset],
    [`CALL`/`RETURN`], [42/43], [fn + argc / return],
    [`HALT`], [FF], [stop],
  ),
)

= CALL / RETURN (4 steps)

+ Caller pushes arguments on stack.
+ `CALL`: pop args → `frame.locals`; save `return_ip`; `ip` = function address.
+ Function runs; nested `CALL`/`RETURN` as needed.
+ `RETURN`: pop value; restore `ip`; push result to caller.

#pagebreak()

= Lexer and parser (detail)

#grid(
  columns: (1fr, 1fr),
  gutter: 10pt,
  [
    == Lexer emits
    Keywords: `let fn return if else while print input true false`\
    Operators: `== != <= >= < > + - * /`\
    Literals, identifiers, `//` comments, `Eof`

    #v(0.2cm)
    *Errors:* bad char, int overflow → phase LEXER
  ],
  [
    == Parser builds
    `Program { functions[], statements[] }`\
    Function body must be `{ block }`\
    Call: `name(expr, …)` — arity checked at compile

    #v(0.2cm)
    *Errors:* bad assign target, unbalanced `{}` → phase PARSER
  ],
)

= Execution traces

== Trace A: `print 1 + 2;`

#tbl(
  ("Step", "Instruction", "Stack"),
  (
    [1], [`PUSH_INT 1`], [[1]],
    [2], [`PUSH_INT 2`], [[1,2]],
    [3], [`ADD`], [[3]],
    [4], [`PRINT`], [output `3`],
  ),
)

== Trace B: `print factorial(5);`

+ Main: push `5`, `CALL factorial`
+ Recursive frames until `n <= 1` returns `1`
+ Unwind multiplies: 1→2→6→24→*120*
+ `PRINT` shows 120

= AST nodes (summary)

#grid(
  columns: (1fr, 1fr),
  gutter: 8pt,
  tbl(
    ("Expressions", "Examples"),
    (
      [Literals], [int, bool],
      [Binary / Unary], [`+` `-` `==` `<` …],
      [Call / Input], [`f(a)` `input`],
    ),
  ),
  tbl(
    ("Statements", "Examples"),
    (
      [let / assign], [declare, update],
      [if / while], [control flow],
      [print / return], [I/O, fn exit],
      [Block], [`{ … }`],
    ),
  ),
)

= REPL vs file · Debug

#grid(
  columns: (1fr, 1fr),
  gutter: 10pt,
  tbl(
    ("", "File", "REPL"),
    (
      [Globals], [Reset each run], [Persist via VmSession],
      [Storage], [Indexed array], [Map by name],
    ),
  ),
  [
    == Debug pipeline
    #codeblock[
      ```bash
      ./build/cvmpp -d examples/hello.cvm
      ```
    ]
    1. Token table\
    2. AST tree\
    3. Bytecode disasm\
    4. Runtime output
  ],
)

#slidebox[
  #text(weight: "bold")[Error phases:]
  LEXER · PARSER · COMPILER · VM · REPL\
  #text(size: 9pt)[Colored message + source caret — process does not crash on user errors.]
]

#v(0.2cm)
#align(center)[#text(size: 8pt, fill: ink-muted)[MIT License · github.com/Dragon9-9/cvmpp]]
