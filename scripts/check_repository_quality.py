#!/usr/bin/env python3
"""Check tracked repository files for common quality issues."""

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TEXT_SUFFIXES = {".c", ".h", ".py", ".sh", ".md", ".yml", ".yaml", ".txt"}
CONFLICT_MARKERS = (
    "<<<<<<< ",
    "=======",
    ">>>>>>> ",
)

def tracked_files():
    result = subprocess.run(
        ["git", "ls-files"],
        cwd=ROOT,
        capture_output=True,
        text=True,
        check=True,
    )
    return [ROOT / name for name in result.stdout.splitlines()]

def main():
    failures = []
    files = tracked_files()

    print(f"Checking {len(files)} tracked files...")

    for path in files:
        if not path.is_file() or path.suffix not in TEXT_SUFFIXES:
            continue

        try:
            content = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            print(f"[FAIL] Invalid UTF-8: {path.relative_to(ROOT)}")
            failures.append(str(path))
            continue

        relative = path.relative_to(ROOT)

        if content and not content.endswith("\n"):
            print(f"[FAIL] Missing final newline: {relative}")
            failures.append(str(relative))

        for number, line in enumerate(content.splitlines(), 1):
            if line.rstrip(" \t") != line:
                print(f"[FAIL] Trailing whitespace: {relative}:{number}")
                failures.append(f"{relative}:{number}")

            if line.startswith(CONFLICT_MARKERS):
                print(f"[FAIL] Possible merge conflict marker: {relative}:{number}")
                failures.append(f"{relative}:{number}")

    print(f"\nRepository quality issues: {len(failures)}")
    if failures:
        return 1

    print("Repository quality checks passed.")
    return 0

if __name__ == "__main__":
    sys.exit(main())
