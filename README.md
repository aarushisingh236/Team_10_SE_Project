# Team 10 — Simple Shell
### A Modular Command-Line Interpreter in C

A lightweight command-line shell developed in **C11** for POSIX-compliant Linux systems. It provides a command-line environment that allows users to execute built-in commands, run external programs and interact with the operating system.

The project follows a modular software architecture, with separate components for command input, parsing, execution, process management and other shell operations.

---

## 1. Project Overview

Simple Shell is a command-line interpreter that accepts commands from users, interprets the input and executes the requested operations.

It provides essential shell functionalities, including:

- Built-in command handling.
- External command execution using `fork()` and `execvp()`.
- Command-line parsing and quoted argument support.
- Foreground and background process management.
- Input, output and append redirection.
- Single and multi-command pipelines.
- Environment variable expansion.
- Command history management.
- Signal handling and error reporting.
- Input validation and security mechanisms.

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
| Python | Automated testing |
| Git & GitHub | Version control and collaboration |
| GitHub Actions | Continuous Integration (CI) |

---

## 3. System Architecture

The shell follows a modular architecture, dividing its functionality into the following major components:

- **Command Input and Parsing:** Accepts user input, validates commands and processes arguments.
- **Command Handling:** Identifies and executes built-in shell commands.
- **Process Execution:** Handles the execution of external programs and manages child processes.
- **I/O Management:** Handles input/output redirection and communication between processes through pipes.
- **Environment and History Management:** Handles environment variable expansion and command history.
- **Signal and Error Handling:** Handles process signals and provides appropriate error reporting.

This separation of responsibilities makes the system easier to maintain, test and extend.

---

## 4. Project Structure

```text
Team_10_SE_Project/
│
├── .github/
│   └── workflows/
│       ├── ci.yml
│       ├── static-analysis.yml
│       ├── security.yml
│       ├── coverage.yml
│       ├── release.yml
│       ├── dependency-validation.yml
│       ├── documentation-quality.yml
│       └── repository-quality.yml
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
│   ├── test_history.c
│   ├── test_environment.c
│   ├── test_shell.sh
│   ├── test_prompt_response.py
│   ├── test_environment_security.py
│   ├── test_reliability.py
│   └── test_background_pid.py
│
├── docs/
│   ├── Branch_Coverage_Report.md
│   ├── Builtins_and_History_Guide.md
│   ├── Code_Coverage_Report.md
│   ├── Module_Architecture_Reference.md
│   ├── POSIX_Portability_Guide.md
│   ├── System_Validation_Report.md
│   ├── Unit_Testing_Report.md
│   └── gcov_raw_report.txt
│
├── scripts/
│   ├── check_repository_quality.py
│   ├── generate_coverage_report.py
│   ├── package_release.py
│   ├── validate_dependencies.py
│   └── validate_documentation.py
│
├── videos/
│   ├── Sprint_1_Demo.mp4
│   └── Sprint_2_Demo.mp4
│
├── Makefile
├── README.md
├── TEAM-10_SAD.pdf
├── Team-10_SRS.pdf
└── Team-10_STP.pdf
```

---

## 5. Installation and Execution

### Prerequisites

- GCC compiler
- GNU Make
- Python 3
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

shell> cd /tmp

shell> ls
[Displays directory contents]

shell> help
[Displays available commands]

shell> sleep 5 &
[Starts a background process]

shell> exit
```

### Clean Build Files

```bash
make clean
```

---

## 6. Testing

The project uses automated unit, integration, performance, security and reliability tests to verify functionality and ensure system stability.

The test suite covers:

- Parser and command input validation.
- Command history management.
- Environment variable handling.
- Shell command execution.
- Prompt response time.
- Environment information disclosure prevention.
- Reliability and process management.
- Background process PID reporting and verification.

Run the complete test suite using:

```bash
make test
```

### Continuous Integration

GitHub Actions is used for Continuous Integration (CI), automating the build and test process whenever changes are pushed or pull requests are submitted.

The CI workflow:

- Compiles the project using GCC.
- Executes the automated test suite.
- Validates the implementation on Ubuntu 22.04.
- Validates the implementation on Ubuntu 24.04.

This helps ensure build consistency and reliability across supported Linux environments.

---

## 7. Development Workflow

The project follows an issue-driven, sprint-based development approach using GitHub.

- Requirements and tasks are organized as GitHub Issues.
- Tasks are assigned among team members.
- Story Points are assigned to backlog items.
- Development activities are organized into Sprint 1 and Sprint 2.
- Features are developed using separate Git branches.
- Pull Requests are raised by assigned team members.
- Pull Requests are reviewed and merged by the repository owner.
- Automated tests are executed through GitHub Actions.
- Project progress is tracked through GitHub Projects.

The development process follows an iterative approach, with functionality implemented, tested and integrated across both sprints.

---

## 8. Project Documentation

The development process is supported by the following documents:

- **Software Requirements Specification (SRS):** Defines the functional and non-functional requirements of the system.
- **Software Architecture and Design (SAD) Specification:** Describes the system architecture, components, design decisions and interfaces.
- **Software Test Plan:** Defines the testing approach, test cases, validation procedures and expected results.

These documents establish the requirements, architectural design and testing strategy followed throughout the project.

---

## 9. Sprint Demonstration Videos

The following videos demonstrate the working Simple Shell application developed across both sprints.

| Sprint | Duration | Demonstration |
|---|---|---|
| Sprint 1 | 1 minute | Initial shell implementation and core command execution |
| Sprint 2 | 2 minutes | Extended functionality, process management, redirection, pipes, environment handling and security |

---

## 10. Team Members

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
