#!/usr/bin/env python3
"""
Package Simple Shell into a versioned release archive.

Creates a compressed tar.gz archive and its SHA-256 checksum.
"""

import argparse
import hashlib
import re
import sys
import tarfile
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parent.parent

RELEASE_CONTENTS = [
    "README.md",
    "Makefile",
    "src",
    "tests",
    "docs",
    "scripts",
    "TEAM-10_SAD.pdf",
    "Team-10_Simple_Shell_SRS.pdf",
    "Team-10_Test_Plan.pdf",
]


def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Create a versioned Simple Shell release archive."
    )
    parser.add_argument(
        "--version",
        default="1.0.0",
        help="Release version (default: 1.0.0)",
    )
    parser.add_argument(
        "--output-dir",
        default="dist",
        help="Directory for release files (default: dist)",
    )
    return parser.parse_args()


def validate_version(version):
    if not re.fullmatch(r"\d+\.\d+\.\d+(?:-[a-zA-Z0-9.-]+)?", version):
        raise ValueError(
            "Invalid version. Expected format such as 1.0.0 or 1.0.0-beta.1"
        )


def add_directory(archive, directory):
    for path in sorted(directory.rglob("*")):
        if not path.is_file():
            continue

        if any(part in {".git", "__pycache__"} for part in path.parts):
            continue

        if path.suffix in {
            ".o", ".gcda", ".gcno", ".gcov", ".pyc"
        }:
            continue

        if path.name in {"simple-shell"}:
            continue

        archive.add(
            path,
            arcname=Path(archive.root_dir_name) / path.relative_to(PROJECT_ROOT),
            recursive=False,
        )


def create_release(version, output_dir):
    validate_version(version)

    output_dir.mkdir(parents=True, exist_ok=True)

    archive_name = f"simple-shell-v{version}.tar.gz"
    archive_path = output_dir / archive_name
    checksum_path = output_dir / f"{archive_name}.sha256"

    missing = [
        item for item in RELEASE_CONTENTS
        if not (PROJECT_ROOT / item).exists()
    ]

    if missing:
        raise FileNotFoundError(
            f"Required release content is missing: {', '.join(missing)}"
        )

    root_name = f"simple-shell-v{version}"

    with tarfile.open(archive_path, "w:gz") as archive:
        archive.root_dir_name = root_name

        for item in RELEASE_CONTENTS:
            path = PROJECT_ROOT / item

            if path.is_dir():
                add_directory(archive, path)
            else:
                archive.add(
                    path,
                    arcname=Path(root_name) / item,
                    recursive=False,
                )

    digest = hashlib.sha256(archive_path.read_bytes()).hexdigest()
    checksum_path.write_text(
        f"{digest}  {archive_name}\n",
        encoding="utf-8",
    )

    print(f"Release archive: {archive_path}")
    print(f"SHA-256 checksum: {checksum_path}")
    print(f"Archive size: {archive_path.stat().st_size:,} bytes")
    print(f"SHA-256: {digest}")


def main():
    args = parse_arguments()

    try:
        create_release(
            args.version,
            Path(args.output_dir).resolve(),
        )
    except (ValueError, FileNotFoundError, OSError, tarfile.TarError) as error:
        print(f"Release packaging failed: {error}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
