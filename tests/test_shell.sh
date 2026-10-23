#!/usr/bin/env bash
set -euo pipefail

shell_bin="${1:-./simple-shell}"
output="$(printf 'exit\n' | "$shell_bin")"

if [[ "$output" != *"shell> "* ]]; then
  echo "FAIL: prompt was not displayed" >&2
  exit 1
fi

# The shell must exit successfully after receiving the exit built-in.
printf 'exit\n' | "$shell_bin" >/dev/null

# An overlong input should be rejected and the shell should continue to exit.
long_line="$(printf '%1100s' '' | tr ' ' 'x')"
output="$(printf '%s\nexit\n' "$long_line" | "$shell_bin" 2>&1)"
if [[ "$output" != *"command line is too long"* ]]; then
  echo "FAIL: oversized input was not rejected with a clear message" >&2
  exit 1
fi

echo "All shell smoke tests passed."
