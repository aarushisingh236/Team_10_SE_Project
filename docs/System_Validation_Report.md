# System Validation Report
## Team 10 — Simple Shell

### Team Members
- Aarushi Singh — PES2UG24CS015
- Akshaya M — PES2UG24CS050
- Akshay Anil Kumar — PES2UG24CS049
- Adarsh Shivanand Madli — PES2UG24CS028

---

### 1. Objective
To validate the integrated shell application against its functional, reliability, performance and security expectations.

### 2. Test Environment
- Operating System: Ubuntu 24.04 (WSL)
- Compiler: GCC 13.3.0
- Build tool: GNU Make
- Test command: `make test`

### 3. System and Integration Test Results

| Validation area | Result |
|---|---|
| Shell smoke and integration tests | Passed |
| Prompt response performance | Passed |
| Environment disclosure integration test | Passed |
| Repeated command execution | Passed |
| Recovery after negative command inputs | Passed |
| Pipeline execution | Passed |
| Background command responsiveness | Passed |
| Background child completion and reaping | Passed |
| Ctrl+C foreground process handling | Passed |
| Background PID and parent PID verification | Passed |

### 4. Performance Results

- Commands tested: 20
- Maximum response time: 0.000229 seconds
- Average response time: 0.000125 seconds
- Acceptance threshold: 1 second
- Result: All 20 prompt responses were within the threshold.

### 5. Reliability Results

All 6 reliability checks passed:
1. Repeated command execution.
2. Recovery after negative command inputs.
3. Successful pipeline execution.
4. Background command responsiveness.
5. Background child completion and reaping.
6. Ctrl+C interrupts foreground command while preserving the shell.

### 6. Process Validation

The background PID test verified that the reported PID identified the sleep child process and that the child's parent PID matched the shell PID.

### 7. Conclusion

All system validation tests executed through the coverage-instrumented make test run passed.
