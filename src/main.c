#define _POSIX_C_SOURCE 200809L

#include "builtins.h"
#include "command_input.h"
#include "environment.h"
#include "errors.h"
#include "history.h"
#include "parser.h"
#include "pipes.h"
#include "process_exec.h"
#include "redirection.h"
#include "signals.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    char line[SHELL_MAX_LINE];
    char original_line[SHELL_MAX_LINE];
    Command command;

    if (setup_signal_handlers() < 0) {
        return 1;
    }

    history_init();

    for (;;) {
        InputResult result = read_command_line(line, sizeof(line));

        if (result == INPUT_EOF) {
            putchar('\n');
            break;
        }

        strncpy(original_line, line, SHELL_MAX_LINE - 1);
        original_line[SHELL_MAX_LINE - 1] = '\0';

        if (result == INPUT_TOO_LONG) {
            print_error_syntax(
                "Error: command line is too long. Shorten it and try again.\n");
            continue;
        }

        if (result == INPUT_ERROR) {
            print_error_syntax(
                "Error: Could not read your command. Please try again.\n");
            return 1;
        }

        if (parse_command(line, &command) != 0) {
            print_error_syntax(
                "Error: malformed command input. Check your quotes and number of arguments.\n");
            continue;
        }

        if (command.argc == 0) {
            continue;
        }

        if (strcmp(command.argv[0], "history") != 0) {
            history_add(original_line);
        }

        if (has_pipe_operator(&command)) {
            print_error_syntax(
                "Error: malformed command input (unsupported pipe operator).\n");
            continue;
        }

        char *input_file = NULL;
        char *output_file = NULL;
        int append_output = 0;
        int background = 0;

        /* Detect a trailing '&' to request background execution. */
        if (command.argc > 0 &&
            strcmp(command.argv[command.argc - 1], "&") == 0) {
            background = 1;
            command.argc--;
            command.argv[command.argc] = NULL;

            if (command.argc == 0) {
                print_error_syntax("Error: Add a command before '&'.\n");
                continue;
            }
        }

        if (parse_input_redirection(&command, &input_file) != 0) {
            continue;
        }

        if (parse_output_redirection(
                &command, &output_file, &append_output) != 0) {
            continue;
        }

        if (command.argc == 0) {
            continue;
        }

        if (is_builtin(command.argv[0])) {
            (void)execute_builtin(
                &command, input_file, output_file, append_output,
                history_get_entries(), history_get_count());
            continue;
        }

        char exe_path[EXE_PATH_MAX];

        ResolveResult rc = resolve_command(
            command.argv[0], exe_path, sizeof(exe_path));

        if (rc == RESOLVE_NOT_FOUND) {
            print_error_command_not_found(command.argv[0]);
            continue;
        }

        if (rc != RESOLVE_OK) {
            fprintf(stderr,
                    "Error: Could not locate the program '%s' because of an internal error.\n",
                    command.argv[0]);
            continue;
        }

        execute_external(
            exe_path, &command, input_file, output_file, append_output,
            background);
    }

    return 0;
}
