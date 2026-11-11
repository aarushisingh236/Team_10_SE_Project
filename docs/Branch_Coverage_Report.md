# Branch Coverage Report
## Team 10 — Simple Shell

### 1. Objective
To measure conditional branch execution across the shell's source modules using GCC gcov.

### 2. Coverage Configuration
- Compiler: GCC 13.3.0
- Instrumentation: --coverage
- Reporting tool: gcov
- Command: gcov -b -c -o src src/*.c

### 3. Overall Branch Metrics

| Metric | Result |
|---|---:|
| Branches analysed | 546 |
| Branches executed at least once | Approximately 442 |
| Branches executed percentage | 80.95% |
| Branch outcomes taken at least once | Approximately 313 |
| Taken-at-least-once percentage | 57.33% |

The two percentages describe different gcov metrics. Branches executed measures whether branch instrumentation was exercised; taken at least once measures whether the branch outcome was observed.

### 4. Module-wise Branch Results

| Module | Branches | Branches executed | Taken at least once |
|---|---:|---:|---:|
| builtins.c | 50 | 88.00% | 64.00% |
| command_input.c | 46 | 78.26% | 58.70% |
| environment.c | 28 | 92.86% | 67.86% |
| errors.c | 18 | 88.89% | 44.44% |
| history.c | 14 | 85.71% | 57.14% |
| main.c | 48 | 100.00% | 77.08% |
| parser.c | 26 | 100.00% | 84.62% |
| pipes.c | 96 | 72.92% | 54.17% |
| process_exec.c | 154 | 63.64% | 40.26% |
| redirection.c | 56 | 100.00% | 71.43% |
| signals.c | 10 | 100.00% | 60.00% |

### 5. Observations
- main.c, parser.c, redirection.c and signals.c reported 100% branches executed.
- process_exec.c has the lowest branch execution percentage at 63.64%.
- The taken-at-least-once metric highlights additional branch outcomes that were not observed during this test run.

### 6. Conclusion
The report records the branch execution measurements from the instrumented WSL test run. Additional targeted tests may improve branch outcome coverage.
