#include "builtins.h"

#include <stdlib.h>
#include <string.h>

int handle_builtin(const Command *command)
{
    if (command == NULL || command->argc == 0 || command->argv[0] == NULL) {
        return 0;
    }

    if (strcmp(command->argv[0], "exit") == 0) {
        exit(0);
    }

    return 0;
}
