#!/usr/bin/env python3
"""
Generate machine-readable and human-readable GCC coverage reports.

The script executes gcov against the instrumented project, extracts
line and branch coverage statistics, and produces JSON and Markdown
reports suitable for local use and GitHub Actions artifacts.
"""

import argparse
import json
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path


SOURCE_DIR = Path("src")
DEFAULT_OUTPUT_DIR = Path("coverage-reports")
SOURCE_FILES = sorted(SOURCE_DIR.glob("*.c"))

SUMMARY_PATTERN = re.compile(
    r"Lines executed:([\d.]+)% of (\d+)"
)

BRANCH_PATTERN = re.compile(
    r"Branches executed:([\d.]+)% of (\d+)"
)

TAKEN_PATTERN = re.compile(
    r"Taken at least once:([\d.]+)% of (\d+)"
)


def run_gcov(source_dir):
    """Run gcov and return its captured output."""

    sources = sorted(source_dir.glob("*.c"))

    if not sources:
        raise RuntimeError(
            f"No C source files found in {source_dir}"
        )

    command = [
        "gcov",
        "-b",
        "-c",
        "-o",
        str(source_dir),
        *[str(source) for source in sources],
    ]

    result = subprocess.run(
        command,
        capture_output=True,
        text=True,
        check=False,
    )

    if result.returncode != 0:
        raise RuntimeError(
            "gcov execution failed:\n" + result.stderr
        )

    return result.stdout + "\n" + result.stderr


def parse_coverage(gcov_output):
    """Extract per-file line and branch statistics."""

    reports = []
    current_file = None

    for line in gcov_output.splitlines():
        if line.startswith("File '") and line.endswith("'"):
            current_file = line[6:-1]
            continue

        if current_file is None:
            continue

        line_match = SUMMARY_PATTERN.search(line)
        branch_match = BRANCH_PATTERN.search(line)
        taken_match = TAKEN_PATTERN.search(line)

        if line_match:
            report = {
                "file": current_file,
                "lines_executed_percent": float(
                    line_match.group(1)
                ),
                "lines_instrumented": int(
                    line_match.group(2)
                ),
            }

            reports.append(report)

        if branch_match and reports:
            reports[-1]["branches_executed_percent"] = float(
                branch_match.group(1)
            )
            reports[-1]["branches_instrumented"] = int(
                branch_match.group(2)
            )

        if taken_match and reports:
            reports[-1]["branches_taken_percent"] = float(
                taken_match.group(1)
            )
            reports[-1]["branches_taken_total"] = int(
                taken_match.group(2)
            )

    if not reports:
        raise RuntimeError(
            "No coverage summaries were found. "
            "Run the instrumented test suite first."
        )

    return reports


def calculate_totals(reports):
    """Calculate weighted project-wide coverage metrics."""

    total_lines = sum(
        item["lines_instrumented"] for item in reports
    )

    executed_lines = sum(
        round(
            item["lines_instrumented"]
            * item["lines_executed_percent"] / 100
        )
        for item in reports
    )

    total_branches = sum(
        item.get("branches_instrumented", 0)
        for item in reports
    )

    executed_branches = sum(
        round(
            item.get("branches_instrumented", 0)
            * item.get("branches_executed_percent", 0) / 100
        )
        for item in reports
    )

    taken_branches = sum(
        round(
            item.get("branches_taken_total", 0)
            * item.get("branches_taken_percent", 0) / 100
        )
        for item in reports
    )

    return {
        "lines_instrumented": total_lines,
        "lines_executed_estimated": executed_lines,
        "line_coverage_percent": round(
            executed_lines / total_lines * 100, 2
        ) if total_lines else 0,
        "branches_instrumented": total_branches,
        "branches_executed_estimated": executed_branches,
        "branch_execution_percent": round(
            executed_branches / total_branches * 100, 2
        ) if total_branches else None,
        "branches_taken_estimated": taken_branches,
        "branch_taken_percent": round(
            taken_branches / total_branches * 100, 2
        ) if total_branches else None,
    }


def write_json(output_dir, reports, totals):
    """Write the complete coverage report as JSON."""

    result = {
        "generated_at": datetime.now(
            timezone.utc
        ).isoformat(),
        "tool": "GCC gcov",
        "modules": reports,
        "summary": totals,
    }

    destination = output_dir / "coverage-summary.json"

    destination.write_text(
        json.dumps(result, indent=2) + "\n",
        encoding="utf-8",
    )

    return destination


def write_markdown(output_dir, reports, totals):
    """Write a readable Markdown coverage report."""

    lines = [
        "# Automated Code Coverage Report",
        "",
        f"Generated: {datetime.now(timezone.utc).isoformat()}",
        "",
        "## Overall Coverage",
        "",
        "| Metric | Result |",
        "|---|---:|",
        (
            f"| Line coverage | "
            f"{totals['line_coverage_percent']:.2f}% |"
        ),
        (
            f"| Lines instrumented | "
            f"{totals['lines_instrumented']} |"
        ),
        (
            f"| Branch execution | "
            f"{totals['branch_execution_percent'] if totals['branch_execution_percent'] is not None else 'N/A'}% |"
        ),
        (
            f"| Branches instrumented | "
            f"{totals['branches_instrumented']} |"
        ),
        (
            f"| Branch outcomes taken | "
            f"{totals['branch_taken_percent'] if totals['branch_taken_percent'] is not None else 'N/A'}% |"
        ),
        "",
        "## Module-wise Coverage",
        "",
        "| Module | Lines | Line coverage | Branches | Branch execution |",
        "|---|---:|---:|---:|---:|",
    ]

    for report in reports:
        lines.append(
            f"| `{Path(report['file']).name}` "
            f"| {report['lines_instrumented']} "
            f"| {report['lines_executed_percent']:.2f}% "
            f"| {report.get('branches_instrumented', 0)} "
            f"| {report.get('branches_executed_percent', 0):.2f}% |"
        )

    lines.extend([
        "",
        "## Notes",
        "",
        "- Measurements are generated from the current instrumented test run.",
        "- Line coverage is weighted using instrumented line counts.",
        "- Branch execution and branch outcomes taken are separate gcov metrics.",
        "- Estimated executed counts are derived from gcov percentages.",
        "",
    ])

    destination = output_dir / "coverage-summary.md"

    destination.write_text(
        "\n".join(lines),
        encoding="utf-8",
    )

    return destination


def main():
    parser = argparse.ArgumentParser(
        description="Generate GCC gcov coverage reports."
    )

    parser.add_argument(
        "--source-dir",
        type=Path,
        default=SOURCE_DIR,
        help="Directory containing instrumented C source files.",
    )

    parser.add_argument(
        "--output-dir",
        type=Path,
        default=DEFAULT_OUTPUT_DIR,
        help="Directory for generated reports.",
    )

    args = parser.parse_args()

    try:
        args.output_dir.mkdir(
            parents=True,
            exist_ok=True,
        )

        print("Running GCC gcov...")
        output = run_gcov(args.source_dir)

        reports = parse_coverage(output)
        totals = calculate_totals(reports)

        json_path = write_json(
            args.output_dir,
            reports,
            totals,
        )

        markdown_path = write_markdown(
            args.output_dir,
            reports,
            totals,
        )

        print("\nCoverage Summary")
        print("----------------")
        print(
            f"Line coverage: "
            f"{totals['line_coverage_percent']:.2f}%"
        )
        print(
            f"Branch execution: "
            f"{totals['branch_execution_percent']}"
        )
        print(f"Modules analysed: {len(reports)}")
        print(f"JSON report: {json_path}")
        print(f"Markdown report: {markdown_path}")

    except (OSError, RuntimeError, ValueError) as error:
        print(f"Coverage report generation failed: {error}",
              file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
