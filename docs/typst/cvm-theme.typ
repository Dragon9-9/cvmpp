// Shared CVM++ documentation theme
#let brand-dark = rgb("#002d5a")
#let brand = rgb("#00529b")
#let accent = rgb("#00a896")
#let surface = rgb("#f4f7fb")
#let ink-muted = rgb("#5a6570")

#let cover(title, subtitle, tagline) = {
  set page(margin: 0pt)
  block(
    width: 100%,
    height: 100%,
    fill: gradient.linear(brand-dark, brand, angle: 135deg),
  )[
    #place(top + left, dx: 2.2cm, dy: 2.4cm)[
      #text(size: 46pt, weight: "bold", fill: white)[CVM#raw("++")]
      #v(0.35cm)
      #text(size: 18pt, fill: rgb("#ffffffcc"))[#subtitle]
      #v(1.2cm)
      #text(size: 12pt, fill: rgb("#ffffffaa"))[#tagline]
    ]
    #place(bottom + left, dx: 2.2cm, dy: 2cm)[
      #text(size: 10pt, fill: rgb("#ffffff99"))[github.com/Dragon9-9/cvmpp  #sym.dot  May 2026]
    ]
  ]
  pagebreak()
  set page(margin: (x: 2.1cm, y: 2.3cm))
}

#let body-setup() = {
  set page(
    paper: "a4",
    header: context {
      if counter(page).get().first() > 1 [
        #grid(
          columns: (1fr, 1fr),
          text(size: 9pt, fill: ink-muted)[CVM++],
          align(right, text(size: 9pt, fill: ink-muted)[#counter(page).display()]),
        )
        line(length: 100%, stroke: 0.5pt + accent.lighten(40%))
      ]
    },
    footer: none,
  )
  set text(font: ("Helvetica Neue", "Arial"), size: 10.5pt, fill: black)
  set par(justify: true, leading: 0.65em, spacing: 0.75em)
  set heading(numbering: none)
  show heading.where(level: 1): it => {
    v(0.6cm)
    text(size: 20pt, weight: "bold", fill: brand-dark)[#it.body]
    v(2pt)
    line(length: 100%, stroke: 1.2pt + accent)
    v(0.45cm)
  }
  show heading.where(level: 2): it => {
    v(0.45cm)
    text(size: 14pt, weight: "bold", fill: brand)[#it.body]
    v(0.3cm)
  }
  show heading.where(level: 3): it => {
    v(0.35cm)
    text(size: 12pt, weight: "bold", fill: ink-muted)[#it.body]
    v(0.2cm)
  }
  show raw.where(block: true): it => {
    block(
      fill: surface,
      stroke: 0.6pt + brand.lighten(60%),
      radius: 4pt,
      inset: 10pt,
      width: 100%,
    )[
      #set text(font: "Menlo", size: 9pt)
      #it
    ]
    v(0.35cm)
  }
}

#let callout(title, body, fill-color: surface, stroke-color: brand) = {
  block(
    fill: fill-color,
    stroke: (left: 3.5pt + stroke-color),
    inset: (left: 14pt, rest: 12pt),
    radius: (right: 4pt),
    width: 100%,
    above: 0.4cm,
    below: 0.4cm,
  )[
    #text(weight: "bold", fill: stroke-color)[#title]
    #v(0.25cm)
    #body
  ]
}

#let pipeline-box(body) = callout("Execution pipeline", body, fill-color: brand.lighten(92%), stroke-color: brand)
#let note-box(body) = callout("Note", body, fill-color: accent.lighten(88%), stroke-color: accent)
#let tip-box(body) = callout("Quick tip", body, fill-color: rgb("#eef6ff"), stroke-color: brand.darken(10%))

#let tbl(headers, rows) = {
  table(
    columns: headers.len(),
    stroke: 0.5pt + brand.lighten(70%),
    fill: (x, y) => if y == 0 { brand.lighten(88%) } else if calc.odd(y) { white } else { surface },
    inset: 8pt,
    ..headers.map(h => table.cell(text(weight: "bold", fill: brand-dark)[#h])),
    ..rows.flatten().map(c => table.cell(c)),
  )
  v(0.35cm)
}
