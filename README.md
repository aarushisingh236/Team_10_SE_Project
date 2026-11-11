# Team 10 — Simple Shell
### A Modular Command-Line Interpreter in C

A lightweight command-line shell developed in **C11** for POSIX-compliant Linux systems. It provides a command-line environment that allows users to execute built-in commands, run external programs and interact with the operating system.

The project follows a modular software architecture, with separate components for command input, parsing, execution, process management and other shell operations.

---

## 1. Project Overview

Simple Shell is a command-line interpreter that accepts commands from users, interprets the input and executes the requested operations.

It provides essential shell functionalities, including built-in command handling, external command execution, process management and command-line operations.

The project was developed using an iterative, sprint-based approach, following Software Engineering principles and practices.

---

## 2. Technology Stack

| Technology | Purpose |
|---|---|
| C11 | Core programming language |
| GCC | Compilation |
| POSIX APIs | Operating system and process interaction |
| Linux | Target operating system |
| GNU Make | Build automation |
| Bash | Integration testing |
| Git & GitHub | Version control and collaboration |
| GitHub Actions | Continuous Integration (CI) |

---

## 3. System Architecture

The shell follows a modular architecture, dividing its functionality into the following major components:

- **Command Input and Parsing:** Accepts user input, validates commands and processes arguments.
- **Command Handling:** Identifies and executes built-in shell commands.
- **Process Execution:** Handles the execution of external programs and manages child processes.
- **I/O Management:** Handles input/output redirection and communication between processes through pipes.
- **Environment and History Management:** Manages environment-related operations and command history.
- **Signal and Error Handling:** Handles process signals and provides appropriate error reporting.

This separation of responsibilities makes the system easier to maintain, test and extend.

---

## 4. Project Structure

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

## 5. Installation and Execution

### Prerequisites

- GCC compiler
- GNU Make
- POSIX-compliant Linux environment

### Clone the Repository

```bash
git clone https://github.com/aarushisingh236/Team_10_SE_Project.git
cd Team_10_SE_Project
```

### Build the Project

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

shell> ls
[Displays directory contents]

shell> help
[Displays available commands]

shell> exit
```

---

## 6. Testing

The project uses automated unit and integration tests to verify functionality and ensure reliability.

Run the test suite using:

```bash
make test
```

GitHub Actions is used for Continuous Integration (CI), automating the build and test process whenever changes are pushed or pull requests are submitted.

---

## 7. Development Workflow

The project follows an issue-driven, sprint-based development approach using GitHub.

- Requirements and tasks are organized as GitHub Issues.
- Tasks are assigned among team members.
- Features are developed using separate Git branches.
- Pull Requests are raised for code review and integration.
- Automated tests are executed before merging changes.
- Project progress is tracked through GitHub Projects.

---

## 8. Project Documentation

The development process is supported by the following documents:

- **Software Requirements Specification (SRS):** Defines the functional and non-functional requirements.
- **Software Architecture Document (SAD):** Describes the system architecture and design.
- **Software Test Plan:** Defines the testing approach, test cases and expected results.

---

## 9. Team Members

**Team 10 — Simple Shell**

| Name | SRN |
|---|---|
| Aarushi Singh | PES2UG24CS015 |
| Akshaya M | PES2UG24CS050 |
| Akshay Anil Kumar | PES2UG24CS049 |
| Adarsh Shivanand Madli | PES2UG24CS028 |

Developed as part of the Software Engineering course project.

---

## Repository

[Team 10 — Simple Shell on GitHub](https://github.com/aarushisingh236/Team_10_SE_Project)
