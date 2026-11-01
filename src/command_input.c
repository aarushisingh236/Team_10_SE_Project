#include "command_input.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

InputResult read_command_line(char *buffer, size_t capacity)
{
    int ch;
    size_t length = 0;
    int too_long = 0;

    if (buffer == NULL || capacity < 2) {
        return INPUT_ERROR;
    }

    fputs("shell> ", stdout);
    fflush(stdout);

    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (length + 1 < capacity) {
            buffer[length++] = (char)ch;
        } else {
            too_long = 1;
        }
    }
    if (ch == EOF && ferror(stdin) && errno == EINTR) {
        clearerr(stdin);
        return INPUT_INTERRUPTED;
    }
    if (ch == EOF && length == 0 && !too_long) {
        return INPUT_EOF;
    }

    buffer[length] = '\0';

    if (too_long) {
        return INPUT_TOO_LONG;
    }

    return INPUT_OK;
}
