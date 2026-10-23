#include "parser.h"

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
    char plain[] = "ls -l /tmp";
    char double_quoted[] = "echo \"hello world\"";
    char single_quoted[] = "echo 'two words'";
    char mixed_quotes[] = "echo pre\"two words\"post";
    char whitespace[] = "  ls\t\t-l   /tmp  ";
    char blank[] = "  \t  ";
    char unmatched[] = "echo \"unclosed";
    char empty_quoted[] = "echo \"\"";
    char many_args[4096] = {0};
    char too_many_args[4096] = {0};
    Command command;
    int i;
    size_t used;

    expect(parse_command(plain, &command) == 0, "plain command parses");
    expect(command.argc == 3, "plain command has three arguments");
    expect(strcmp(command.argv[0], "ls") == 0, "command name is ls");
    expect(strcmp(command.argv[1], "-l") == 0, "first argument is -l");
    expect(strcmp(command.argv[2], "/tmp") == 0, "second argument is /tmp");
    expect(command.argv[3] == NULL, "argument list is NULL-terminated");

    expect(parse_command(double_quoted, &command) == 0,
           "double-quoted argument parses");
    expect(command.argc == 2, "double-quoted command has two arguments");
    expect(strcmp(command.argv[1], "hello world") == 0,
           "double quotes preserve spaces");

    expect(parse_command(single_quoted, &command) == 0,
           "single-quoted argument parses");
    expect(strcmp(command.argv[1], "two words") == 0,
           "single quotes preserve spaces");

    expect(parse_command(mixed_quotes, &command) == 0,
           "mixed quoted and unquoted text parses");
    expect(strcmp(command.argv[1], "pretwo wordspost") == 0,
           "adjacent text forms one argument");

    expect(parse_command(whitespace, &command) == 0,
           "repeated spaces and tabs are accepted");
    expect(command.argc == 3, "whitespace does not create extra arguments");

    expect(parse_command(blank, &command) == 0, "blank input parses");
    expect(command.argc == 0, "blank input produces no arguments");

    expect(parse_command(empty_quoted, &command) == 0,
           "empty quoted argument parses");
    expect(command.argc == 2, "empty quoted argument is retained");
    expect(strcmp(command.argv[1], "") == 0,
           "empty quoted argument is an empty string");

    expect(parse_command(unmatched, &command) != 0,
           "unmatched quotes are rejected");
    expect(command.argc == 0, "failed parse clears argument count");

    used = 0;
    for (i = 0; i < SHELL_MAX_ARGS; i++) {
        int written = snprintf(many_args + used, sizeof(many_args) - used,
                               "%sarg", i == 0 ? "" : " ");
        if (written < 0 || (size_t)written >= sizeof(many_args) - used) {
            fprintf(stderr, "Could not construct maximum-argument test.\n");
            return 1;
        }
        used += (size_t)written;
    }

    expect(parse_command(many_args, &command) == 0,
           "maximum supported arguments parse");
    expect(command.argc == SHELL_MAX_ARGS,
           "maximum argument count is preserved");

    used = 0;
    for (i = 0; i < SHELL_MAX_ARGS + 1; i++) {
        int written = snprintf(too_many_args + used,
                               sizeof(too_many_args) - used,
                               "%sarg", i == 0 ? "" : " ");
        if (written < 0 ||
            (size_t)written >= sizeof(too_many_args) - used) {
            fprintf(stderr, "Could not construct too-many-arguments test.\n");
            return 1;
        }
        used += (size_t)written;
    }

    expect(parse_command(too_many_args, &command) != 0,
           "excess arguments are rejected");

    if (failures != 0) {
        fprintf(stderr, "%d parser test(s) failed.\n", failures);
        return 1;
    }

    puts("All parser tests passed.");
    return 0;
}
