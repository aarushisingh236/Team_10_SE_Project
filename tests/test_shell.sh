
#!/usr/bin/env bash
set -euo pipefail

shell_bin="${1:-./simple-shell}"

# Test 1: Prompt appears and exit works.
output="$(printf 'exit\n' | "$shell_bin" 2>&1)"
if [[ "$output" != *"shell> "* ]]; then
    echo "FAIL: prompt was not displayed" >&2
    exit 1
fi

# Test 2: Quoted text remains a single argument when echoed.
output="$(printf 'echo "hello world"\nexit\n' | "$shell_bin" 2>&1)"
if [[ "$output" != *"hello world"* ]]; then
    echo "FAIL: quoted argument was not preserved" >&2
    exit 1
fi

# Test 3: Blank input is handled safely.
output="$(printf '\nexit\n' | "$shell_bin" 2>&1)"

# Test 4: Unmatched quotes produce an error and the shell continues.
output="$(printf 'echo "unclosed\nexit\n' | "$shell_bin" 2>&1)"
if [[ "$output" != *"malformed command input"* ]]; then
    echo "FAIL: unmatched quote was not reported" >&2
    exit 1
fi

if [[ "$output" != *"shell> "* ]]; then
    echo "FAIL: shell did not continue after malformed input" >&2
    exit 1
fi

# Test 5: Oversized input is rejected safely.
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

# Use a temporary directory so tests do not leave files in the repository.
test_dir="$(mktemp -d)"
trap 'rm -rf "$test_dir"' EXIT

input_file="$test_dir/input.txt"
output_file="$test_dir/output.txt"

# Test 6: Input redirection reads from a file.
printf 'banana\napple\ncherry\n' > "$input_file"
output="$(printf 'sort < %s\nexit\n' "$input_file" | "$shell_bin" 2>&1)"

if [[ "$output" != *"apple"* ||
      "$output" != *"banana"* ||
      "$output" != *"cherry"* ]]; then
    echo "FAIL: input redirection did not read the file" >&2
    exit 1
fi

# Test 7: Missing input file produces an error without crashing.
output="$(printf 'sort < %s\nexit\n' "$test_dir/missing.txt" |
    "$shell_bin" 2>&1)"

if [[ "$output" != *"cannot open input file"* ]]; then
    echo "FAIL: missing input file was not reported" >&2
    exit 1
fi

# Test 8: Missing input filename produces a syntax error.
output="$(printf 'sort <\nexit\n' | "$shell_bin" 2>&1)"

if [[ "$output" != *"input redirection requires a filename"* ]]; then
    echo "FAIL: missing input filename was not reported" >&2
    exit 1
fi

# Test 9: Output redirection works for the echo built-in.
printf 'echo hello > %s\nexit\n' "$output_file" | "$shell_bin" >/dev/null

if [[ ! -f "$output_file" ]] ||
   [[ "$(cat "$output_file")" != "hello" ]]; then
    echo "FAIL: output redirection for echo did not work" >&2
    exit 1
fi

# Test 10: Output redirection truncates an existing file.
printf 'old content\n' > "$output_file"
printf 'echo new content > %s\nexit\n' "$output_file" | "$shell_bin" >/dev/null

if [[ "$(cat "$output_file")" != "new content" ]]; then
    echo "FAIL: output redirection did not truncate the existing file" >&2
    exit 1
fi

# Test 11: Output redirection works for an external command.
printf 'printf external-output > %s\nexit\n' "$output_file" | "$shell_bin" >/dev/null

if [[ "$(cat "$output_file")" != "external-output" ]]; then
    echo "FAIL: output redirection for external command did not work" >&2
    exit 1
fi

# Test 12: Missing output filename produces a syntax error.
output="$(printf 'echo hello >\nexit\n' | "$shell_bin" 2>&1)"

if [[ "$output" != *"output redirection requires a filename"* ]]; then
    echo "FAIL: missing output filename was not reported" >&2
    exit 1
fi

echo "All shell smoke tests passed."
