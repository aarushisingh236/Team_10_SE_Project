#include "history.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        failures++;
    }
}

int main(void)
{
    const char **entries;

    history_init();

    history_add(NULL);
    history_add("");
    history_add("   ");
    history_add("\t\t");
    history_add(" \t\n ");

    expect(history_get_count() == 0,
           "empty and whitespace-only input is not recorded");

    history_add("echo first");
    history_add("ls -l");

    expect(history_get_count() == 2,
           "valid commands are recorded");

    entries = history_get_entries();

    expect(strcmp(entries[0], "echo first") == 0,
           "first command is preserved");

    expect(strcmp(entries[1], "ls -l") == 0,
           "second command is preserved");

    if (failures != 0) {
        fprintf(stderr, "%d history test(s) failed.\n", failures);
        return 1;
    }

    puts("All history tests passed.");
    return 0;
}
