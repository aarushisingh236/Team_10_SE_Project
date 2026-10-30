#define _POSIX_C_SOURCE 200809L

#include "pipes.h"

int has_pipe_operator(const Command *command)
{
    int i;

    if (command == NULL) {
        return 0;
    }

    for (i = 0; i < command->argc; i++) {
        const char *token = command->argv[i];

        if (token != NULL && token[0] == '|') {
            int j = 0;

            while (token[j] == '|') {
                j++;
            }

            if (token[j] == '\0') {
                return 1;
            }
        }
    }

    return 0;
}
