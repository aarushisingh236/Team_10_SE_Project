# Code Coverage Report
## Team 10 — Simple Shell

### Team Members
- Aarushi Singh — PES2UG24CS015
- Akshaya M — PES2UG24CS050
- Akshay Anil Kumar — PES2UG24CS049
- Adarsh Shivanand Madli — PES2UG24CS028

---

### 1. Objective
To measure source-line execution during the automated test run using GCC gcov.

### 2. Coverage Configuration
- Compiler: GCC 13.3.0
- Optimization: `-O0`
- Instrumentation: `--coverage`
- Reporting tool: gcov
- Command: `gcov -b -c -o src src/*.c`

### 3. Overall Result

| Metric | Result |
|---|---:|
| Source lines analysed | 727 |
| Lines executed | Approximately 491 |
| Overall line coverage | 67.54% |

### 4. Module-wise Line Coverage

| Module | Lines analysed | Line coverage |
|---|---:|---:|
| builtins.c | 55 | 60.00% |
| command_input.c | 39 | 84.62% |
| environment.c | 47 | 89.36% |
| errors.c | 25 | 88.00% |
| history.c | 31 | 77.42% |
| main.c | 85 | 74.12% |
| parser.c | 39 | 92.31% |
| pipes.c | 118 | 60.17% |
| process_exec.c | 191 | 46.60% |
| redirection.c | 70 | 82.86% |
| signals.c | 27 | 74.07% |

### 5. Observations
- parser.c recorded the highest line coverage at 92.31%.
- environment.c recorded 89.36%.
- process_exec.c recorded the lowest line coverage at 46.60%.
- The results identify areas where additional tests could improve execution coverage.

### 6. Conclusion
The automated test run executed 67.54% of the analysed source lines. These measurements represent the recorded WSL test run and are not a claim of complete code coverage.
