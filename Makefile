CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Werror -g
CPPFLAGS := -Isrc

SRC := src/main.c src/command_input.c src/parser.c src/builtins.c
OBJ := $(SRC:.c=.o)
TARGET := simple-shell
TEST_TARGET := tests/test_parser

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

src/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TEST_TARGET): tests/test_parser.c src/parser.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

test: $(TARGET) $(TEST_TARGET)
	./$(TEST_TARGET)
	bash tests/test_shell.sh ./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET) $(TEST_TARGET)
