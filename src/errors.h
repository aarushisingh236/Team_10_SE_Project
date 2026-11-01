#ifndef ERRORS_H
#define ERRORS_H

/**
 * @brief Prints a command-not-found error.
 */
void print_error_command_not_found(const char *command_name);

/**
 * @brief Prints an error when a command cannot be executed.
 */
void print_error_command_not_executable(
    const char *command_name, const char *reason);

/**
 * @brief Prints an input-file access error.
 */
void print_error_input_file(const char *filename, const char *reason);

/**
 * @brief Prints an output-file access error.
 */
void print_error_output_file(const char *filename, const char *reason);

/**
 * @brief Prints a directory-access error.
 */
void print_error_directory(const char *directory, const char *reason);

/**
 * @brief Prints an error for a failed system operation.
 */
void print_error_system(const char *operation, const char *reason);

/**
 * @brief Prints a syntax error.
 */
void print_error_syntax(const char *message);

#endif /* ERRORS_H */
