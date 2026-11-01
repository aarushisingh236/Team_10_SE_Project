#ifndef PIPES_H
#define PIPES_H

#include "parser.h"

/**
 * @brief Detects whether a command contains standalone pipe operators ('|').
 *
 * Quoted pipe characters ('|') are not treated as pipe operators.
 *
 * @param command Pointer to the parsed command.
 * @return 1 if any unquoted standalone pipe operator is detected, 0 otherwise.
 */
int has_pipe_operator(const Command *command);

/**
 * @brief Executes a pipeline of commands connected via pipes.
 *
 * Splits command arguments on unquoted '|' tokens, sets up pipes,
 * forks child processes with redirected stdin/stdout, and waits for all children.
 *
 * @param command Pointer to the parsed command containing the pipeline.
 */
void execute_pipeline(Command *command);

#endif /* PIPES_H */
