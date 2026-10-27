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

# Test 2a: Echo prints multiple arguments separated by single spaces.
output="$(printf 'echo hello world\nexit\n' | "$shell_bin" 2>&1)"
if [[ "$output" != *"hello world"* ]]; then
    echo "FAIL: echo did not print multiple arguments correctly" >&2
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

# Test 5: Oversized input is rejected and the shell recovers.
long_line="$(printf '%1100s' '' | tr ' ' 'x')"
output="$(printf '%s\necho RECOVERY_OK\nexit\n' "$long_line" |
    "$shell_bin" 2>&1)"

if [[ "$output" != *"command line is too long"* ]]; then
    echo "FAIL: oversized input was not rejected" >&2
    exit 1
fi

if [[ "$output" != *"RECOVERY_OK"* ]]; then
    echo "FAIL: shell did not execute a valid command after oversized input" >&2
    exit 1
fi

# Test 6: A command at the maximum input length is accepted.
boundary_line="echo $(printf '%1018s' '' | tr ' ' 'x')"
if [[ ${#boundary_line} -ne 1023 ]]; then
    echo "FAIL: could not construct the input boundary test" >&2
    exit 1
fi

output="$(printf '%s\nexit\n' "$boundary_line" | "$shell_bin" 2>&1)"

if [[ "$output" == *"command line is too long"* ]]; then
    echo "FAIL: maximum-length input was incorrectly rejected" >&2
    exit 1
fi

# Test 7: Input one character beyond the limit is rejected.
over_limit="$(printf '%1024s' '' | tr ' ' 'x')"
output="$(printf '%s\necho BOUNDARY_RECOVERY_OK\nexit\n' \
    "$over_limit" | "$shell_bin" 2>&1)"

if [[ "$output" != *"command line is too long"* ]]; then
    echo "FAIL: input beyond the limit was not rejected" >&2
    exit 1
fi

if [[ "$output" != *"BOUNDARY_RECOVERY_OK"* ]]; then
    echo "FAIL: shell did not recover after boundary rejection" >&2
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

# Test 20: Output redirection respects file write permissions.
protected_output="$test_dir/protected_output.txt"
printf 'original content\n' > "$protected_output"
chmod 444 "$protected_output"

if [[ ! -w "$protected_output" ]]; then
    output="$(printf 'echo overwrite > %s\necho shell-still-works\nexit\n' \
        "$protected_output" | "$shell_bin" 2>&1)"

    if [[ "$output" != *"cannot open output file"* ]]; then
        echo "FAIL: write-protected output file was not rejected" >&2
        chmod 600 "$protected_output"
        exit 1
    fi

    if [[ "$output" != *"Permission denied"* ]]; then
        echo "FAIL: permission error was not reported" >&2
        chmod 600 "$protected_output"
        exit 1
    fi

    if [[ "$(cat "$protected_output")" != "original content" ]]; then
        echo "FAIL: write-protected file contents were modified" >&2
        chmod 600 "$protected_output"
        exit 1
    fi

    if [[ "$output" != *"shell-still-works"* ]]; then
        echo "FAIL: shell did not recover after a permission error" >&2
        chmod 600 "$protected_output"
        exit 1
    fi
else
    echo "SKIP: write-permission test (filesystem does not enforce permissions)"
fi

chmod 600 "$protected_output"

# Test 20: Shell remains usable after an input redirection failure.
output="$(printf 'sort < %s\necho shell-still-works\nexit\n' \
    "$missing_input" | "$shell_bin" 2>&1)"

if [[ "$output" != *"cannot open input file"* ||
      "$output" != *"shell-still-works"* ]]; then
    echo "FAIL: shell did not recover after an input redirection failure" >&2
    exit 1
fi

# Test 21: A leading pipe operator is rejected.
output="$(printf '| echo hello\nexit\n' | "$shell_bin" 2>&1)"

if [[ "$output" != *"malformed command input"* ]]; then
    echo "FAIL: leading pipe operator was not rejected" >&2
    exit 1
fi

# Test 22: A trailing pipe operator is rejected.
output="$(printf 'echo hello |\nexit\n' | "$shell_bin" 2>&1)"

if [[ "$output" != *"malformed command input"* ]]; then
    echo "FAIL: trailing pipe operator was not rejected" >&2
    exit 1
fi

# Test 23: Repeated pipe operators are rejected.
output="$(printf 'echo hello || echo world\nexit\n' |
    "$shell_bin" 2>&1)"

if [[ "$output" != *"malformed command input"* ]]; then
    echo "FAIL: repeated pipe operators were not rejected" >&2
    exit 1
fi

# Test 24: Shell remains usable after malformed pipe input.
output="$(printf 'echo hello |\necho shell-still-works\nexit\n' |
    "$shell_bin" 2>&1)"

if [[ "$output" != *"malformed command input"* ||
      "$output" != *"shell-still-works"* ]]; then
    echo "FAIL: shell did not recover after malformed pipe input" >&2
    exit 1
fi

# Test 25: Malformed pipe syntax does not create an output file.
malformed_output="$test_dir/malformed_output.txt"
rm -f "$malformed_output"

output="$(printf 'echo hello | > %s\nexit\n' \
    "$malformed_output" | "$shell_bin" 2>&1)"

if [[ "$output" != *"malformed command input"* ||
      -e "$malformed_output" ]]; then
    echo "FAIL: malformed pipe syntax created an output file" >&2
    exit 1
fi

# Test 26: Malformed pipe syntax does not truncate an existing file.
printf 'preserve this content\n' > "$malformed_output"

output="$(printf 'echo hello | > %s\nexit\n' \
    "$malformed_output" | "$shell_bin" 2>&1)"

if [[ "$output" != *"malformed command input"* ||
      "$(cat "$malformed_output")" != "preserve this content" ]]; then
    echo "FAIL: malformed pipe syntax modified an output file" >&2
    exit 1
fi

# Test 27: Shell remains usable after malformed redirection and pipe syntax.
output="$(printf 'echo hello | > %s\necho shell-still-works\nexit\n' \
    "$malformed_output" | "$shell_bin" 2>&1)"

if [[ "$output" != *"malformed command input"* ||
      "$output" != *"shell-still-works"* ]]; then
    echo "FAIL: shell did not recover after malformed pipe/redirection syntax" >&2
    exit 1
fi

echo "All shell smoke tests passed."
