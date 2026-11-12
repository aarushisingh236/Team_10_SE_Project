#!/usr/bin/env python3
"""Validate project documentation and internal Markdown links."""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

REQUIRED_SECTIONS = [
    "Project Overview",
    "Technology Stack",
    "System Architecture",
    "Project Structure",
    "Installation and Execution",
    "Testing",
    "Continuous Integration",
    "Development Workflow",
    "Project Documentation",
    "Team Members",
]

REQUIRED_DOCS = [
    "README.md",
    "docs/Code_Coverage_Report.md",
    "docs/Branch_Coverage_Report.md",
    "docs/System_Validation_Report.md",
    "docs/Unit_Testing_Report.md",
]

def main():
    failures = []
    readme = ROOT / "README.md"
    content = readme.read_text(encoding="utf-8")

    print("Checking README sections...")
    for section in REQUIRED_SECTIONS:
        if re.search(r"^#{1,6}\s+.*" + re.escape(section), content, re.M | re.I):
            print(f"[PASS] {section}")
        else:
            print(f"[FAIL] Missing section: {section}")
            failures.append(section)

    print("\nChecking required documentation files...")
    for filename in REQUIRED_DOCS:
        if (ROOT / filename).is_file():
            print(f"[PASS] {filename}")
        else:
            print(f"[FAIL] {filename}")
            failures.append(filename)

    print("\nChecking internal Markdown links...")
    for markdown in ROOT.rglob("*.md"):
        if ".git" in markdown.parts:
            continue

        text = markdown.read_text(encoding="utf-8")
        for target in re.findall(r"(?<!!)\[[^\]]+\]\(([^)]+)\)", text):
            target = target.strip().split("#", 1)[0]
            if not target or target.startswith(("http://", "https://", "mailto:")):
                continue

            resolved = (markdown.parent / target).resolve()
            if resolved.exists():
                print(f"[PASS] {markdown.relative_to(ROOT)} -> {target}")
            else:
                print(f"[FAIL] Broken link: {markdown.relative_to(ROOT)} -> {target}")
                failures.append(target)

    print(f"\nDocumentation checks failed: {len(failures)}")
    if failures:
        print("Documentation validation failed.")
        return 1

    print("Documentation validation passed.")
    return 0

if __name__ == "__main__":
    sys.exit(main())
