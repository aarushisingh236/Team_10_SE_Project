#ifndef REDIRECTION_H
#define REDIRECTION_H

#include "parser.h"

/**
 * @brief Parses and extracts an input redirection (<) token from the command arguments.
 *
 * Modifies the Command structure to remove the redirection token and target filename.
 *
 * @param command Pointer to the parsed command.
 * @param input_file Output pointer to the redirection input filename, or NULL if none.
 * @return 0 on success (or if no input redirection is present), -1 on syntax/usage error.
 */
int parse_input_redirection(Command *command, char **input_file);

/**
 * @brief Parses and extracts an output redirection (> or >>) token from the command arguments.
 *
 * Modifies the Command structure to remove the redirection token and target filename.
 *
 * @param command Pointer to the parsed command.
 * @param output_file Output pointer to the redirection output filename, or NULL if none.
 * @param append_output Output flag set to 1 if append mode (>>), 0 for overwrite (>).
 * @return 0 on success (or if no output redirection is present), -1 on syntax/usage error.
 */
int parse_output_redirection(Command *command, char **output_file, int *append_output);

/**
 * @brief Opens an output file with appropriate flags (overwrite or append).
 *
 * @param output_file Path to the target output file.
 * @param append_output 1 for append (O_APPEND), 0 for truncate (O_TRUNC).
 * @return File descriptor on success, or -1 on error.
 */
int open_output_file(const char *output_file, int append_output);

#endif /* REDIRECTION_H */
