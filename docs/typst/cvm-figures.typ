#import "cvm-theme.typ": brand, brand-dark, brand-light, border, ink-muted, surface, tbl

#let flow-arrow = text(size: 11pt, fill: brand)[$arrow.r.long$]

#let flow-node(label, sub: none) = box(
  stroke: 0.65pt + brand,
  fill: brand-light,
  radius: 4pt,
  inset: (x: 9pt, y: 7pt),
)[
  #align(center)[
    #text(size: 9.25pt, weight: "bold", fill: brand-dark)[#label]
    #if sub != none [
      #v(0.08em)
      #text(size: 7.75pt, fill: ink-muted)[#sub]
    ]
  ]
]

#let fig-analogy() = figure(
  caption: [Same program, four representations — each stage makes the next easier for the computer.],
  supplement: [Figure],
)[
  #set text(size: 8.75pt)
  #grid(
    columns: (1fr, 1fr),
    gutter: 10pt,
    box(stroke: 0.5pt + border, fill: white, inset: 10pt, radius: 4pt)[
      #text(weight: "bold", fill: brand-dark)[You write]
      #v(0.2em)
      #text(size: 8.5pt)[`.cvm` source text\
      Human-readable CVM++]
    ],
    box(stroke: 0.5pt + border, fill: white, inset: 10pt, radius: 4pt)[
      #text(weight: "bold", fill: brand-dark)[Lexer makes]
      #v(0.2em)
      #text(size: 8.5pt)[*Tokens* — labeled words\
      `Let`, `Identifier`, `42`, `;` …]
    ],
    box(stroke: 0.5pt + border, fill: white, inset: 10pt, radius: 4pt)[
      #text(weight: "bold", fill: brand-dark)[Parser makes]
      #v(0.2em)
      #text(size: 8.5pt)[*AST* — tree structure\
      “this `if` contains a `<` compare”]
    ],
    box(stroke: 0.5pt + border, fill: white, inset: 10pt, radius: 4pt)[
      #text(weight: "bold", fill: brand-dark)[Compiler + VM]
      #v(0.2em)
      #text(size: 8.5pt)[*Bytecode* then *stack*\
      Numbers and jump addresses]
    ],
  )
]

#let fig-chunk-parts() = figure(
  caption: [`BytecodeChunk` — three parallel arrays the VM needs at runtime.],
  supplement: [Figure],
)[
  #tbl(
    ("Part", "Type", "Purpose"),
    (
      [`code[]`], [`vector<uint8_t>`], [Raw instructions: opcode bytes + operands],
      [`names[]`], [`vector<string>`], [Global names: index 0 → `"x"`, index 1 → `"flag"`],
      [`functions[]`], [`vector<FunctionMeta>`], [Each `fn`: name, start address, parameter count],
    ),
  )
]

#let fig-stack-steps() = figure(
  caption: [Operand stack while evaluating `1 + 2` (bottom = first pushed).],
  supplement: [Figure],
)[
  #set text(size: 8.5pt, font: "Menlo")
  #grid(
    columns: (1fr, 1fr, 1fr, 1fr),
    gutter: 6pt,
    align: center,
    box(stroke: 0.4pt + border, inset: 8pt)[*After*\
    `PUSH 1`\
    #v(0.2em)
    ┌───┐\
    │ 1 │\
    └───┘],
    box(stroke: 0.4pt + border, inset: 8pt)[*After*\
    `PUSH 2`\
    #v(0.2em)
    ┌───┐\
    │ 2 │\
    │ 1 │\
    └───┘],
    box(stroke: 0.4pt + border, inset: 8pt)[*After*\
    `ADD`\
    #v(0.2em)
    ┌───┐\
    │ 3 │\
    └───┘],
    box(stroke: 0.4pt + border, inset: 8pt)[*After*\
    `PRINT`\
    #v(0.2em)
    (empty)\
    stdout: 3],
  )
]

#let fig-driver-flow() = figure(
  caption: [`main.cpp` control flow from argv to compile or run.],
  supplement: [Figure],
)[
  #set text(size: 8.5pt)
  #grid(
    columns: 1,
    row-gutter: 5pt,
    box(fill: surface, inset: 8pt, radius: 3pt, width: 100%)[`main` → parse CLI flags (`-d`, `-c`, `-q`, script path)],
    box(fill: surface, inset: 8pt, radius: 3pt, width: 100%)[Path given? → `run_file` reads `.cvm` into a string · else → `run_repl` loop],
    box(fill: surface, inset: 8pt, radius: 3pt, width: 100%)[`run_source` → `compile_frontend(source)` → `print_frontend_result`],
    box(fill: surface, inset: 8pt, radius: 3pt, width: 100%)[On success: if `-d` print tokens / AST / bytecode · if `-c` stop · else `execute(chunk)` → print output],
  )
]

#let fig-pipeline() = figure(
  caption: [Pipeline: source → tokens → AST → bytecode → VM.],
)[
  #align(center)[
    #grid(
      columns: 13,
      column-gutter: 0.15em,
      align: horizon,
      flow-node([.cvm], sub: [source]),
      flow-arrow, flow-node([Lexer], sub: [tokens]),
      flow-arrow, flow-node([Parser], sub: [AST]),
      flow-arrow, flow-node([Compiler], sub: [bytes]),
      flow-arrow, flow-node([VM], sub: [values]),
      flow-arrow, flow-node([Output], sub: [stdout]),
    )
  ]
]

#let fig-compile-runtime() = figure(
  caption: [Compile time (front end) versus runtime (virtual machine).],
  kind: image,
  supplement: [Figure],
)[
  #grid(
    columns: (1fr, 1fr),
    gutter: 12pt,
    box(width: 100%, stroke: 0.6pt + brand, fill: brand-light, radius: 4pt, inset: 7pt)[
      #text(weight: "bold", fill: brand)[Compile time]
      #v(0.12em)
      #text(size: 9pt)[
        Lexer → Parser → Compiler\
        Produces `BytecodeChunk`\
        Diagnostics: Lexer, Parser, Compiler
      ]
    ],
    box(width: 100%, stroke: 0.6pt + rgb("#0f766e"), fill: rgb("#ecfdf5"), radius: 4pt, inset: 7pt)[
      #text(weight: "bold", fill: rgb("#0f766e"))[Runtime]
      #v(0.12em)
      #text(size: 9pt)[
        `VirtualMachine::run`\
        Operand stack + call frames\
        Diagnostics: Vm
      ]
    ],
  )
]

#let mem-row(title, detail, fill: brand-light) = block(
  width: 100%,
  fill: fill,
  stroke: 0.45pt + brand,
  inset: 8pt,
)[
  #text(size: 9.25pt, weight: "bold", fill: brand-dark)[#title]
  #v(0.1em)
  #text(size: 8.75pt, fill: ink-muted)[#detail]
]

#let fig-bytecode-layout() = figure(
  caption: [Logical layout of `BytecodeChunk::code` after compilation (function-first placement).],
  kind: image,
  supplement: [Figure],
)[
  #stack(spacing: 0pt)[
    #mem-row[Offset 0][`JUMP` + u32 target → main entry (patched last)]
    #mem-row[Low addresses][Function bodies: `fn` declarations emitted in order]
    #mem-row[Main entry label][Top-level statements from `Program::statements`]
    #mem-row[End of chunk][`HALT` opcode]
  ]
  #v(0.15em)
  #text(size: 8.25pt, fill: ink-muted)[
    Metadata: `names[]`, `functions[]` (name, address, arity).
  ]
]

#let fig-call-protocol() = figure(
  caption: [Function call: arguments move from operand stack to callee local slots.],
  kind: image,
  supplement: [Figure],
)[
  #grid(
    columns: (1fr, 0.35fr, 1fr),
    gutter: 8pt,
    align: horizon,
    box(stroke: 0.5pt + border, inset: 8pt, radius: 3pt)[
      #text(size: 8.75pt, weight: "bold")[Caller]
      #v(0.15em)
      #text(size: 8.5pt)[Eval args → stack]\
      #text(size: 8.5pt)[`CALL` idx, arity]
    ],
    align(center)[#flow-arrow],
    box(stroke: 0.5pt + border, inset: 8pt, radius: 3pt)[
      #text(size: 8.75pt, weight: "bold")[Callee frame]
      #v(0.15em)
      #text(size: 8.5pt)[Pop args → `locals`]\
      #text(size: 8.5pt)[`ip` = fn address]\
      #text(size: 8.5pt)[`RETURN` → caller]
    ],
  )
]

#let fig-phase-timeline() = figure(
  caption: [Implementation order: each phase unlocks the next and has a concrete verification step.],
  kind: image,
  supplement: [Figure],
)[
  #let p(n, name, check) = box(
    width: 100%,
    stroke: 0.5pt + brand,
    fill: if calc.rem(n, 2) == 0 { brand-light } else { white },
    inset: 7pt,
    radius: 3pt,
  )[
    #align(center)[
      #text(size: 8pt, fill: ink-muted)[Phase #n]
      #v(0.1em)
      #text(size: 9pt, weight: "bold")[#name]
      #v(0.15em)
      #text(size: 7.75pt)[#check]
    ]
  ]
  #grid(
    columns: (1fr, 1fr, 1fr, 1fr, 1fr),
    gutter: 6pt,
    p(1, [Lexer], [token dump]),
    p(2, [Parser], [AST print]),
    p(3, [Compiler], [disasm]),
    p(4, [VM], [`print 1+2`]),
    p(5, [Product], [`make verify`]),
  )
]

#let flow-step(num, label, detail, fill: surface) = box(
  width: 100%,
  fill: fill,
  stroke: 0.45pt + brand,
  radius: 4pt,
  inset: (left: 10pt, rest: 8pt),
)[
  #grid(
    columns: (auto, 1fr),
    column-gutter: 8pt,
    align: horizon,
    box(fill: brand, inset: (x: 6pt, y: 3pt), radius: 2pt)[
      #text(size: 8pt, weight: "bold", fill: white)[#num]
    ],
    [#text(size: 8.75pt, weight: "bold", fill: brand-dark)[#label] #text(size: 8pt, fill: ink-muted)[ — #detail]],
  )
]

#let jump-arrow = align(center)[#text(size: 11pt, fill: brand)[↓]]

#let fig-jump-patch() = figure(
  caption: [
    How `if (x < 100) { print x + 8; }` becomes bytecode: compile the condition, branch on false,
    then patch jump targets once addresses are known.
  ],
  supplement: [Figure],
)[
  #set text(size: 8.5pt)
  #grid(
    columns: (1fr, 1fr),
    gutter: 14pt,
    // Left: source + mental model
    box(width: 100%, stroke: 0.5pt + border, fill: white, radius: 4pt, inset: 10pt)[
      #text(weight: "bold", fill: brand-dark)[Source (`hello.cvm`)]
      #v(0.25em)
      #block(
        width: 100%,
        fill: rgb("#f1f5f9"),
        inset: 8pt,
        radius: 3pt,
      )[
        #set text(font: "Menlo", size: 8pt)
        if (x < 100) {\n
        #h(1em)  print x + 8;\n
        }
      ]
      #v(0.3em)
      #text(weight: "bold", fill: brand-dark)[With `else` (general case)]
      #v(0.15em)
      #block(fill: rgb("#f1f5f9"), inset: 8pt, radius: 3pt, width: 100%)[
        #set text(font: "Menlo", size: 7.75pt)
        if (c) { A } else { B }
      ]
    ],
    // Right: bytecode flow
    box(width: 100%, stroke: 0.5pt + border, fill: brand-light, radius: 4pt, inset: 10pt)[
      #text(weight: "bold", fill: brand-dark)[Bytecode layout (in order)]
      #v(0.2em)
      #flow-step[①][Compile condition][`LOAD x` · `PUSH 100` · `LT` → bool on stack]
      #jump-arrow
      #flow-step[②][`JUMP_IF_FALSE`][placeholder → skip *then* if `false` (patch later)]
      #jump-arrow
      #flow-step[③][Compile *then*][`print x + 8` for `hello.cvm`]
      #jump-arrow
      #flow-step[④][`JUMP` (if `else` exists)][skip `else` block — patch target later]
      #jump-arrow
      #flow-step[⑤][Compile `else` (optional)][`hello.cvm` has no else — skip]
      #jump-arrow
      #flow-step[⑥][`HALT` / next stmt][patch ② → here if false; patch ④ → here if used]
    ],
  )
  #v(0.45em)
  // Bottom: linear address map for hello.cvm
  #text(weight: "bold", fill: brand-dark)[`hello.cvm` — actual offsets after patch]
  #v(0.15em)
  #box(width: 100%, stroke: 0.4pt + border, radius: 4pt, inset: 0pt)[
    #grid(
      columns: (0.42fr, 0.38fr, 0.2fr),
      gutter: 0pt,
      box(fill: rgb("#dbeafe"), inset: 8pt)[
        #align(center)[
          #text(size: 7.5pt, weight: "bold")[bytes 26–38]\
          #text(size: 7pt)[condition: `x < 100`]
        ]
      ],
      box(fill: rgb("#fef3c7"), inset: 8pt)[
        #align(center)[
          #text(size: 7.5pt, weight: "bold")[byte 39]\
          #text(size: 7pt)[`JUMP_IF_FALSE @58`\
          false → skip to HALT path]
        ]
      ],
      box(fill: rgb("#d1fae5"), inset: 8pt)[
        #align(center)[
          #text(size: 7.5pt, weight: "bold")[bytes 44–57]\
          #text(size: 7pt)[*then:* `print x+8`\
          → prints 50]
        ]
      ],
    )
  ]
  #v(0.2em)
  #grid(
    columns: (1fr, 1fr),
    gutter: 10pt,
    box(fill: rgb("#ecfdf5"), stroke: 0.35pt + rgb("#0f766e"), inset: 8pt, radius: 3pt)[
      #text(size: 8pt, weight: "bold", fill: rgb("#0f766e"))[When condition is *true* (`42 < 100`)]
      #v(0.1em)
      #text(size: 8pt)[Do not jump at 39 · run bytes 44–57 · print `50`]
    ],
    box(fill: white, stroke: 0.35pt + border, inset: 8pt, radius: 3pt)[
      #text(size: 8pt, weight: "bold", fill: brand-dark)[When condition is *false*]
      #v(0.1em)
      #text(size: 8pt)[Jump to 58 (`HALT`) · skip bytes 44–57 · then-branch never runs]
    ],
  )
]

// Monospace tree — same style as `cvmpp -d` AST output (`ast_print.cpp`)
#let ast-tree(code) = block(
  width: 100%,
  fill: rgb("#f1f5f9"),
  stroke: 0.35pt + border,
  radius: 3pt,
  inset: 5pt,
)[
  #set text(font: "Menlo", size: 7.25pt)
  #raw(block: true, lang: none, code)
]

#let fig-ast-print-add() = figure(
  caption: [AST for `print 1 + 2;` — expression tree under one `Print` statement.],
  supplement: [Figure],
)[
  #ast-tree("Program\n+-- Print\n    +-- Binary (+)\n        |-- IntLiteral (1)\n        +-- IntLiteral (2)")
]

#let fig-ast-hello() = figure(
  caption: [`hello.cvm` AST from `cvmpp -d`.],
)[
  #ast-tree(
    "Program\n|-- Let (x)\n|   +-- IntLiteral (42)\n|-- Let (flag)\n|   +-- BoolLiteral (true)\n|-- Print\n|   +-- Variable (x)\n+-- If\n    |-- Binary (<)\n    |   |-- Variable (x)\n    |   +-- IntLiteral (100)\n    +-- Block (1 stmt)\n        +-- Print\n            +-- Binary (+)\n                |-- Variable (x)\n                +-- IntLiteral (8)"
  )
]

#let fig-ast-functions() = figure(
  caption: [AST for `functions.cvm` (`cvmpp -d`).],
  supplement: [Figure],
)[
  #ast-tree(
    "Program\n+-- Fn factorial (1 params)\n|   |-- If\n|   |   |-- Binary (<=)\n|   |   |   |-- Variable (n)\n|   |   |   +-- IntLiteral (1)\n|   |   +-- Block\n|   |       +-- Return → IntLiteral (1)\n|   +-- Return\n|       +-- Binary (*)\n|           |-- Variable (n)\n|           +-- Call factorial\n|               +-- Binary (-) n - 1\n|-- Let (x)\n|   +-- Call factorial(5)\n+-- Print → Variable (x)"
  )
]

#let fig-program-shape() = figure(
  caption: [Parser output: one `Program` node with two lists — `fn` declarations vs everything else at top level.],
  supplement: [Figure],
)[
  #align(center)[
    #box(stroke: 0.6pt + brand, fill: brand-light, inset: 8pt, radius: 4pt)[
      #text(weight: "bold", fill: brand-dark)[Program]
      #v(0.35em)
      #grid(
        columns: (1fr, 1fr),
        gutter: 14pt,
        box(stroke: 0.4pt + border, fill: white, inset: 8pt, radius: 3pt)[
          #text(size: 9pt, weight: "bold")[functions\[\]]
          #v(0.1em)
          #text(size: 8.5pt)[`FunctionDecl` …]
        ],
        box(stroke: 0.4pt + border, fill: white, inset: 8pt, radius: 3pt)[
          #text(size: 9pt, weight: "bold")[statements\[\]]
          #v(0.1em)
          #text(size: 8.5pt)[`Let`, `Print`, `If`, …]
        ],
      )
    ]
  ]
]

#let fig-while-flow() = figure(
  caption: [`while` lowering: condition at top of loop; exit jump patched after the body.],
  supplement: [Figure],
)[
  #align(center)[
    #box(stroke: 0.5pt + border, inset: 10pt, radius: 4pt)[
      #text(size: 8.75pt)[
        loop start → compile condition → `JumpIfFalse` (exit)\
        body → `Jump` back to loop start → patch exit label
      ]
    ]
  ]
]

#let opcode-band(hex, name, ops) = block(
  width: 100%,
  fill: brand-light,
  stroke: 0.4pt + brand,
  inset: 8pt,
  radius: 3pt,
)[
  #text(size: 8.5pt, weight: "bold", fill: brand-dark)[#hex  #name]
  #v(0.08em)
  #text(size: 8.25pt)[#ops]
]

#let fig-opcode-map() = figure(
  caption: [Opcode groups in `opcode.hpp` (byte tag + fixed operands).],
  supplement: [Figure],
)[
  #stack(spacing: 5pt)[
    #opcode-band[0x01–0x03][Literals / stack][`PushInt` `PushBool` `Pop`]
    #opcode-band[0x10–0x13][Variables][`LoadVar` `StoreVar` `LoadLocal` `StoreLocal`]
    #opcode-band[0x20–0x2A][Math / compare][`Add`…`Div` `Eq` `Ne` `Lt` `Gt` `Neg` `Le` `Ge`]
    #opcode-band[0x30–0x31][I/O][`Input` `Print`]
    #opcode-band[0x40–0x43][Control / calls][`Jump` `JumpIfFalse` `Call` `Return`]
    #opcode-band[0xFF][Stop][`Halt`]
  ]
]

#let fig-pipeline-stops() = figure(
  caption: [Where `compile_frontend` can stop — later stages never run after a failure.],
  supplement: [Figure],
)[
  #set text(size: 8.5pt)
  #grid(
    columns: 1,
    row-gutter: 4pt,
    box(fill: rgb("#fef2f2"), stroke: 0.4pt + rgb("#dc2626"), inset: 8pt, radius: 3pt, width: 100%)[
      *Lexer fails* → return immediately · exit `1` · parser not called
    ],
    box(fill: rgb("#fff7ed"), stroke: 0.4pt + rgb("#ea580c"), inset: 8pt, radius: 3pt, width: 100%)[
      *Parser fails* → return · exit `1` · compiler not called
    ],
    box(fill: rgb("#fff7ed"), stroke: 0.4pt + rgb("#ea580c"), inset: 8pt, radius: 3pt, width: 100%)[
      *Compiler fails* → return · exit `1` · VM not called
    ],
    box(fill: rgb("#ecfdf5"), stroke: 0.4pt + rgb("#0f766e"), inset: 8pt, radius: 3pt, width: 100%)[
      *All OK* → `main` calls `execute()` · or stops at `-c` without VM
    ],
  )
]

#let fig-if-runtime() = figure(
  caption: [`if (x < 100)` at runtime — `JUMP_IF_FALSE` skips the then-branch when the condition is false.],
  supplement: [Figure],
)[
  #grid(
    columns: (1fr, 1fr),
    gutter: 10pt,
    box(stroke: 0.5pt + rgb("#0f766e"), fill: rgb("#ecfdf5"), inset: 10pt, radius: 4pt)[
      #text(weight: "bold", fill: rgb("#0f766e"))[`x = 42`]
      #v(0.2em)
      #text(size: 8.5pt)[`42 < 100` → `true` on stack\
      `JUMP_IF_FALSE` → *do not* jump\
      Run `print x + 8` → *50*\
      Output: `42` then `50`]
    ],
    box(stroke: 0.5pt + border, fill: white, inset: 10pt, radius: 4pt)[
      #text(weight: "bold", fill: brand-dark)[If `x = 200`]
      #v(0.2em)
      #text(size: 8.5pt)[`200 < 100` → `false`\
      `JUMP_IF_FALSE @58` → jump to `HALT` path\
      Skip then-branch\
      Output: only `42`]
    ],
  )
]

#let fig-bytecode-bar() = figure(
  caption: [
    `functions.cvm`: the compiler lays function code before main in the file, but the VM *runs* main
    first (via `JUMP` at offset 0). `CALL` is what enters `factorial` at byte 5.
  ],
  supplement: [Figure],
)[
  #set text(size: 9pt)
  #text(weight: "bold", fill: brand-dark)[A — Order the VM executes]
  #v(0.2em)
  #align(center)[
    #grid(
      columns: 11,
      column-gutter: 0.1em,
      align: horizon,
      flow-node([Start], sub: [ip = 0]),
      flow-arrow,
      flow-node([JUMP], sub: [→ 52]),
      flow-arrow,
      flow-node([Main], sub: [push 5]),
      flow-arrow,
      flow-node([CALL], sub: [→ fn]),
      flow-arrow,
      flow-node([factorial], sub: [recurse]),
      flow-arrow,
      flow-node([RETURN], sub: [→ main]),
      flow-arrow,
      flow-node([PRINT], sub: [120]),
    )
  ]
  #v(0.45em)
  #text(weight: "bold", fill: brand-dark)[B — Order bytes sit in `code[]` (73 bytes total)]
  #v(0.15em)
  #tbl(
    ("Offsets", "Region", "What is stored there"),
    (
      [0–4], [Entry], [`JUMP` to main — VM must not fall into the function by accident],
      [5–51], [`factorial` body], [Compiled first; entered only through `CALL`],
      [52–71], [Main script], [`PUSH 5` · `CALL` · `STORE x` · `PRINT x`],
      [72], [End], [`HALT`],
    ),
  )
  #v(0.2em)
  #text(size: 8.5pt, fill: ink-muted)[
    Byte 0 is not the start of `factorial` — it is the skip to main. Without that `JUMP`, the VM
    would hit `LOAD_LOCAL` at 5 with no call frame and error out.
  ]
]

#let fig-read-roadmap() = figure(
  caption: [Suggested reading order through this guide.],
  supplement: [Figure],
)[
  #set text(size: 8.75pt)
  #grid(
    columns: (auto, 1fr),
    row-gutter: 6pt,
    box(fill: brand, inset: (x: 6pt, y: 3pt), radius: 2pt)[#text(fill: white, weight: "bold")[1]],
    [Big picture + glossary — what each stage produces],
    box(fill: brand, inset: (x: 6pt, y: 3pt), radius: 2pt)[#text(fill: white, weight: "bold")[2]],
    [`main.cpp` + `compile_frontend` — who calls whom],
    box(fill: brand, inset: (x: 6pt, y: 3pt), radius: 2pt)[#text(fill: white, weight: "bold")[3]],
    [`hello.cvm` walkthrough — follow with `cvmpp -d`],
    box(fill: brand, inset: (x: 6pt, y: 3pt), radius: 2pt)[#text(fill: white, weight: "bold")[4]],
    [`print 1+2` + `functions.cvm` — stack and calls],
    box(fill: brand, inset: (x: 6pt, y: 3pt), radius: 2pt)[#text(fill: white, weight: "bold")[5]],
    [Stage reference + errors + file map — use while reading code],
  )
]

#let fig-bytecode-snippet() = figure(
  caption: [Bytecode excerpt for `hello.cvm` (from `cvmpp -d`) — entry `JUMP`, then main.],
  supplement: [Figure],
)[
  #ast-tree(
    "  0  JUMP           → main\n  5  PUSH_INT       42\n 14  STORE_VAR      x\n ...\n 39  JUMP_IF_FALSE  → exit\n 44  ... then-branch ...\n 58  HALT"
  )
]
