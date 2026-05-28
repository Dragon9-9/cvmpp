// CVM++ documentation — shared Typst theme (Project & Build, Architecture guides)
#let brand-dark = rgb("#1e293b")
#let brand = rgb("#0f4c81")
#let brand-light = rgb("#e8f0f8")
#let surface = rgb("#f8fafc")
#let border = rgb("#cbd5e1")
#let ink-muted = rgb("#64748b")
#let ink-body = rgb("#1e293b")
#let status-ok = rgb("#0f766e")

#let doc-title(title, subtitle: none, tight: false) = {
  text(size: if tight { 14pt } else { 18pt }, weight: "bold", fill: brand-dark)[#title]
  if subtitle != none {
    v(if tight { 0.1em } else { 0.15em })
    text(size: if tight { 9pt } else { 10pt }, fill: ink-muted)[#subtitle]
  }
  v(if tight { 0.12em } else { 0.25em })
  line(length: 100%, stroke: 1.15pt + brand)
  v(if tight { 0.1em } else { 0.25em })
}

#let doc-cover(title, subtitle: none, meta: none) = {
  page(
    header: none,
    footer: none,
    margin: (top: 2.4cm, bottom: 2cm, x: 2.2cm),
  )[
    #set align(center + horizon)
    #v(1fr)
    #text(size: 22pt, weight: "bold", fill: brand-dark)[#title]
    #if subtitle != none [
      #v(0.55em)
      #text(size: 12pt, fill: ink-muted)[#subtitle]
    ]
    #v(1.2em)
    #line(length: 42%, stroke: 1.5pt + brand)
    #if meta != none [
      #v(0.9em)
      #text(size: 9.5pt, fill: ink-muted)[#meta]
    ]
    #v(1fr)
    #align(left)[
      #text(size: 8.5pt, fill: ink-muted)[
        Companion to *Project and Build* · Pipeline: lexer → parser → compiler → VM
      ]
    ]
  ]
}

#let doc-outline(depth: 2) = [
  #pagebreak(weak: true)
  #outline(
    title: text(size: 13pt, weight: "bold", fill: brand-dark)[Contents],
    depth: depth,
    indent: auto,
  )
  #v(0.5em)
  #pagebreak(weak: true)
]

// Architecture guide: no TOC, no section numbers, no page numbers, tight layout
#let doc-setup-plain(header-label: "CVM++") = {
  set text(font: ("Helvetica Neue", "Arial"), size: 10pt, fill: ink-body)
  // Avoid stretched spacing from full justification in narrow columns
  set par(justify: false, leading: 0.58em, spacing: 0.3em)
  set block(spacing: 0.35em)
  set figure(placement: none, gap: 0.15em)
  // Keep caption attached to diagram — no floating / orphan captions
  // Allow block breaking to avoid large blank page regions
  show figure: it => block(breakable: true, above: 0.04em, below: 0.05em, width: 100%)[
    #align(center)[#it.body]
    #v(0.05em)
    #text(size: 8pt, fill: ink-muted)[#it.caption]
  ]
  show emph: it => text(fill: brand-dark, style: "italic")[#it.body]
  set page(
    paper: "a4",
    margin: (top: 1.2cm, bottom: 1.1cm, x: 1.45cm),
    fill: white,
    header: none,
    footer: none,
  )
  set heading(numbering: none)
  show heading.where(level: 1): it => {
    v(0.16em)
    text(size: 11.5pt, weight: "bold", fill: brand-dark)[#it.body]
    v(0.08em)
    line(length: 100%, stroke: 0.4pt + brand)
    v(0.12em)
  }
  show heading.where(level: 2): it => {
    v(0.12em)
    text(size: 10.25pt, weight: "bold", fill: brand)[#it.body]
    v(0.06em)
  }
  show raw.where(block: true): set text(font: "Menlo", size: 8.5pt)
  show raw.where(block: true): block(
    fill: rgb("#f1f5f9"),
    stroke: 0.35pt + border,
    radius: 2pt,
    inset: 6pt,
    width: 100%,
    above: 0.08em,
    below: 0.08em,
  )
}

#let doc-setup(header-label: "CVM++") = {
  set text(font: ("Helvetica Neue", "Arial"), size: 10.5pt, fill: ink-body)
  set par(justify: true, leading: 0.64em, spacing: 0.38em)
  show figure: it => {
    set align(center)
    block(above: 0.15em, below: 0.2em)[#it.body]
    text(size: 8pt, fill: ink-muted)[#it.caption]
  }
  show emph: it => text(fill: brand-dark, style: "italic")[#it.body]
  set page(
    paper: "a4",
    margin: (top: 1.85cm, bottom: 1.65cm, x: 2cm),
    fill: white,
    header: context {
      if counter(page).get().first() > 1 [
        #grid(
          columns: (1fr, auto),
          text(size: 8pt, fill: ink-muted)[#header-label],
          text(size: 8pt, fill: ink-muted)[#counter(page).display("1 / 1", both: true)],
        )
        #v(0.12em)
        #line(length: 100%, stroke: 0.35pt + border)
      ]
    },
  )
  set heading(numbering: "1.")
  show heading.where(level: 1): set heading(numbering: "1.")
  show heading.where(level: 2): set heading(numbering: "1.1")
  show heading.where(level: 3): set heading(numbering: "1.1.1")
  show heading.where(level: 1): it => {
    v(0.35em)
    block(breakable: false)[
      #text(size: 12.5pt, weight: "bold", fill: brand-dark)[
        #counter(heading).display()#h(0.3em)#it.body
      ]
      #v(0.08em)
      #line(length: 100%, stroke: 0.55pt + brand)
    ]
    v(0.18em)
  }
  show heading.where(level: 2): it => {
    v(0.22em)
    text(size: 10.5pt, weight: "bold", fill: brand)[
      #counter(heading).display()#h(0.25em)#it.body
    ]
    v(0.08em)
  }
  show heading.where(level: 3): it => {
    v(0.15em)
    text(size: 10pt, weight: "bold", fill: brand-dark)[
      #counter(heading).display()#h(0.25em)#it.body
    ]
    v(0.06em)
  }
  show raw.where(block: true): set text(font: "Menlo", size: 8.75pt)
  show raw.where(block: true): block(
    fill: rgb("#f1f5f9"),
    stroke: 0.35pt + border,
    radius: 3pt,
    inset: 8pt,
    width: 100%,
    above: 0.2em,
    below: 0.2em,
  )
}

#let flow-diagram() = {
  let stage(l, sub: none) = box(
    stroke: 0.5pt + brand,
    fill: brand-light,
    inset: (x: 7pt, y: 5pt),
    radius: 3pt,
  )[
    #align(center)[
      #text(size: 8.75pt, weight: "bold")[#l]
      #if sub != none [
        #v(0.05em)
        #text(size: 7.5pt, fill: ink-muted)[#sub]
      ]
    ]
  ]
  let arr = text(size: 9pt, fill: brand)[$arrow.r$]
  v(0.1em)
  align(center)[
    #grid(
      columns: 11,
      column-gutter: 0.12em,
      align: horizon,
      stage[.cvm], arr,
      stage([Lexer], sub: [tokens]), arr,
      stage([Parser], sub: [AST]), arr,
      stage([Compiler], sub: [bytes]), arr,
      stage([VM], sub: [run]), arr,
      stage[Out],
    )
  ]
  v(0.1em)
}

#let stage-head(num, title, file) = block(
  width: 100%,
  fill: brand-light,
  stroke: (left: 4pt + brand),
  inset: (left: 12pt, rest: 10pt),
  radius: (right: 4pt),
  above: 0.35em,
  below: 0.2em,
)[
  #grid(
    columns: (auto, 1fr, auto),
    gutter: 10pt,
    align: horizon,
    box(fill: brand, inset: (x: 8pt, y: 4pt), radius: 3pt)[
      #text(size: 9pt, weight: "bold", fill: white)[#num]
    ],
    text(size: 11pt, weight: "bold", fill: brand-dark)[#title],
    text(size: 8.5pt, fill: ink-muted, font: "Menlo")[#file],
  )
]

#let terminal(body, tight: false) = block(
  width: 100%,
  fill: rgb("#1e293b"),
  radius: 4pt,
  inset: if tight { 7pt } else { 10pt },
  above: if tight { 0.06em } else { 0.15em },
  below: if tight { 0.06em } else { 0.2em },
)[
  #set text(font: "Menlo", size: if tight { 8.25pt } else { 8.75pt }, fill: rgb("#e2e8f0"))
  #body
]

#let tip(title, body) = block(
  width: 100%,
  stroke: (left: 3pt + rgb("#0f766e")),
  fill: rgb("#ecfdf5"),
  inset: (left: 10pt, rest: 9pt),
  radius: (right: 3pt),
)[
  #text(size: 9pt, weight: "bold", fill: rgb("#0f766e"))[#title]
  #v(0.15em)
  #set par(spacing: 0.38em)
  #body
]

#let callout(title, body) = block(
  width: 100%,
  stroke: (left: 3pt + brand),
  fill: brand-light,
  inset: (left: 10pt, rest: 9pt),
  radius: (right: 3pt),
)[
  #text(size: 9pt, weight: "bold", fill: brand)[#title]
  #v(0.18em)
  #set par(spacing: 0.38em)
  #body
]

#let lbl(body) = text(weight: "bold", fill: brand, size: 9.5pt)[#body]

#let io-block(input, output, file: none) = block(
  width: 100%,
  fill: surface,
  stroke: 0.35pt + border,
  inset: 10pt,
  radius: 4pt,
  above: 0.12em,
  below: 0.2em,
)[
  #grid(
    columns: (1.1cm, 1fr),
    row-gutter: 6pt,
    [#lbl[In]], [#input],
    [#lbl[Out]], [#output],
    ..if file != none { ([#lbl[File]], [#text(font: "Menlo", size: 9pt)[#file]]) },
  )
]

#let step(num, title, file: none, body) = [
  #block(above: 0.14em, below: 0.06em)[
    #text(weight: "bold", fill: brand-dark)[Step #num — #title]
    #if file != none [
      #h(0.35em)
      #text(size: 8.25pt, font: "Menlo", fill: ink-muted)[#file]
    ]
    #v(0.08em)
    #set par(spacing: 0.28em)
    #body
  ]
]

#let component(num, title, file, input, output, flow, example) = [
  #stage-head(num, title, file)
  #io-block(input, output, file: file)
  #v(0.12em)
  #lbl[Flow] #h(0.35em) #flow
  #v(0.18em)
  #lbl[Example] #h(0.35em) #example
  #v(0.28em)
]

#let phase-detail(files, work, verify) = [
  #v(0.12em)
  #lbl[Files] #h(0.35em) #text(size: 9.5pt)[#files]
  #v(0.2em)
  #lbl[Work] #h(0.35em)
  #work
  #v(0.2em)
  #lbl[Verification] #h(0.35em) #text(size: 9.5pt)[#verify]
]

#let defn(term, body) = block(
  width: 100%,
  fill: surface,
  stroke: (left: 2.5pt + brand),
  inset: (left: 8pt, rest: 5pt),
  radius: (right: 2pt),
  above: 0.06em,
  below: 0.04em,
)[
  #grid(
    columns: (2.35cm, 1fr),
    gutter: 8pt,
    align: (left, left),
    text(weight: "bold", fill: brand-dark, size: 9.5pt)[#term],
    text(size: 9.5pt)[#body],
  )
]

#let ref-section(title, body) = block(
  width: 100%,
  above: 0.15em,
  below: 0.08em,
)[
  #text(size: 10pt, weight: "bold", fill: brand)[#title]
  #v(0.06em)
  #body
]

#let ex(title, note, body) = block(
  width: 100%,
  stroke: 0.4pt + border,
  fill: white,
  inset: 9pt,
  radius: 3pt,
)[
  #text(size: 9.5pt, weight: "bold", fill: brand-dark)[#title]
  #v(0.18em)
  #body
  #v(0.15em)
  #text(size: 8.5pt, fill: ink-muted)[#note]
]

// Worked example: heading + source + explanation + run + output
#let worked-example(title, file, summary, source, explain, run, out, exit: [0]) = [
  === #title
  #block(
    width: 100%,
    stroke: 0.4pt + border,
    fill: white,
    inset: 10pt,
    radius: 4pt,
    above: 0.12em,
    below: 0.28em,
  )[
  #text(size: 9pt, fill: ink-muted)[`examples/#file`]
  #v(0.1em)
  #text(size: 9.5pt)[#summary]
  #v(0.18em)
  #lbl[Source]
  #source
  #v(0.16em)
  #lbl[Explanation]
  #set par(spacing: 0.36em)
  #explain
  #v(0.16em)
  #lbl[Run]
  #text(font: "Menlo", size: 8.75pt)[#run]
  #v(0.1em)
  #lbl[Output]
  #block(
    width: 100%,
    fill: rgb("#f1f5f9"),
    stroke: 0.3pt + border,
    radius: 3pt,
    inset: 8pt,
  )[
    #set text(font: "Menlo", size: 8.75pt)
    #out
  ]
  #v(0.08em)
  #text(size: 8.5pt, fill: ink-muted)[Exit code #exit]
  ]
]

// Featured .cvm example: purpose, run line, I/O, source
#let example-card(file, purpose, run, stdout, exit, source) = block(
  width: 100%,
  stroke: 0.4pt + border,
  fill: white,
  inset: 10pt,
  radius: 4pt,
)[
  #text(size: 10pt, weight: "bold", fill: brand)[`examples/#file`]
  #v(0.15em)
  #text(size: 9.25pt)[#purpose]
  #v(0.2em)
  #text(size: 8.75pt, font: "Menlo")[#run]
  #text(size: 9pt, fill: ink-muted)[ #h(0.3em) → #stdout #h(0.3em) · exit #exit]
  #v(0.22em)
  #text(size: 8.5pt, weight: "bold", fill: ink-muted)[Source]
  #v(0.1em)
  #source
]

// One phase = plain summary + short bullets + files + one-line check
#let phase-card(plain, built, files, check, extra: none) = block(
  width: 100%,
  fill: surface,
  stroke: 0.35pt + border,
  inset: 10pt,
  radius: 4pt,
  above: 0.25em,
  below: 0.2em,
)[
  #text(size: 10pt, fill: brand-dark)[#plain]
  #v(0.22em)
  #lbl[Built]
  #set list(indent: 0.9em, spacing: 0.32em)
  #built
  #v(0.18em)
  #lbl[Files] #h(0.3em) #text(size: 9pt)[#files]
  #v(0.12em)
  #lbl[Check] #h(0.3em) #text(size: 9pt, weight: "medium")[#check]
  #if extra != none [
    #v(0.2em)
    #extra
  ]
]

#let tbl(headers, rows) = {
  table(
    columns: headers.len(),
    stroke: 0.32pt + border,
    fill: (x, y) => if y == 0 {
      brand-light
    } else if calc.rem(y, 2) == 0 {
      white
    } else {
      rgb("#fafbfc")
    },
    inset: (x: 6pt, y: 4pt),
    align: (left + horizon,),
    ..headers.map(h => table.cell(text(weight: "bold", size: 9pt, fill: brand-dark)[#h])),
    ..rows.flatten().map(c => table.cell(text(size: 9pt)[#c])),
  )
  v(0.12em)
}

#let tbl-tight(headers, rows) = {
  table(
    columns: headers.len(),
    stroke: 0.32pt + border,
    fill: (x, y) => if y == 0 {
      brand-light
    } else if calc.rem(y, 2) == 0 {
      white
    } else {
      rgb("#fafbfc")
    },
    inset: (x: 6pt, y: 3.5pt),
    align: (left + horizon,),
    ..headers.map(h => table.cell(text(weight: "bold", size: 9.2pt, fill: brand-dark)[#h])),
    ..rows.flatten().map(c => table.cell(text(size: 9.2pt)[#c])),
  )
}

#let cvm-sample-tight(code) = {
  raw(block: true, lang: none, code)
}

// Non-floating diagram + caption (Architecture guide — avoids page-break gaps)
#let diagram(caption, body) = block(breakable: true, above: 0.08em, below: 0.14em, width: 100%)[
  #body
  #v(0.08em)
  #text(size: 8pt, fill: ink-muted)[#caption]
]

// One build phase: short prose blocks
#let phase-block(num, title, what, built, check) = [
  == Phase #num — #title
  #text(size: 10pt)[#what]
  #v(0.2em)
  #lbl[What we built] #text(size: 10pt)[#built]
  #v(0.15em)
  #lbl[How we checked] #text(size: 10pt)[#check]
  #v(0.25em)
]

#let done = text(size: 9pt, fill: status-ok, weight: "semibold")[Complete]

#let bytecode-layout() = block(
  width: 100%,
  stroke: (left: 2pt + brand),
  fill: brand-light,
  inset: (left: 8pt, rest: 6pt),
  radius: (right: 2pt),
  above: 0.1em,
  below: 0.1em,
)[
  #text(size: 9pt, weight: "bold", fill: brand)[Bytecode layout]
  #v(0.1em)
  #tbl(
    ("Order in `code[]`", "What lives there"),
    (
      [Start], [`JUMP` → skip to main],
      [Middle], [All `fn` bodies (low addresses)],
      [After patch], [Main script statements],
      [End], [`HALT`],
    ),
  )
]

#let cvm-sample(code) = {
  raw(block: true, lang: none, code)
  v(0.1em)
}
