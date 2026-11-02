#define _POSIX_C_SOURCE 200809L

#include "environment.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_SECRET_SIZE 5000

int main(void)
{
    Command command = {0};
    char secret[TEST_SECRET_SIZE];
    char oversized_name[MAX_EXPANDED_ARG + 2];
    char normal_argument[] = "$HOME";

    memset(secret, 'S', sizeof(secret) - 1);
    secret[sizeof(secret) - 1] = '\0';

    if (setenv("TEST_SECRET", secret, 1) != 0) {
        fprintf(stderr, "FAIL: could not set test environment variable\n");
        return 1;
    }

    command.argc = 2;
    command.argv[0] = "echo";
    command.argv[1] = "$TEST_SECRET";

    if (environment_expand_variables(&command) != -1) {
        fprintf(stderr, "FAIL: oversized environment value was accepted\n");
        return 1;
    }

    if (environment_get_expansion_error() !=
        ENV_EXPANSION_VALUE_TOO_LONG) {
        fprintf(stderr, "FAIL: incorrect expansion error type\n");
        return 1;
    }

    if (strcmp(environment_get_failed_variable(), "TEST_SECRET") != 0) {
        fprintf(stderr, "FAIL: incorrect failed variable name\n");
        return 1;
    }

    if (strstr(environment_get_failed_variable(), secret) != NULL) {
        fprintf(stderr, "FAIL: secret value leaked into diagnostic data\n");
        return 1;
    }

    command.argc = 2;
    command.argv[0] = "echo";
    command.argv[1] = normal_argument;

    if (environment_expand_variables(&command) != 0) {
        fprintf(stderr, "FAIL: normal variable expansion failed\n");
        return 1;
    }

    if (strcmp(command.argv[1], getenv("HOME")) != 0) {
        fprintf(stderr, "FAIL: HOME expansion produced an incorrect value\n");
        return 1;
    }

    oversized_name[0] = '$';
    memset(oversized_name + 1, 'A', MAX_EXPANDED_ARG);
    oversized_name[MAX_EXPANDED_ARG + 1] = '\0';

    command.argc = 2;
    command.argv[0] = "echo";
    command.argv[1] = oversized_name;

    if (environment_expand_variables(&command) != -1 ||
        environment_get_expansion_error() !=
        ENV_EXPANSION_NAME_TOO_LONG) {
        fprintf(stderr, "FAIL: oversized variable name was not rejected\n");
        return 1;
    }

    unsetenv("TEST_SECRET");

    printf("Environment security tests passed.\n");
    return 0;
}
