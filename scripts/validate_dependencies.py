#!/usr/bin/env python3
"""Validate build tools and required project files."""

import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

REQUIRED_TOOLS = {
    "gcc": ["--version"],
    "make": ["--version"],
    "python3": ["--version"],
}

REQUIRED_FILES = [
    "Makefile",
    "README.md",
    "src/main.c",
    "src/parser.c",
    "src/builtins.c",
    "tests/test_shell.sh",
    "tests/test_parser.c",
    "scripts/generate_coverage_report.py",
    "scripts/package_release.py",
]

def main():
    failures = []

    print("Checking required build tools...")
    for tool, arguments in REQUIRED_TOOLS.items():
        executable = shutil.which(tool)

        if not executable:
            print(f"[FAIL] {tool} not found")
            failures.append(tool)
            continue

        result = subprocess.run(
            [executable, *arguments],
            capture_output=True,
            text=True,
            check=False,
        )

        if result.returncode == 0:
            version = (result.stdout or result.stderr).splitlines()
            print(f"[PASS] {tool}: {version[0] if version else 'available'}")
        else:
            print(f"[FAIL] {tool} could not execute")
            failures.append(tool)

    print("\nChecking required project files...")
    for filename in REQUIRED_FILES:
        if (ROOT / filename).is_file():
            print(f"[PASS] {filename}")
        else:
            print(f"[FAIL] {filename}")
            failures.append(filename)

    print("\nDependency validation summary")
    print(f"Checks failed: {len(failures)}")

    if failures:
        print("Validation failed.")
        return 1

    print("All dependency and project structure checks passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
