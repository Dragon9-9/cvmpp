#!/usr/bin/env bash
# Regenerate PDF documentation from Markdown guides.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
GUIDES="$ROOT/docs/guides"
PDF_OUT="$ROOT/docs/pdf"
DEFAULTS="$ROOT/docs/pandoc/defaults.yaml"

if ! command -v pandoc >/dev/null 2>&1; then
  echo "error: pandoc is not installed."
  echo "  macOS:  brew install pandoc && brew install --cask mactex-no-gui"
  echo "  Ubuntu: sudo apt install pandoc texlive-latex-base texlive-fonts-recommended"
  exit 1
fi

if ! command -v xelatex >/dev/null 2>&1 && ! command -v pdflatex >/dev/null 2>&1; then
  echo "error: xelatex/pdflatex not found (install a LaTeX distribution)."
  exit 1
fi

mkdir -p "$PDF_OUT"

build_one() {
  local src="$1"
  local out="$2"
  local title="$3"
  echo "  → $(basename "$out")"
  pandoc "$src" \
    --defaults="$DEFAULTS" \
    --include-in-header="$ROOT/docs/pandoc/header.tex" \
    -o "$out" \
    -V title="$title"
}

echo "Building CVM++ documentation PDFs..."
echo ""

build_one \
  "$GUIDES/01-Project-Build-Guide.md" \
  "$PDF_OUT/CVM++_01_Project_and_Build_Guide.pdf" \
  "CVM++ Project and Build Guide"

build_one \
  "$GUIDES/02-Architecture-Workflow-Guide.md" \
  "$PDF_OUT/CVM++_02_Architecture_and_Workflow_Guide.pdf" \
  "CVM++ Architecture and Workflow Guide"

echo ""
echo "Done. Output:"
ls -la "$PDF_OUT"/*.pdf
