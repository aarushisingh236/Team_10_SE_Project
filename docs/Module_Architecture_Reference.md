# Module Architecture Reference
## Team 10 — Simple Shell (`SH-NF-004`)

This document summarizes the 10-module software architecture of Simple Shell and the public interfaces exposed by each header in [`src/`](../src).

---

## 1. Orchestration Flow

[`src/main.c`](../src/main.c) serves as the top-level Read-Eval-Print Loop (REPL) coordinator:

1. Installs `SIGCHLD` and `SIGINT` handlers via `setup_signal_handlers()`.
2. Initializes bounded command history via `history_init()`.
3. Reads bounded user input via `read_command_line()`.
4. Parses tokens and quote metadata via `parse_command()`.
5. Expands environment variables (`$VAR`) via `environment_expand_variables()`.
6. Records non-blank commands (excluding `history` itself) via `history_add()`.
7. Routes pipelines (`|`) to `execute_pipeline()`, built-ins to `execute_builtin()`, and external commands to `resolve_command()` and `execute_external()`.

---

## 2. Module Directory

| Module | Files | Responsibility | Key Public Interfaces |
|---|---|---|---|
| Command Input (CIM) | `command_input.h` / `.c` | Prompt display and bounded line reading (`SHELL_MAX_LINE = 1024`) | `read_command_line()` |
| Command Parser (CP) | `parser.h` / `.c` | Whitespace tokenization and single/double quote handling | `parse_command()` |
| Built-in Commands (BCM) | `builtins.h` / `.c` | Built-in detection and execution (`cd`, `pwd`, `echo`, `help`, `history`, `exit`) | `is_builtin()`, `handle_builtin()` |
| Process Execution (PEM) | `process_exec.h` / `.c` | `PATH` lookup, executable checks, built-in I/O redirection, `fork()`/`execvp()` | `is_executable_file()`, `resolve_command()`, `execute_builtin()`, `execute_external()` |
| I/O Redirection (IRM) | `redirection.h` / `.c` | Parsing `<`, `>`, `>>` tokens and opening target files | `parse_input_redirection()`, `parse_output_redirection()`, `open_output_file()` |
| Pipe Management (PMM) | `pipes.h` / `.c` | Unquoted `|` detection and multi-stage pipeline execution | `has_pipe_operator()`, `execute_pipeline()` |
| Environment Variables (EVM) | `environment.h` / `.c` | `PATH` retrieval and safe `$VAR` expansion | `environment_get_path()`, `environment_expand_variables()`, `environment_get_expansion_error()`, `environment_get_failed_variable()` |
| History (HM) | `history.h` / `.c` | Bounded chronological command history (`SHELL_HISTORY_CAPACITY = 50`) | `history_init()`, `history_add()`, `history_get_count()`, `history_get_entries()`, `history_print()` |
| Signal Handling (SHM) | `signals.h` / `.c` | Reaping background children (`SIGCHLD`) and interactive `Ctrl+C` (`SIGINT`) | `setup_signal_handlers()`, `handle_sigchld()`, `handle_sigint()` |
| Error Handling (EHM) | `errors.h` / `.c` | Plain-English diagnostic formatting to `stderr` | `print_error_command_not_found()`, `print_error_command_not_executable()`, `print_error_input_file()`, `print_error_output_file()`, `print_error_directory()`, `print_error_system()`, `print_error_syntax()` |
