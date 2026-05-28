#import "cvm-theme.typ": brand, brand-dark, brand-light, border, ink-muted, surface

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
    box(width: 100%, stroke: 0.6pt + brand, fill: brand-light, radius: 4pt, inset: 10pt)[
      #text(weight: "bold", fill: brand)[Compile time]
      #v(0.25em)
      #text(size: 9pt)[
        Lexer → Parser → Compiler\
        Produces `BytecodeChunk`\
        Diagnostics: Lexer, Parser, Compiler
      ]
    ],
    box(width: 100%, stroke: 0.6pt + rgb("#0f766e"), fill: rgb("#ecfdf5"), radius: 4pt, inset: 10pt)[
      #text(weight: "bold", fill: rgb("#0f766e"))[Runtime]
      #v(0.25em)
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
  #v(0.35em)
  #text(size: 8.75pt, fill: ink-muted)[
    Parallel metadata: `names[]` (global pool), `functions[]` (name, address, arity).
  ]
]

#let fig-call-protocol() = figure(
  caption: [Function call: arguments on operand stack become callee local slots.],
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

#let fig-jump-patch() = figure(
  caption: [`if` lowering: condition compiled first; false branch jumps around `then` or to `else`.],
  kind: image,
  supplement: [Figure],
)[
  #set text(size: 8.75pt)
  #grid(
    columns: 1,
    row-gutter: 4pt,
    box(fill: surface, inset: 8pt, radius: 3pt, width: 100%)[1. Compile condition expression → stack],
    box(fill: surface, inset: 8pt, radius: 3pt, width: 100%)[2. `JUMP_IF_FALSE` → placeholder offset (else / exit)],
    box(fill: surface, inset: 8pt, radius: 3pt, width: 100%)[3. Compile `then` branch],
    box(fill: surface, inset: 8pt, radius: 3pt, width: 100%)[4. Optional `JUMP` over `else`; patch false target],
    box(fill: surface, inset: 8pt, radius: 3pt, width: 100%)[5. Compile `else`; patch forward jump targets],
  )
]

// Monospace tree — same style as `cvmpp -d` AST output (`ast_print.cpp`)
#let ast-tree(code) = block(
  width: 100%,
  fill: rgb("#f1f5f9"),
  stroke: 0.35pt + border,
  radius: 3pt,
  inset: 7pt,
)[
  #set text(font: "Menlo", size: 7.5pt)
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
  caption: [`Program` root: top-level functions compiled first, then main statements.],
  supplement: [Figure],
)[
  #align(center)[
    #box(stroke: 0.6pt + brand, fill: brand-light, inset: 12pt, radius: 4pt)[
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

#let fig-bytecode-snippet() = figure(
  caption: [Bytecode excerpt for `hello.cvm` (from `cvmpp -d`) — entry `JUMP`, then main.],
  supplement: [Figure],
)[
  #ast-tree(
    "  0  JUMP           → main\n  5  PUSH_INT       42\n 14  STORE_VAR      x\n ...\n 39  JUMP_IF_FALSE  → exit\n 44  ... then-branch ...\n 58  HALT"
  )
]
