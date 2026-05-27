// CVM++ presentation theme (5–6 pages)
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
    #place(left + top, dx: 2cm, dy: 2.2cm)[
      #text(size: 40pt, weight: "bold", fill: white)[CVM#raw("++")]
      #v(0.3cm)
      #text(size: 16pt, fill: rgb("#ffffffcc"))[#subtitle]
      #v(0.8cm)
      #text(size: 11pt, fill: rgb("#ffffffaa"))[#tagline]
      #v(1.5cm)
      #text(size: 10pt, fill: rgb("#ffffff88"))[
        Satish Kumar Munda #sym.dot
        github.com/Dragon9-9/cvmpp #sym.dot
        C++17 Personal Project
      ]
    ]
  ]
  pagebreak()
  set page(margin: (x: 1.8cm, y: 1.7cm))
}

#let slide-setup() = {
  set text(font: ("Helvetica Neue", "Arial"), size: 10.5pt)
  set par(leading: 0.62em, spacing: 0.55em)
  set heading(numbering: none)
  show heading.where(level: 1): it => {
    v(0.15cm)
    text(size: 18pt, weight: "bold", fill: brand-dark)[#it.body]
    v(1pt)
    line(length: 100%, stroke: 1.5pt + accent)
    v(0.35cm)
  }
  show heading.where(level: 2): it => {
    v(0.25cm)
    text(size: 12pt, weight: "bold", fill: brand)[#it.body]
    v(0.2cm)
  }
  set page(
    paper: "a4",
    header: context {
      if counter(page).get().first() > 1 [
        #grid(columns: (1fr, 1fr),
          text(size: 8pt, fill: ink-muted)[CVM++],
          align(right, text(size: 8pt, fill: ink-muted)[#counter(page).display()]),
        )
        #v(-0.15cm)
        line(length: 100%, stroke: 0.4pt + accent.lighten(50%))
      ]
    },
  )
}

#let tbl(headers, rows) = {
  table(
    columns: headers.len(),
    stroke: 0.4pt + brand.lighten(70%),
    fill: (x, y) => if y == 0 { brand.lighten(90%) } else { white },
    inset: 6pt,
    ..headers.map(h => table.cell(text(weight: "bold", size: 9pt)[#h])),
    ..rows.flatten().map(c => table.cell(text(size: 9pt)[#c])),
  )
  v(0.25cm)
}

#let slidebox(body) = {
  block(
    fill: surface,
    stroke: (left: 3pt + accent),
    inset: 10pt,
    radius: (right: 3pt),
    width: 100%,
  )[#body]
  v(0.2cm)
}

#let codeblock(body) = {
  block(
    fill: surface,
    stroke: 0.4pt + brand.lighten(60%),
    radius: 3pt,
    inset: 8pt,
    width: 100%,
  )[
    #set text(font: "Menlo", size: 8.5pt)
    #body
  ]
  v(0.2cm)
}
