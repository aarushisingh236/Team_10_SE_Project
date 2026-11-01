#define _POSIX_C_SOURCE 200809L

#include "environment.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

const char *environment_get_path(void)
{
    const char *p = getenv("PATH");
    if (p == NULL) {
        p = DEFAULT_PATH;
    }
    return p;
}

int environment_expand_variables(Command *command)
{
    static char expanded[SHELL_MAX_ARGS][MAX_EXPANDED_ARG];
    int i;

    if (command == NULL) {
        return 0;
    }

    for (i = 0; i < command->argc; i++) {
        const char *src = command->argv[i];
        size_t used = 0;

        while (*src != '\0') {
            if (*src == '$' &&
                (isalpha((unsigned char)src[1]) || src[1] == '_')) {
                char name[MAX_EXPANDED_ARG];
                size_t name_len = 0;
                const char *value;
                size_t value_len;

                src++;

                while (isalnum((unsigned char)*src) || *src == '_') {
                    if (name_len >= sizeof(name) - 1) {
                        return -1;
                    }
                    name[name_len++] = *src++;
                }

                name[name_len] = '\0';
                value = getenv(name);

                if (value == NULL) {
                    value = "";
                }

                value_len = strlen(value);

                if (value_len >= MAX_EXPANDED_ARG - used) {
                    return -1;
                }

                memcpy(expanded[i] + used, value, value_len);
                used += value_len;
            } else {
                if (used >= MAX_EXPANDED_ARG - 1) {
                    return -1;
                }

                expanded[i][used++] = *src++;
            }
        }

        expanded[i][used] = '\0';
        command->argv[i] = expanded[i];
    }

    return 0;
}
