#define _POSIX_C_SOURCE 200809L

#include "errors.h"
#include <stdio.h>

void print_error_command_not_found(const char *command_name)
{
    fprintf(stderr,
            "Error: The command '%s' was not found. Check its spelling.\n",
            command_name ? command_name : "");
}

void print_error_input_file(const char *filename, const char *reason)
{
    fprintf(stderr,
            "Error: cannot open input file '%s': %s\n",
            filename ? filename : "",
            reason ? reason : "");
}

void print_error_output_file(const char *filename, const char *reason)
{
    fprintf(stderr,
            "Error: cannot open output file '%s': %s\n",
            filename ? filename : "",
            reason ? reason : "");
}

void print_error_syntax(const char *message)
{
    if (message != NULL) {
        fputs(message, stderr);
    }
}
