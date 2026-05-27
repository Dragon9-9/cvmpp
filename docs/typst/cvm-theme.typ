// CVM++ documentation theme — readable, self-contained guides
#let brand-dark = rgb("#002d5a")
#let brand = rgb("#00529b")
#let accent = rgb("#00a896")
#let surface = rgb("#f4f7fb")
#let ink-muted = rgb("#5a6570")

#let cover(title, subtitle) = {
  set page(margin: 0pt)
  block(width: 100%, height: 100%, fill: gradient.linear(brand-dark, brand, angle: 135deg))[
    #place(left + top, dx: 2cm, dy: 2cm)[
      #text(size: 38pt, weight: "bold", fill: white)[CVM#raw("++")]
      #v(0.35cm)
      #text(size: 15pt, fill: rgb("#ffffffcc"))[#subtitle]
      #v(1cm)
      #text(size: 10pt, fill: rgb("#ffffffaa"))[
        A self-contained guide — read top to bottom.\
        github.com/Dragon9-9/cvmpp · Satish Kumar Munda · MIT License
      ]
    ]
  ]
  pagebreak()
  set page(margin: (x: 2cm, y: 2.1cm))
}

#let doc-setup() = {
  set text(font: ("Helvetica Neue", "Arial"), size: 10.5pt)
  set par(justify: true, leading: 0.68em, spacing: 0.65em, first-line-indent: 0pt)
  set heading(numbering: none)
  show heading.where(level: 1): it => {
    v(0.9em, weak: true)
    text(size: 17pt, weight: "bold", fill: brand-dark)[#it.body]
    v(0.15em)
    line(length: 100%, stroke: 1pt + accent)
    v(0.5em)
  }
  show heading.where(level: 2): it => {
    v(0.65em, weak: true)
    text(size: 13pt, weight: "bold", fill: brand)[#it.body]
    v(0.35em)
  }
  show heading.where(level: 3): it => {
    v(0.45em, weak: true)
    text(size: 11.5pt, weight: "bold", fill: ink-muted)[#it.body]
    v(0.25em)
  }
  show raw.where(block: true): set text(font: "Menlo", size: 9pt)
  show raw.where(block: true): block(
    fill: surface, stroke: 0.4pt + brand.lighten(55%), radius: 3pt,
    inset: 9pt, width: 100%, above: 0.35em, below: 0.35em,
  )
  set page(
    paper: "a4",
    header: context {
      if counter(page).get().first() > 1 [
        #grid(columns: (1fr, 1fr),
          text(size: 8.5pt, fill: ink-muted)[CVM++ Documentation],
          align(right, text(size: 8.5pt, fill: ink-muted)[#counter(page).display()]),
        )
        #v(-0.1em)
        line(length: 100%, stroke: 0.35pt + accent.lighten(45%))
      ]
    },
  )
}

#let explain(title, body) = block(
  fill: accent.lighten(92%), stroke: (left: 3.5pt + accent),
  inset: (left: 12pt, rest: 10pt), radius: (right: 3pt),
  width: 100%, above: 0.4em, below: 0.4em,
)[
  #text(weight: "bold", fill: brand-dark)[#title]\
  #body
]

#let note(body) = block(
  fill: brand.lighten(94%), stroke: (left: 3.5pt + brand),
  inset: (left: 12pt, rest: 10pt), radius: (right: 3pt),
  width: 100%, above: 0.35em, below: 0.35em,
)[#text(weight: "bold")[In plain words:] #body]

#let tbl(headers, rows) = {
  table(
    columns: headers.len(),
    stroke: 0.45pt + brand.lighten(65%),
    fill: (x, y) => if y == 0 { brand.lighten(88%) } else if calc.rem(y, 2) == 0 { white } else { surface },
    inset: 7pt,
    ..headers.map(h => table.cell(text(weight: "bold", size: 9.5pt)[#h])),
    ..rows.flatten().map(c => table.cell(text(size: 9.5pt)[#c])),
  )
  v(0.4em)
}
