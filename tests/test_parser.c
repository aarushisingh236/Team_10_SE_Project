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
    char line1[] = "ls -l /tmp";
    char line2[] = "echo \"hello world\"";
    char line3[] = "  \t  ";
    char line4[] = "echo 'two words'";
    char line5[] = "echo \"unclosed";
    Command command;

    expect(parse_command(line1, &command) == 0, "plain command parses");
    expect(command.argc == 3, "plain command has three arguments");
    expect(strcmp(command.argv[0], "ls") == 0, "command name is ls");
    expect(strcmp(command.argv[1], "-l") == 0, "first argument is -l");
    expect(strcmp(command.argv[2], "/tmp") == 0, "second argument is /tmp");

    expect(parse_command(line2, &command) == 0, "double-quoted argument parses");
    expect(command.argc == 2, "quoted command has two arguments");
    expect(strcmp(command.argv[1], "hello world") == 0, "double quotes preserve inner whitespace");

    expect(parse_command(line3, &command) == 0, "blank line parses");
    expect(command.argc == 0, "blank line produces no arguments");

    expect(parse_command(line4, &command) == 0, "single-quoted argument parses");
    expect(strcmp(command.argv[1], "two words") == 0, "single quotes preserve inner whitespace");

    expect(parse_command(line5, &command) != 0, "unclosed quote is rejected");

    if (failures != 0) {
        fprintf(stderr, "%d parser test(s) failed.\n", failures);
        return 1;
    }

    puts("All parser tests passed.");
    return 0;
}
