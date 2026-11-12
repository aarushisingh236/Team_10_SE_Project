# POSIX Portability and CI Environment Guide
## Team 10 — Simple Shell (`SH-NF-005`)

This note summarizes how Simple Shell maintains POSIX portability across Linux distributions and compilers (`TC-PORT-01`).

---

## 1. Standards and Compiler Flags

- **Language Standard:** C11 (`-std=c11`) with `_POSIX_C_SOURCE 200809L` for POSIX APIs (`fork`, `execvp`, `waitpid`, `pipe`, `sigaction`, `getcwd`, `chdir`).
- **Strict Warning Policy:** [`Makefile`](../Makefile) compiles all 11 source files with `-Wall -Wextra -Wpedantic -Werror` to catch non-portable constructs early.
- **Line Ending Normalization:** [`.gitattributes`](../.gitattributes) enforces Unix `LF` endings on `.c`, `.h`, `.sh`, and `Makefile` files so cross-platform edits on Windows do not break Linux builds or Bash test scripts.

---

## 2. Multi-Environment CI Verification

[`.github/workflows/ci.yml`](../.github/workflows/ci.yml) runs `make` and `make test` across a 4-way matrix:

| Operating System | Compilers Tested | Verification |
|---|---|---|
| Ubuntu 22.04 LTS | `gcc`, `clang` | Logs OS/compiler info, compiles cleanly, and runs unit, Bash, and Python test suites |
| Ubuntu 24.04 LTS | `gcc`, `clang` | Logs OS/compiler info, compiles cleanly, and runs unit, Bash, and Python test suites |
