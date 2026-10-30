#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#define SHELL_MAX_ARGS 128

typedef struct {
    int argc;
    char *argv[SHELL_MAX_ARGS + 1];
    int quoted[SHELL_MAX_ARGS];
} Command;

/*
 * Tokenizes whitespace-separated arguments and supports single/double quotes.
 * The input line is modified in place. Returns 0 on success, -1 on malformed
 * quotes or too many arguments. An empty/blank line succeeds with argc == 0.
 */
int parse_command(char *line, Command *command);

#endif
