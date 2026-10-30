#define _POSIX_C_SOURCE 200809L

#include "redirection.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int parse_input_redirection(
    Command *command, char **input_file)
{
    int i;
    int redirect_index = -1;

    *input_file = NULL;

    for (i = 0; i < command->argc; i++) {
        if (strcmp(command->argv[i], "<") == 0) {
            if (redirect_index != -1) {
                fprintf(stderr,
                        "Error: Use only one input redirection (<) in a command.\n");
                return -1;
            }

            redirect_index = i;
        }
    }

    if (redirect_index == -1) {
        return 0;
    }

    if (redirect_index + 1 >= command->argc ||
        command->argv[redirect_index + 1][0] == '\0' ||
        strcmp(command->argv[redirect_index + 1], "<") == 0 ||
        strcmp(command->argv[redirect_index + 1], ">") == 0 ||
        strcmp(command->argv[redirect_index + 1], ">>") == 0) {
        fprintf(stderr,
                "Error: input redirection requires a filename.\n");
        return -1;
    }

    *input_file = command->argv[redirect_index + 1];

    for (i = redirect_index; i + 2 < command->argc; i++) {
        command->argv[i] = command->argv[i + 2];
    }

    command->argc -= 2;
    command->argv[command->argc] = NULL;

    if (command->argc == 0) {
        fprintf(stderr,
                "Error: Add a command before the input filename.\n");
        return -1;
    }

    return 0;
}

int parse_output_redirection(
    Command *command,
    char **output_file,
    int *append_output)
{
    int i;
    int redirect_index = -1;

    *output_file = NULL;
    *append_output = 0;

    for (i = 0; i < command->argc; i++) {
        if (strcmp(command->argv[i], ">") == 0 ||
            strcmp(command->argv[i], ">>") == 0) {
            if (redirect_index != -1) {
                fprintf(stderr,
                        "Error: Use only one output redirection (>, or >>) in a command.\n");
                return -1;
            }

            redirect_index = i;
            *append_output =
                strcmp(command->argv[i], ">>") == 0;
        }
    }

    if (redirect_index == -1) {
        return 0;
    }

    if (redirect_index + 1 >= command->argc ||
        command->argv[redirect_index + 1][0] == '\0' ||
        strcmp(command->argv[redirect_index + 1], ">") == 0 ||
        strcmp(command->argv[redirect_index + 1], ">>") == 0 ||
        strcmp(command->argv[redirect_index + 1], "<") == 0) {
        fprintf(stderr,
                "Error: output redirection requires a filename.\n");
        return -1;
    }

    *output_file = command->argv[redirect_index + 1];

    for (i = redirect_index; i + 2 < command->argc; i++) {
        command->argv[i] = command->argv[i + 2];
    }

    command->argc -= 2;
    command->argv[command->argc] = NULL;

    if (command->argc == 0) {
        fprintf(stderr,
                "Error: Add a command before the output filename.\n");
        return -1;
    }

    return 0;
}

int open_output_file(
    const char *output_file, int append_output)
{
    int flags = O_WRONLY | O_CREAT;

    if (append_output) {
        flags |= O_APPEND;
    } else {
        flags |= O_TRUNC;
    }

    return open(output_file, flags, 0666);
}
