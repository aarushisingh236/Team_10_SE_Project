#ifndef PIPES_H
#define PIPES_H

#include "parser.h"

/**
 * @brief Detects whether a command contains standalone pipe operators ('|').
 *
 * @param command Pointer to the parsed command.
 * @return 1 if any standalone pipe operator is detected, 0 otherwise.
 */
int has_pipe_operator(const Command *command);

#endif /* PIPES_H */
