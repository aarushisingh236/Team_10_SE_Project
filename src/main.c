#include "builtins.h"
#include "command_input.h"
#include "parser.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    char line[SHELL_MAX_LINE];
    Command command;

    for (;;) {
        InputResult result = read_command_line(line, sizeof(line));

        if (result == INPUT_EOF) {
            putchar('\n');
            break;
        }

        if (result == INPUT_TOO_LONG) {
            fputs("Error: command line is too long; input was rejected.\n", stderr);
            continue;
        }

        if (result == INPUT_ERROR) {
            fputs("Error: unable to read command input.\n", stderr);
            return 1;
        }

        if (parse_command(line, &command) != 0) {
            fputs("Error: malformed command input (check quotes or argument count).\n", stderr);
            continue;
        }

        /* Blank or whitespace-only input is ignored. */
        if (command.argc == 0) {
            continue;
        }

        if (handle_builtin(&command)) {
            /* Reserved for future built-ins. */
            continue;
        }

        /*
         * External command execution is intentionally left for the relevant
         * sprint stories. Do not invoke system() or pass raw input to a shell.
         */
        fprintf(stderr, "Error: command '%s' is not implemented yet.\n", command.argv[0]);
    }

    return 0;
}
