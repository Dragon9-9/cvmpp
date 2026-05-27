# CVM++ Documentation

Official documentation for the public repository. **Start with the PDFs** if you want a printable, structured read; use the Markdown sources to edit and regenerate.

## PDF guides (recommended)

| Document | File | Contents |
|----------|------|----------|
| **Project & Build Guide** | [pdf/CVM++_01_Project_and_Build_Guide.pdf](pdf/CVM++_01_Project_and_Build_Guide.pdf) | Clone, build, repo layout, step-by-step construction path, full glossary |
| **Architecture & Workflow** | [pdf/CVM++_02_Architecture_and_Workflow_Guide.pdf](pdf/CVM++_02_Architecture_and_Workflow_Guide.pdf) | Pipeline, each stage I/O, opcodes, AST, worked traces, REPL vs file |

## Regenerate PDFs

Requires [Pandoc](https://pandoc.org/) and LaTeX (`pdflatex`):

```bash
./scripts/build-docs.sh
```

Sources live in `guides/`; styling in `pandoc/defaults.yaml`.

## Markdown sources

- [guides/01-Project-Build-Guide.md](guides/01-Project-Build-Guide.md)
- [guides/02-Architecture-Workflow-Guide.md](guides/02-Architecture-Workflow-Guide.md)

## Other

- [../README.md](../README.md) — project home
- [../CV.md](../CV.md) — resume bullets (optional)
