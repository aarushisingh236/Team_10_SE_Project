# Unit Testing Report
## Team 10 — Simple Shell

### Team Members
- Aarushi Singh — PES2UG24CS015
- Akshaya M — PES2UG24CS050
- Akshay Anil Kumar — PES2UG24CS049
- Adarsh Shivanand Madli — PES2UG24CS028

---

### 1. Objective
To validate individual shell modules and their core functionality using automated unit tests.

### 2. Test Environment
- Operating System: Ubuntu 24.04 (WSL)
- Compiler: GCC 13.3.0
- Language: C11
- Build system: GNU Make
- Compiler flags: `-Wall -Wextra -Wpedantic -Werror`
- Coverage instrumentation: GCC `--coverage`, `-O0`

### 3. Unit Test Execution

| Test file | Result |
|---|---|
| tests/test_parser.c | Passed |
| tests/test_history.c | Passed |
| tests/test_environment.c | Passed |

### 4. Test Coverage Areas
- Command parsing and argument handling.
- Command history operations.
- Environment-related functionality and security checks.

### 5. Execution Result
All three compiled unit test executables completed successfully.

- Parser tests: Passed.
- History tests: Passed.
- Environment tests: Passed.

### 6. Conclusion
All executed unit test suites passed in the recorded coverage-instrumented run.
