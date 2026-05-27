#!/usr/bin/env bash
# Build PDF documentation (Typst — primary).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TYPST="$ROOT/docs/typst"
PDF_OUT="$ROOT/docs/pdf"

if ! command -v typst >/dev/null 2>&1; then
  echo "error: typst is not installed."
  echo "  macOS: brew install typst"
  exit 1
fi

mkdir -p "$PDF_OUT"

echo "Building CVM++ PDFs (Typst)..."
echo ""

typst compile "$TYPST/01-project-build.typ" \
  "$PDF_OUT/CVM++_01_Project_and_Build_Guide.pdf"

typst compile "$TYPST/02-architecture-workflow.typ" \
  "$PDF_OUT/CVM++_02_Architecture_and_Workflow_Guide.pdf"

echo ""
echo "Done:"
ls -la "$PDF_OUT"/*.pdf
