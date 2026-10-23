
#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

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

    if (strcmp(command->argv[0], "cd") == 0) {
        const char *directory;

        if (command->argc == 1) {
            directory = getenv("HOME");

            if (directory == NULL || directory[0] == '\0') {
                fprintf(stderr, "cd: HOME not set\n");
                return 1;
            }
        } else if (command->argc == 2) {
            directory = command->argv[1];
        } else {
            fprintf(stderr, "cd: too many arguments\n");
            return 1;
        }

        if (chdir(directory) != 0) {
            fprintf(stderr, "cd: %s: %s\n",
                    directory, strerror(errno));
        }

        return 1;
    }

    return 0;
}
