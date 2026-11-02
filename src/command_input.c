#define _POSIX_C_SOURCE 200809L

#include "command_input.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#define INPUT_BUFFER_SIZE 4096

static unsigned char input_buffer[INPUT_BUFFER_SIZE];
static size_t buffer_position = 0;
static size_t buffer_length = 0;

static int read_next_character(void)
{
    if (buffer_position >= buffer_length) {
        ssize_t bytes_read;

        size_t read_size = isatty(STDIN_FILENO)
            ? sizeof(input_buffer) : 1;

        bytes_read = read(STDIN_FILENO, input_buffer, read_size);

        if (bytes_read < 0 && errno == EINTR) {
            return -3;
        }

        if (bytes_read <= 0) {
            return bytes_read == 0 ? EOF : -2;
        }

        buffer_position = 0;
        buffer_length = (size_t)bytes_read;
    }

    return input_buffer[buffer_position++];
}

InputResult read_command_line(char *buffer, size_t capacity)
{
    int ch;
    size_t length = 0;
    int too_long = 0;
    int invalid = 0;

    if (buffer == NULL || capacity < 2) {
        return INPUT_ERROR;
    }

    fputs("shell> ", stdout);
    fflush(stdout);

    while ((ch = read_next_character()) != '\n' && ch != EOF) {
        if (ch == -3) {
            return INPUT_INTERRUPTED;
        }

        if (ch == -2) {
            return INPUT_ERROR;
        }

        if (ch == '\0' || (ch < 32 && ch != '\t') || ch == 127) {
            invalid = 1;
            continue;
        }

        if (length + 1 < capacity) {
            buffer[length++] = (char)ch;
        } else {
            too_long = 1;
        }
    }

    if (ch == EOF && length == 0 && !too_long && !invalid) {
        return INPUT_EOF;
    }

    buffer[length] = '\0';

    if (invalid) {
        return INPUT_INVALID;
    }

    if (too_long) {
        return INPUT_TOO_LONG;
    }

    return INPUT_OK;
}
