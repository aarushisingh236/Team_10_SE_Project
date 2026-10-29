#include "command_input.h"

#include <stdio.h>
#include <string.h>

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

    while ((ch = getchar()) != '\n' && ch != EOF) {
        /*
         * Reject embedded NUL and non-printable control characters,
         * including tabs and DEL.
         * Continue consuming the line after rejection so the next
         * command starts at a clean line boundary.
         */
        if (ch == '\0' ||
            (ch < 32) ||
            ch == 127) {
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
