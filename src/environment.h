#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "parser.h"

#define DEFAULT_PATH "/usr/bin:/bin"
#define MAX_EXPANDED_ARG 4096

/**
 * @brief Retrieves the active system PATH string, or a sensible default if unset.
 *
 * @return Active PATH string pointer.
 */
const char *environment_get_path(void);

/**
 * @brief Expands environment variables prefixed with '$' in command arguments.
 *
 * Scans each argument for $VAR_NAME (alphanumeric and underscore) and replaces
 * it in-place using getenv().
 *
 * @param command Pointer to the command struct whose arguments are expanded.
 * @return 0 on success, -1 on buffer overflow or expansion failure.
 */
int environment_expand_variables(Command *command);

#endif /* ENVIRONMENT_H */
