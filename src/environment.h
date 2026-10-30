#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "parser.h"

#define DEFAULT_PATH "/usr/bin:/bin"

/**
 * @brief Retrieves the active system PATH string, or a sensible default if unset.
 *
 * @return Active PATH string pointer.
 */
const char *environment_get_path(void);

/**
 * @brief Expands environment variables prefixed with '$' in command arguments.
 *
 * @param command Command struct whose arguments are inspected.
 */
void environment_expand_variables(Command *command);

#endif /* ENVIRONMENT_H */
