#!/usr/bin/env bash
set -euo pipefail

shell_bin="${1:-./simple-shell}"

# Test 1: The prompt is displayed and exit terminates the shell.
output="$(printf 'exit\n' | "$shell_bin" 2>&1)"

if [[ "$output" != *"shell> "* ]]; then
    echo "FAIL: command prompt was not displayed" >&2
    exit 1
fi

# Test 2: Blank input is ignored and the shell continues.
output="$(printf '\nexit\n' | "$shell_bin" 2>&1)"

if [[ "$output" != *"shell> "* ]]; then
    echo "FAIL: shell did not handle blank input" >&2
    exit 1
fi

# Test 3: Unmatched quotes are rejected without crashing.
output="$(printf 'echo "unclosed\nexit\n' | "$shell_bin" 2>&1)"

if [[ "$output" != *"malformed command input"* ]]; then
    echo "FAIL: unmatched quotes were not reported" >&2
    exit 1
fi

if [[ "$output" != *"shell> "* ]]; then
    echo "FAIL: shell did not continue after malformed input" >&2
    exit 1
fi

# Test 4: Oversized input is rejected safely.
long_line="$(printf '%1100s' '' | tr ' ' 'x')"
output="$(printf '%s\nexit\n' "$long_line" | "$shell_bin" 2>&1)"

if [[ "$output" != *"command line is too long"* ]]; then
    echo "FAIL: oversized input was not rejected" >&2
    exit 1
fi

if [[ "$output" != *"shell> "* ]]; then
    echo "FAIL: shell did not continue after oversized input" >&2
    exit 1
fi

echo "All shell smoke tests passed."
