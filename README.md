# Team 10 — Simple Shell (Command Line Interpreter)

A modular command-line shell implemented in C11 for POSIX-compliant Linux, following the team's SRS v1.0, SAD v1.0, and Software Test Plan.

## Current implementation status

Initial Sprint 1 scaffold. Implemented:
- `SH-F-001`: displays `shell> ` and waits for input.
- `SH-F-002`: bounded command-line input; rejects oversized lines without crashing.
- `SH-F-003`: tokenizes whitespace-separated command names and arguments.
- `SH-F-004`: supports single- and double-quoted arguments.
- `SH-F-010`: `exit` terminates the shell with status 0.
- Blank/whitespace-only lines are ignored as a small baseline behaviour.

External command execution and the remaining built-ins are not implemented in this scaffold. They should be added through the team's assigned issues and documented modules; do not mark those stories complete until implemented and tested.

## Requirements and architecture

The project follows the submitted SRS and SAD:
- C11 + GCC on POSIX-compliant Linux.
- Command input and parsing are separate modules.
- No raw command input is passed to `system()` or an equivalent shell evaluator.
- Automated tests run with `make test`.
- GitHub Actions builds and tests every push and pull request.

## Project structure

```text
.
├── .github/workflows/ci.yml
├── src/
│   ├── main.c
│   ├── command_input.c
│   ├── command_input.h
│   ├── parser.c
│   ├── parser.h
│   ├── builtins.c
│   └── builtins.h
├── tests/
│   ├── test_parser.c
│   └── test_shell.sh
├── Makefile
└── README.md
```

## Build and run

Requires GCC, Make, and a POSIX-compliant Linux environment (verified on Ubuntu 22.04 LTS and Ubuntu 24.04 LTS via CI for TC-PORT-01).

```bash
make
./simple-shell
```

Try:

```text
shell> exit
```

Oversized input is rejected with an error message. At this stage, commands other than `exit` are reported as not implemented yet.

## Run tests

```bash
make test
```

The tests cover parser examples, quoted arguments, blank input, malformed quotes, prompt display, clean exit, and oversized-input rejection.

## Team workflow

1. Create a feature branch for an assigned issue.
2. Implement the feature and add tests.
3. Run `make test`.
4. Push the branch and open a pull request against `main`.
5. Review the PR and merge only after CI passes.
6. Update the corresponding GitHub issue/project status.

## Documentation

The submitted SRS, SAD, and Software Test Plan PDFs remain the source of truth for the full requirement set and acceptance criteria.
