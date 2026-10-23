#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int handle_builtin(const Command *command)
{
    int i;

    if (command == NULL || command->argc == 0 ||
        command->argv[0] == NULL) {
        return 0;
    }

    if (strcmp(command->argv[0], "exit") == 0) {
        exit(0);
    }

    if (strcmp(command->argv[0], "echo") == 0) {
        for (i = 1; i < command->argc; i++) {
            if (i > 1) {
                putchar(' ');
            }
            fputs(command->argv[i], stdout);
        }

        putchar('\n');
        return 1;
    }

    return 0;
}
