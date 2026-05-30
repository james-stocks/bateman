#!/usr/bin/env bash
set -euo pipefail

BATEMAN="${BATEMAN:-build/bateman}"
TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

PASS=0
FAIL=0

pass() { echo "PASS: $1"; PASS=$((PASS + 1)); }
fail() { echo "FAIL: $1 -- $2"; FAIL=$((FAIL + 1)); }

compile() {
    local src="$1" out="$2"
    "$BATEMAN" "$src" "$out" >/dev/null 2>&1
}

# ---------------------------------------------------------------------------
# dorsia: prints "Hello, Dorsia?"
# ---------------------------------------------------------------------------
BIN="$TMPDIR/dorsia"
compile test/dorsia.bateman "$BIN"
OUTPUT="$("$BIN" 2>/dev/null)"
if [ "$OUTPUT" = '"Hello, Dorsia?"' ]; then
    pass "dorsia prints greeting"
else
    fail "dorsia prints greeting" "got: $OUTPUT"
fi

# ---------------------------------------------------------------------------
# scoping: main's 'result' is 10 even after calling a function that sets
#          its own local 'result' to 99
# ---------------------------------------------------------------------------
BIN="$TMPDIR/scoping"
compile test/scoping.bateman "$BIN"
OUTPUT="$("$BIN" 2>/dev/null)"
if [ "$OUTPUT" = "10" ]; then
    pass "scoping: main result unchanged after function call"
else
    fail "scoping: main result unchanged after function call" "got: $OUTPUT"
fi

# ---------------------------------------------------------------------------
# safe_div (valid inputs): 8 / 2 = 4
# ---------------------------------------------------------------------------
BIN="$TMPDIR/safe_div"
compile test/input.bateman "$BIN"
OUTPUT="$(printf '8\n2\n' | "$BIN" 2>/dev/null)"
if [ "$OUTPUT" = "4" ]; then
    pass "safe_div: 8 / 2 = 4"
else
    fail "safe_div: 8 / 2 = 4" "got: $OUTPUT"
fi

# ---------------------------------------------------------------------------
# div_zero: calling safe_div(8, 0) must raise, exit non-zero, and print the
#           exception message
# ---------------------------------------------------------------------------
BIN="$TMPDIR/div_zero"
compile test/div_zero.bateman "$BIN"
COMBINED_OUTPUT="$("$BIN" 2>&1 || true)"
EXIT_CODE=0; "$BIN" >/dev/null 2>/dev/null || EXIT_CODE=$?
if [ "$EXIT_CODE" -ne 0 ] && echo "$COMBINED_OUTPUT" | grep -q "Division by zero"; then
    pass "div_zero: raises and exits non-zero on division by zero"
else
    fail "div_zero: raises and exits non-zero on division by zero" \
         "exit=$EXIT_CODE output=$COMBINED_OUTPUT"
fi

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
echo ""
echo "Results: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
