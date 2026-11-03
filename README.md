# Team 10 — Simple Shell
### A Modular Command-Line Interpreter in C

A lightweight, modular command-line shell implemented in **C11** for POSIX-compliant Linux systems. The project aims to provide a simplified command-line environment supporting built-in commands, external command execution, command parsing and process management.

The application is developed following Software Engineering principles, with requirements and architecture documented in the Software Requirements Specification (SRS), Software Architecture Document (SAD), and Software Test Plan.

---

## 1. Project Overview

Simple Shell is a command-line interpreter that accepts user commands, parses the input, identifies the requested operation and executes it.

The system is designed with a modular architecture, separating command input, parsing, built-in command handling, process execution and other shell functionalities.

**Project:** Team 10 — Simple Shell  
**Language:** C11  
**Platform:** POSIX-compliant Linux  
**Compiler:** GCC  
**Build System:** GNU Make

---

## 2. Implemented Features

### Sprint 1 — Core Shell Functionality

Sprint 1 focused on establishing the shell's core functionality, including command input, parsing, built-in commands and external command execution.

The following 14 functional requirements have been implemented:

| Requirement ID | Feature | Description |
|---|---|---|
| SH-F-001 | Display Command Prompt | Displays the `shell> ` prompt and accepts user input. |
| SH-F-002 | Validate Command-Line Length | Enforces bounded input and handles oversized commands safely. |
| SH-F-003 | Tokenize Command Line | Splits command input into command names and arguments. |
| SH-F-004 | Support Quoted Arguments | Handles single-quoted and double-quoted arguments. |
| SH-F-005 | Ignore Blank Input | Ignores empty and whitespace-only commands. |
| SH-F-006 | `cd` Built-in | Changes the current working directory. |
| SH-F-007 | `pwd` Built-in | Displays the current working directory. |
| SH-F-008 | `echo` Built-in | Displays the provided arguments. |
| SH-F-009 | `help` Built-in | Displays available commands and usage information. |
| SH-F-010 | `exit` Built-in | Terminates the shell. |
| SH-F-012 | Search PATH | Locates external executables using the PATH environment variable. |
| SH-F-013 | Execute External Commands | Creates child processes using `fork()` and executes programs using `execvp()`. |
| SH-F-014 | Wait for Foreground Commands | Waits for foreground child processes to complete. |
| SH-F-015 | Command-Not-Found Errors | Reports errors when a requested command cannot be found. |

**Sprint 1 Status:** All 14 assigned functional requirements are marked Done in the GitHub Project.

---

## 3. Technology Stack

| Technology | Purpose |
|---|---|
| C11 | Core implementation |
| GCC | Compilation |
| POSIX APIs | Process creation and management |
| Linux | Target operating system |
| GNU Make | Build automation |
| Bash | Shell-based integration tests |
| Git & GitHub | Version control and collaboration |
| GitHub Actions | Continuous Integration |

---

## 4. Project Architecture

The project follows a modular architecture in which individual components are responsible for specific shell operations.

The major modules include:

- **Command Input Module (CIM):** Handles user input and input validation.
- **Command Parser (CP):** Tokenizes command lines and processes arguments.
- **Built-in Command Module (BCM):** Implements shell-specific commands such as `cd`, `pwd`, `echo`, `help` and `exit`.
- **Process Execution Module (PEM):** Handles external command execution and process management.
- **I/O Redirection Module (IRM):** Handles input and output redirection.
- **Pipe Management Module (PMM):** Manages communication between processes through pipes.
- **Environment Variable Module (EVM):** Handles environment-related operations.
- **History Module (HM):** Manages command history.
- **Signal Handling Module (SHM):** Handles shell and process signals.
- **Error Handling Module (EHM):** Provides error reporting and handling.

The modular structure allows individual functionalities to be developed, tested and maintained independently.

---

## 5. Project Structure

```text
Team_10_SE_Project/
│
├── .github/
│   └── workflows/
│       └── ci.yml
│
├── src/
│   ├── main.c
│   ├── command_input.c
│   ├── command_input.h
│   ├── parser.c
│   ├── parser.h
│   ├── builtins.c
│   ├── builtins.h
│   ├── process_exec.c
│   ├── process_exec.h
│   ├── redirection.c
│   ├── redirection.h
│   ├── pipes.c
│   ├── pipes.h
│   ├── environment.c
│   ├── environment.h
│   ├── history.c
│   ├── history.h
│   ├── signals.c
│   ├── signals.h
│   ├── errors.c
│   └── errors.h
│
├── tests/
│   ├── test_parser.c
│   └── test_shell.sh
│
├── Makefile
└── README.md
```

---

## 6. Installation and Execution

### Prerequisites

Ensure that the following tools are installed:

- GCC compiler
- GNU Make
- POSIX-compliant Linux environment

### Clone the Repository

```bash
git clone https://github.com/aarushisingh236/Team_10_SE_Project.git
cd Team_10_SE_Project
```

### Compile the Project

```bash
make
```

### Run the Shell

```bash
./simple-shell
```

### Example Usage

```text
shell> pwd
/home/user/Team_10_SE_Project

shell> echo Hello World
Hello World

shell> help
[Displays available commands]

shell> ls
[Displays directory contents]

shell> exit
```

---

## 7. Testing

Testing is performed using automated unit tests and shell-based integration tests.

Run the test suite using:

```bash
make test
```

The test suite covers functionality such as:

- Command-line input and validation.
- Tokenization and argument parsing.
- Quoted arguments.
- Blank input handling.
- Built-in command execution.
- External command execution.
- Process management.
- Error handling.

GitHub Actions is configured to automate building and testing whenever changes are pushed or pull requests are submitted.

---

## 8. Development Workflow

The project follows an issue-driven GitHub workflow.

1. Requirements are organized into GitHub Issues.
2. Issues are assigned to individual team members.
3. Each feature is developed in a dedicated Git branch.
4. Changes are committed and pushed to GitHub.
5. Pull requests are raised against the main branch.
6. Code is reviewed and automated tests are executed.
7. Approved pull requests are merged.
8. Corresponding GitHub Issues and Project statuses are updated.

The project uses sprint-based development to organize and track implementation progress.

---

## 9. Project Documentation

The following documents guide the development process:

- **Software Requirements Specification (SRS):** Defines functional and non-functional requirements.
- **Software Architecture Document (SAD):** Describes the system architecture and module responsibilities.
- **Software Test Plan:** Defines testing strategies, test cases and acceptance criteria.

These documents serve as the reference for implementation and validation.

---

## 10. Authors / Team Members

**Team 10 — Simple Shell**

| Name | SRN |
|---|---|
| Aarushi Singh | PES2UG24CS015 |
| Akshaya M | PES2UG24CS050 |
| Akshay Anil Kumar | PES2UG24CS049 |
| Adarsh Shivanand Madli | PES2UG24CS028 |

Developed as part of the Software Engineering course project.
