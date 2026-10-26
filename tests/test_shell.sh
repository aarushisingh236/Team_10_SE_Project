
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

# Use a temporary directory so tests leave no files in the repository.
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
printf 'echo hello > %s\nexit\n' "$output_file" |
    "$shell_bin" >/dev/null

if [[ ! -f "$output_file" ]] ||
   [[ "$(cat "$output_file")" != "hello" ]]; then
    echo "FAIL: output redirection for echo did not work" >&2
    exit 1
fi

# Test 10: Output redirection truncates an existing file.
printf 'old content\n' > "$output_file"
printf 'echo new content > %s\nexit\n' "$output_file" |
    "$shell_bin" >/dev/null

if [[ "$(cat "$output_file")" != "new content" ]]; then
    echo "FAIL: output redirection did not truncate the existing file" >&2
    exit 1
fi

# Test 11: Output redirection works for an external command.
printf 'printf external-output > %s\nexit\n' "$output_file" |
    "$shell_bin" >/dev/null

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

# Test 13: Append redirection preserves existing file contents.
printf 'first\n' > "$output_file"
printf 'echo second >> %s\nexit\n' "$output_file" |
    "$shell_bin" >/dev/null

if [[ "$(cat "$output_file")" != $'first\nsecond' ]]; then
    echo "FAIL: append redirection did not preserve existing contents" >&2
    exit 1
fi

# Test 14: Multiple append commands append in order.
printf 'echo third >> %s\necho fourth >> %s\nexit\n' \
    "$output_file" "$output_file" | "$shell_bin" >/dev/null

if [[ "$(cat "$output_file")" != $'first\nsecond\nthird\nfourth' ]]; then
    echo "FAIL: multiple append commands did not preserve order" >&2
    exit 1
fi

# Test 15: Append redirection creates a file that does not exist.
new_file="$test_dir/new_output.txt"
printf 'echo created >> %s\nexit\n' "$new_file" |
    "$shell_bin" >/dev/null

if [[ ! -f "$new_file" ]] ||
   [[ "$(cat "$new_file")" != "created" ]]; then
    echo "FAIL: append redirection did not create a new file" >&2
    exit 1
fi

# Test 16: Append redirection works for an external command.
# The existing file ends with a newline after "fourth".
# printf does not add a newline after "external".
printf 'printf external >> %s\nexit\n' "$output_file" |
    "$shell_bin" >/dev/null

if [[ "$(cat "$output_file")" != $'first\nsecond\nthird\nfourth\nexternal' ]]; then
    echo "FAIL: append redirection for an external command did not work" >&2
    exit 1
fi

# Test 17: Missing append filename produces a syntax error.
output="$(printf 'echo hello >>\nexit\n' | "$shell_bin" 2>&1)"

if [[ "$output" != *"output redirection requires a filename"* ]]; then
    echo "FAIL: missing append filename was not reported" >&2
    exit 1
fi

# Test 18: Missing input file reports the filename and failure reason.
missing_input="$test_dir/missing_input.txt"
output="$(printf 'sort < %s\nexit\n' "$missing_input" |
    "$shell_bin" 2>&1)"

if [[ "$output" != *"$missing_input"* ||
      "$output" != *"No such file or directory"* ]]; then
    echo "FAIL: missing input error did not identify the file and reason" >&2
    exit 1
fi

# Test 19: Unavailable output directory reports the filename and reason.
missing_output="$test_dir/missing_dir/output.txt"
output="$(printf 'echo hello > %s\nexit\n' "$missing_output" |
    "$shell_bin" 2>&1)"

if [[ "$output" != *"$missing_output"* ||
      "$output" != *"No such file or directory"* ]]; then
    echo "FAIL: output-open error did not identify the file and reason" >&2
    exit 1
fi

# Test 20: Shell remains usable after an input redirection failure.
output="$(printf 'sort < %s\necho shell-still-works\nexit\n' \
    "$missing_input" | "$shell_bin" 2>&1)"

if [[ "$output" != *"cannot open input file"* ||
      "$output" != *"shell-still-works"* ]]; then
    echo "FAIL: shell did not recover after an input redirection failure" >&2
    exit 1
fi

echo "All shell smoke tests passed."
