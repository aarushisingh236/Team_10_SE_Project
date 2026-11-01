#include "parser.h"

#include <ctype.h>
#include <stddef.h>

int parse_command(char *line, Command *command)
{
    char *read_ptr;
    char *write_ptr;

    if (line == NULL || command == NULL) {
        return -1;
    }

    command->argc = 0;
    command->argv[0] = NULL;

    read_ptr = line;
    write_ptr = line;

    while (*read_ptr != '\0') {
        char quote = '\0';

        while (isspace((unsigned char)*read_ptr)) {
            read_ptr++;
        }

        if (*read_ptr == '\0') {
            break;
        }

        if (command->argc >= SHELL_MAX_ARGS) {
            command->argv[command->argc] = NULL;
            return -1;
        }

        int arg_index = command->argc;
        command->argv[arg_index] = write_ptr;
        command->quoted[arg_index] = 0;
        command->argc++;

        while (*read_ptr != '\0') {
            char current = *read_ptr++;

            if (quote != '\0') {
                if (current == quote) {
                    quote = '\0';
                } else {
                    *write_ptr++ = current;
                }
            }  else if (current == '\'' || current == '"') {
                    quote = current;
                    command->quoted[arg_index] = 1;
             } else if (isspace((unsigned char)current)) {
                break;
            } else {
                *write_ptr++ = current;
            }
        }

        if (quote != '\0') {
            command->argc = 0;
            command->argv[0] = NULL;
            return -1;
        }

        *write_ptr++ = '\0';

        /* If the delimiter was whitespace, read_ptr already points after it. */
    }

    command->argv[command->argc] = NULL;
    return 0;
}
