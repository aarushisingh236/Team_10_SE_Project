#ifndef PROCESS_EXEC_H
#define PROCESS_EXEC_H

#include "parser.h"
#include <stddef.h>

#define EXE_PATH_MAX 4096

typedef enum {
    RESOLVE_OK,
    RESOLVE_NOT_FOUND,
    RESOLVE_ERROR
} ResolveResult;

/**
 * @brief Checks if a path is an executable regular file.
 *
 * @param path File path to test.
 * @return 1 if regular executable file, 0 otherwise.
 */
int is_executable_file(const char *path);

/**
 * @brief Resolves a command name or path to an executable file using PATH.
 *
 * @param cmd Command name or relative/absolute path.
 * @param out Destination buffer for resolved path.
 * @param outsz Buffer capacity.
 * @return RESOLVE_OK on success, RESOLVE_NOT_FOUND if not found, RESOLVE_ERROR on bad args.
 */
ResolveResult resolve_command(const char *cmd, char *out, size_t outsz);

/**
 * @brief Executes a built-in command with optional input/output redirection.
 *
 * Restores stdin/stdout file descriptors upon completion.
 *
 * @param command Parsed command arguments.
 * @param input_file Input redirection target file or NULL.
 * @param output_file Output redirection target file or NULL.
 * @param append_output 1 for >> (append), 0 for > (overwrite).
 * @param history Array of recorded command strings.
 * @param history_count Number of entries in history.
 * @return Result of the built-in execution.
 */
int execute_builtin(
    Command *command,
    const char *input_file,
    const char *output_file,
    int append_output,
    const char *history[],
    int history_count);

/**
 * @brief Executes an external program with optional redirection in foreground or background.
 *
 * @param path Full resolved path to the executable program.
 * @param command Parsed command arguments.
 * @param input_file Input redirection target or NULL.
 * @param output_file Output redirection target or NULL.
 * @param append_output 1 for append mode, 0 for truncate.
 * @param background 1 for background execution (&), 0 for foreground.
 */
void execute_external(
    const char *path,
    Command *command,
    const char *input_file,
    const char *output_file,
    int append_output,
    int background);

#endif /* PROCESS_EXEC_H */
