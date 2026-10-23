#ifndef COMMAND_INPUT_H
#define COMMAND_INPUT_H

#include <stddef.h>

#define SHELL_MAX_LINE 1024

typedef enum {
    INPUT_OK,
    INPUT_EOF,
    INPUT_TOO_LONG,
    INPUT_ERROR
} InputResult;

/* Prints the prompt and reads one bounded line from stdin. */
InputResult read_command_line(char *buffer, size_t capacity);

#endif
