#define _POSIX_C_SOURCE 200809L

#include "environment.h"
#include <stdlib.h>

const char *environment_get_path(void)
{
    const char *p = getenv("PATH");
    if (p == NULL) {
        p = DEFAULT_PATH;
    }
    return p;
}

void environment_expand_variables(Command *command)
{
    (void)command;
}
