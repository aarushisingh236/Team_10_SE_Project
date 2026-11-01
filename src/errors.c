#define _POSIX_C_SOURCE 200809L

#include "errors.h"

#include <stdio.h>

static const char *error_reason(const char *reason)
{
    return (reason != NULL && reason[0] != '\0')
        ? reason : "unknown error";
}

void print_error_command_not_found(const char *command_name)
{
    fprintf(stderr,
            "Error: The command '%s' was not found. Check its spelling.\n",
            command_name != NULL ? command_name : "");
}

void print_error_command_not_executable(
    const char *command_name, const char *reason)
{
    fprintf(stderr,
            "Error: The command '%s' cannot be executed: %s\n",
            command_name != NULL ? command_name : "",
            error_reason(reason));
}

void print_error_input_file(const char *filename, const char *reason)
{
    fprintf(stderr,
            "Error: cannot open input file '%s': %s\n",
            filename != NULL ? filename : "",
            error_reason(reason));
}

void print_error_output_file(const char *filename, const char *reason)
{
    fprintf(stderr,
            "Error: cannot open output file '%s': %s\n",
            filename != NULL ? filename : "",
            error_reason(reason));
}

void print_error_directory(const char *directory, const char *reason)
{
    fprintf(stderr,
            "Error: cannot access directory '%s': %s\n",
            directory != NULL ? directory : "",
            error_reason(reason));
}

void print_error_system(const char *operation, const char *reason)
{
    fprintf(stderr,
            "Error: system operation '%s' failed: %s\n",
            operation != NULL ? operation : "unknown",
            error_reason(reason));
}

void print_error_syntax(const char *message)
{
    if (message != NULL) {
        fputs(message, stderr);
    }
}
