#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CVMPP="${CVMPP:-$ROOT/build/cvmpp}"

if [[ ! -x "$CVMPP" ]]; then
  echo "error: $CVMPP not found. Run: make"
  exit 1
fi

pass=0
fail=0

run_expect() {
  local file="$1"
  local expected_exit="$2"
  set +e
  "$CVMPP" -q "$file" >/dev/null 2>&1
  local code=$?
  set -e
  if [[ "$code" -eq "$expected_exit" ]]; then
    echo "  ok  $(basename "$file") (exit $code)"
    pass=$((pass + 1))
  else
    echo "  FAIL $(basename "$file") (expected $expected_exit, got $code)"
    fail=$((fail + 1))
  fi
}

echo "CVM++ verify"
echo ""

for f in \
  hello.cvm arithmetic.cvm booleans.cvm if_else.cvm \
  factorial.cvm multiline_demo.cvm assignment.cvm \
  functions.cvm comparisons.cvm; do
  run_expect "$ROOT/examples/$f" 0
done

set +e
echo 21 | "$CVMPP" -q "$ROOT/examples/input_demo.cvm" >/dev/null 2>&1
[[ $? -eq 0 ]] && echo "  ok  input_demo.cvm" && pass=$((pass+1)) || { echo "  FAIL input_demo"; fail=$((fail+1)); }
set -e

run_expect "$ROOT/examples/div_by_zero.cvm" 2

echo ""
echo "Results: $pass passed, $fail failed"
[[ "$fail" -eq 0 ]]
