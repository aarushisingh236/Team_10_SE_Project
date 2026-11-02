#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "parser.h"

#define DEFAULT_PATH "/usr/bin:/bin"
#define MAX_EXPANDED_ARG 4096

typedef enum {
    ENV_EXPANSION_OK = 0,
    ENV_EXPANSION_NAME_TOO_LONG,
    ENV_EXPANSION_VALUE_TOO_LONG,
    ENV_EXPANSION_ARGUMENT_TOO_LONG
} EnvironmentExpansionError;

/**
 * @brief Retrieves the active system PATH string.
 */
const char *environment_get_path(void);

/**
 * @brief Expands environment variables in command arguments.
 *
 * @return 0 on success, -1 on failure.
 */
int environment_expand_variables(Command *command);

/**
 * @brief Returns the most recent expansion error.
 */
EnvironmentExpansionError environment_get_expansion_error(void);

/**
 * @brief Returns the name of the variable involved in the last failure.
 *        Never returns the variable's value.
 */
const char *environment_get_failed_variable(void);

#endif /* ENVIRONMENT_H */
