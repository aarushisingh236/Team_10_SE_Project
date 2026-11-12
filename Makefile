CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Werror -g
CPPFLAGS := -Isrc

SRC := src/main.c \
       src/command_input.c \
       src/parser.c \
       src/builtins.c \
       src/process_exec.c \
       src/redirection.c \
       src/pipes.c \
       src/environment.c \
       src/history.c \
       src/signals.c \
       src/errors.c

OBJ := $(SRC:.c=.o)
TARGET := simple-shell
TEST_TARGET := tests/test_parser
HISTORY_TEST_TARGET := tests/test_history
ENVIRONMENT_TEST_TARGET := tests/test_environment

.PHONY: all clean test coverage-test coverage-clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

src/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TEST_TARGET): tests/test_parser.c src/parser.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(HISTORY_TEST_TARGET): tests/test_history.c src/history.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(ENVIRONMENT_TEST_TARGET): tests/test_environment.c src/environment.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

test: $(TARGET) $(TEST_TARGET) $(HISTORY_TEST_TARGET) $(ENVIRONMENT_TEST_TARGET)
	./$(TEST_TARGET)
	./$(HISTORY_TEST_TARGET)
	./$(ENVIRONMENT_TEST_TARGET)
	/bin/bash tests/test_shell.sh ./$(TARGET)
	python3 tests/test_prompt_response.py ./$(TARGET)
	python3 tests/test_environment_security.py ./$(TARGET)
	python3 tests/test_reliability.py ./$(TARGET)
	python3 tests/test_background_pid.py ./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET) $(TEST_TARGET) $(HISTORY_TEST_TARGET) $(ENVIRONMENT_TEST_TARGET)


# Build and run the complete test suite with GCC coverage instrumentation.
coverage-test:
	$(MAKE) coverage-clean
	$(MAKE) CC=gcc CFLAGS="$(CFLAGS) -O0 --coverage" test

# Remove coverage data and generated gcov reports.
coverage-clean: clean
	find src tests -type f \( -name '*.gcda' -o -name '*.gcno' -o -name '*.gcov' \) -delete
	rm -f ./*.gcov
