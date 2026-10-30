#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser.h"

/**
 * @brief Checks if a given command name corresponds to a shell built-in command.
 *
 * @param name Command name string.
 * @return 1 if command is built-in (cd, pwd, echo, help, exit, history), 0 otherwise.
 */
int is_builtin(const char *name);

/**
 * @brief Executes a shell built-in command.
 *
 * @param command Parsed command struct.
 * @param history Array of recorded command lines in history.
 * @param history_count Number of entries in history.
 * @return 1 if handled, 0 otherwise.
 */
int handle_builtin(const Command *command,
                   const char *history[],
                   int history_count);

#endif /* BUILTINS_H */
