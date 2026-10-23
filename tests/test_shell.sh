#!/usr/bin/env bash
set -euo pipefail

shell_bin="${1:-./simple-shell}"

# Prompt appears when the shell starts.
output="$(printf 'exit\n' | "$shell_bin" 2>&1)"
if [[ "$output" != *"shell> "* ]]; then
    echo "FAIL: prompt was not displayed" >&2
    exit 1
fi

# Blank input should not crash the shell.
output="$(printf '\nexit\n' | "$shell_bin" 2>&1)"

# Unmatched quotes should produce an error, then allow exit.
output="$(printf 'echo "unclosed\nexit\n' | "$shell_bin" 2>&1)"
if [[ "$output" != *"malformed command input"* ]]; then
    echo "FAIL: unmatched quote was not reported" >&2
    exit 1
fi

# Oversized input must be rejected safely.
long_line="$(printf '%1100s' '' | tr ' ' 'x')"
output="$(printf '%s\nexit\n' "$long_line" | "$shell_bin" 2>&1)"
if [[ "$output" != *"command line is too long"* ]]; then
    echo "FAIL: oversized input was not rejected" >&2
    exit 1
fi

echo "All shell smoke tests passed."
