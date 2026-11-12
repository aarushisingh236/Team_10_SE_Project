# Built-in Commands and History Guide
## Team 10 — Simple Shell

This guide documents the six built-in commands handled directly by Simple Shell in [`src/builtins.c`](../src/builtins.c) and the bounded history mechanism in [`src/history.c`](../src/history.c).

---

## 1. Supported Built-in Commands

| Command | Syntax | Behavior |
|---|---|---|
| `cd` | `cd [dir]` | Changes the working directory to `dir`, or to `$HOME` if no argument is given. Reports plain-English errors if `$HOME` is unset, too many arguments are passed, or `chdir()` fails. |
| `pwd` | `pwd` | Prints the absolute current working directory using `getcwd()`. |
| `echo` | `echo [args...]` | Prints all arguments separated by single spaces followed by a trailing newline. Quoted arguments remain intact as single tokens. |
| `help` | `help` | Prints a formatted list of all six built-in commands with a one-line description for each. |
| `history` | `history` | Displays recorded commands numbered sequentially (`1`, `2`, `3`, ...) in chronological order of entry. |
| `exit` | `exit` | Exits the shell cleanly with status code `0`. |

All built-ins are identified via `is_builtin()` and executed by `execute_builtin()`, which also supports `<`, `>`, and `>>` I/O redirection while restoring `stdin` and `stdout` afterward.

---

## 2. Bounded Command History (`SH-F-011`, `SH-F-027`)

- **Default Capacity:** `SHELL_HISTORY_CAPACITY` is configured to `50` entries in [`src/history.h`](../src/history.h).
- **Raw Command Preservation:** The shell captures the original input line before tokenization so flags, spacing, and quoted arguments appear as typed.
- **Sliding-Window Eviction:** Once 50 entries are stored, `history_add()` shifts older entries forward and discards the oldest entry so the 50 most recent commands remain in chronological order.
- **Exclusions:** `NULL`, empty lines, whitespace-only lines (`SH-F-028`), and the `history` command itself are excluded from recorded history.
