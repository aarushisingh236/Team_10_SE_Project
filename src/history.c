#define _POSIX_C_SOURCE 200809L

#include "history.h"
#include <stdio.h>
#include <string.h>

static char s_history[SHELL_HISTORY_CAPACITY][SHELL_MAX_LINE];
static const char *s_history_ptrs[SHELL_HISTORY_CAPACITY];
static int s_history_count = 0;

void history_init(void)
{
    s_history_count = 0;
}

void history_add(const char *line)
{
    if (line == NULL || line[0] == '\0') {
        return;
    }

    if (s_history_count < SHELL_HISTORY_CAPACITY) {
        strncpy(s_history[s_history_count], line, SHELL_MAX_LINE - 1);
        s_history[s_history_count][SHELL_MAX_LINE - 1] = '\0';
        s_history_ptrs[s_history_count] = s_history[s_history_count];
        s_history_count++;
    } else {
        int i;
        for (i = 0; i < SHELL_HISTORY_CAPACITY - 1; i++) {
            strncpy(s_history[i], s_history[i + 1], SHELL_MAX_LINE - 1);
            s_history[i][SHELL_MAX_LINE - 1] = '\0';
            s_history_ptrs[i] = s_history[i];
        }
        strncpy(s_history[SHELL_HISTORY_CAPACITY - 1], line, SHELL_MAX_LINE - 1);
        s_history[SHELL_HISTORY_CAPACITY - 1][SHELL_MAX_LINE - 1] = '\0';
        s_history_ptrs[SHELL_HISTORY_CAPACITY - 1] = s_history[SHELL_HISTORY_CAPACITY - 1];
    }
}

int history_get_count(void)
{
    return s_history_count;
}

const char **history_get_entries(void)
{
    return s_history_ptrs;
}

void history_print(void)
{
    int i;
    for (i = 0; i < s_history_count; i++) {
        printf("%d  %s\n", i + 1, s_history[i]);
    }
}
