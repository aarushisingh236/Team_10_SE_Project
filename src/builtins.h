#ifndef BUILTINS_H
#define BUILTINS_H

#include "parser.h"

/* Returns 1 if the command was the built-in 'exit'; otherwise returns 0. */
int handle_builtin(const Command *command,
                  const char *history[],
                  int history_count);

#endif
