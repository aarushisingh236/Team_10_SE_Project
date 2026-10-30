#ifndef ERRORS_H
#define ERRORS_H

/**
 * @brief Prints a standard plain-English command-not-found error message.
 *
 * @param command_name Unrecognized command name.
 */
void print_error_command_not_found(const char *command_name);

/**
 * @brief Prints an error message when an input redirection file cannot be opened.
 *
 * @param filename Target input file.
 * @param reason System error string.
 */
void print_error_input_file(const char *filename, const char *reason);

/**
 * @brief Prints an error message when an output redirection file cannot be opened.
 *
 * @param filename Target output file.
 * @param reason System error string.
 */
void print_error_output_file(const char *filename, const char *reason);

/**
 * @brief Prints a syntax error message to stderr.
 *
 * @param message Plain-English syntax description.
 */
void print_error_syntax(const char *message);

#endif /* ERRORS_H */
