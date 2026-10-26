
#define _POSIX_C_SOURCE 200809L

#include "builtins.h"
#include "command_input.h"
#include "parser.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define EXE_PATH_MAX 4096
#define DEFAULT_PATH "/usr/bin:/bin"

typedef enum {
    RESOLVE_OK,
    RESOLVE_NOT_FOUND,
    RESOLVE_ERROR
} ResolveResult;

static int is_executable_file(const char *path)
{
    struct stat st;

    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) {
        return 0;
    }

    return access(path, X_OK) == 0;
}

static ResolveResult resolve_command(
    const char *cmd, char *out, size_t outsz)
{
    const char *p;
    size_t cmd_len;

    if (cmd == NULL || cmd[0] == '\0' ||
        out == NULL || outsz == 0) {
        return RESOLVE_ERROR;
    }

    cmd_len = strlen(cmd);

    if (strchr(cmd, '/') != NULL) {
        if (cmd_len + 1 > outsz) {
            return RESOLVE_ERROR;
        }

        if (!is_executable_file(cmd)) {
            return RESOLVE_NOT_FOUND;
        }

        memcpy(out, cmd, cmd_len + 1);
        return RESOLVE_OK;
    }

    p = getenv("PATH");
    if (p == NULL) {
        p = DEFAULT_PATH;
    }

    for (;;) {
        const char *end = strchr(p, ':');
        size_t dir_len = end != NULL
            ? (size_t)(end - p) : strlen(p);
        const char *dir = dir_len == 0 ? "." : p;
        size_t used_len = dir_len == 0 ? 1 : dir_len;

        if (used_len + 1 + cmd_len + 1 <= outsz) {
            memcpy(out, dir, used_len);
            out[used_len] = '/';
            memcpy(out + used_len + 1, cmd, cmd_len + 1);

            if (is_executable_file(out)) {
                return RESOLVE_OK;
            }
        }

        if (end == NULL) {
            break;
        }

        p = end + 1;
    }

    return RESOLVE_NOT_FOUND;
}

/* Extract one input redirection and remove it from argv. */
static int parse_input_redirection(
    Command *command, char **input_file)
{
    int i;
    int redirect_index = -1;

    *input_file = NULL;

    for (i = 0; i < command->argc; i++) {
        if (strcmp(command->argv[i], "<") == 0) {
            if (redirect_index != -1) {
                fprintf(stderr,
                        "Error: multiple input redirections are not supported.\n");
                return -1;
            }

            redirect_index = i;
        }
    }

    if (redirect_index == -1) {
        return 0;
    }

    if (redirect_index + 1 >= command->argc ||
        command->argv[redirect_index + 1][0] == '\0' ||
        strcmp(command->argv[redirect_index + 1], "<") == 0 ||
        strcmp(command->argv[redirect_index + 1], ">") == 0) {
        fprintf(stderr,
                "Error: input redirection requires a filename.\n");
        return -1;
    }

    *input_file = command->argv[redirect_index + 1];

    for (i = redirect_index; i + 2 < command->argc; i++) {
        command->argv[i] = command->argv[i + 2];
    }

    command->argc -= 2;
    command->argv[command->argc] = NULL;

    if (command->argc == 0) {
        fprintf(stderr,
                "Error: input redirection requires a command.\n");
        return -1;
    }

    return 0;
}

/* Extract one output redirection and remove it from argv. */
static int parse_output_redirection(
    Command *command, char **output_file)
{
    int i;
    int redirect_index = -1;

    *output_file = NULL;

    for (i = 0; i < command->argc; i++) {
        if (strcmp(command->argv[i], ">") == 0) {
            if (redirect_index != -1) {
                fprintf(stderr,
                        "Error: multiple output redirections are not supported.\n");
                return -1;
            }

            redirect_index = i;
        }
    }

    if (redirect_index == -1) {
        return 0;
    }

    if (redirect_index + 1 >= command->argc ||
        command->argv[redirect_index + 1][0] == '\0' ||
        strcmp(command->argv[redirect_index + 1], ">") == 0 ||
        strcmp(command->argv[redirect_index + 1], "<") == 0) {
        fprintf(stderr,
                "Error: output redirection requires a filename.\n");
        return -1;
    }

    *output_file = command->argv[redirect_index + 1];

    for (i = redirect_index; i + 2 < command->argc; i++) {
        command->argv[i] = command->argv[i + 2];
    }

    command->argc -= 2;
    command->argv[command->argc] = NULL;

    if (command->argc == 0) {
        fprintf(stderr,
                "Error: output redirection requires a command.\n");
        return -1;
    }

    return 0;
}

/*
 * Execute a built-in with optional redirection.
 * Save and restore the shell's standard input/output so that
 * subsequent commands and prompts continue working normally.
 */
static int execute_builtin(
    Command *command,
    const char *input_file,
    const char *output_file)
{
    int saved_stdin = -1;
    int saved_stdout = -1;
    int input_fd = -1;
    int output_fd = -1;
    int builtin_result;

    if (input_file == NULL && output_file == NULL) {
        return handle_builtin(command);
    }

    /*
     * exit is intentionally not executed when redirection is present:
     * preserving the shell process is safer for this implementation.
     */
    if (strcmp(command->argv[0], "exit") == 0) {
        fprintf(stderr,
                "Error: redirection for the exit built-in is not supported.\n");
        return 1;
    }

    if (input_file != NULL) {
        input_fd = open(input_file, O_RDONLY);
        if (input_fd < 0) {
            fprintf(stderr, "Error: cannot open input file '%s': %s\n",
                    input_file, strerror(errno));
            return 1;
        }
    }

    if (output_file != NULL) {
        output_fd = open(output_file,
                         O_WRONLY | O_CREAT | O_TRUNC,
                         0666);
        if (output_fd < 0) {
            fprintf(stderr, "Error: cannot open output file '%s': %s\n",
                    output_file, strerror(errno));
            if (input_fd >= 0) {
                close(input_fd);
            }
            return 1;
        }
    }

    if (input_fd >= 0) {
        saved_stdin = dup(STDIN_FILENO);
        if (saved_stdin < 0 || dup2(input_fd, STDIN_FILENO) < 0) {
            perror("simple-shell: redirect stdin");
            if (saved_stdin >= 0) {
                close(saved_stdin);
            }
            close(input_fd);
            if (output_fd >= 0) {
                close(output_fd);
            }
            return 1;
        }
        close(input_fd);
    }

    if (output_fd >= 0) {
        saved_stdout = dup(STDOUT_FILENO);
        if (saved_stdout < 0 || dup2(output_fd, STDOUT_FILENO) < 0) {
            perror("simple-shell: redirect stdout");

            if (saved_stdin >= 0) {
                (void)dup2(saved_stdin, STDIN_FILENO);
                close(saved_stdin);
            }
            if (saved_stdout >= 0) {
                close(saved_stdout);
            }
            close(output_fd);
            return 1;
        }
        close(output_fd);
    }

    builtin_result = handle_builtin(command);

    /*
     * Flush buffered output before restoring stdout, otherwise
     * output from echo/pwd could appear at the terminal instead.
     */
    if (fflush(stdout) == EOF) {
        perror("simple-shell: fflush");
    }

    if (saved_stdout >= 0) {
        if (dup2(saved_stdout, STDOUT_FILENO) < 0) {
            perror("simple-shell: restore stdout");
            close(saved_stdout);
            if (saved_stdin >= 0) {
                close(saved_stdin);
            }
            return 1;
        }
        close(saved_stdout);
    }

    if (saved_stdin >= 0) {
        if (dup2(saved_stdin, STDIN_FILENO) < 0) {
            perror("simple-shell: restore stdin");
            close(saved_stdin);
            return 1;
        }
        close(saved_stdin);
    }

    return builtin_result;
}

static void execute_external(
    const char *path,
    Command *command,
    const char *input_file,
    const char *output_file)
{
    int input_fd = -1;
    int output_fd = -1;

    if (input_file != NULL) {
        input_fd = open(input_file, O_RDONLY);
        if (input_fd < 0) {
            fprintf(stderr, "Error: cannot open input file '%s': %s\n",
                    input_file, strerror(errno));
            return;
        }
    }

    if (output_file != NULL) {
        output_fd = open(output_file,
                         O_WRONLY | O_CREAT | O_TRUNC,
                         0666);
        if (output_fd < 0) {
            fprintf(stderr, "Error: cannot open output file '%s': %s\n",
                    output_file, strerror(errno));
            if (input_fd >= 0) {
                close(input_fd);
            }
            return;
        }
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("simple-shell: fork");
        if (input_fd >= 0) {
            close(input_fd);
        }
        if (output_fd >= 0) {
            close(output_fd);
        }
        return;
    }

    if (pid == 0) {
        if (input_fd >= 0 &&
            dup2(input_fd, STDIN_FILENO) < 0) {
            perror("simple-shell: dup2 stdin");
            _exit(1);
        }

        if (output_fd >= 0 &&
            dup2(output_fd, STDOUT_FILENO) < 0) {
            perror("simple-shell: dup2 stdout");
            _exit(1);
        }

        if (input_fd >= 0) {
            close(input_fd);
        }
        if (output_fd >= 0) {
            close(output_fd);
        }

        execvp(path, command->argv);

        fprintf(stderr, "simple-shell: cannot execute '%s': %s\n",
                path, strerror(errno));
        _exit(errno == ENOENT ? 127 : 126);
    }

    if (input_fd >= 0) {
        close(input_fd);
    }
    if (output_fd >= 0) {
        close(output_fd);
    }

    int status;
    pid_t result;

    do {
        result = waitpid(pid, &status, 0);
    } while (result < 0 && errno == EINTR);

    if (result < 0) {
        perror("simple-shell: waitpid");
    }
}

int main(void)
{
    char line[SHELL_MAX_LINE];
    Command command;

    for (;;) {
        InputResult result = read_command_line(line, sizeof(line));

        if (result == INPUT_EOF) {
            putchar('\n');
            break;
        }

        if (result == INPUT_TOO_LONG) {
            fputs("Error: command line is too long; input was rejected.\n",
                  stderr);
            continue;
        }

        if (result == INPUT_ERROR) {
            fputs("Error: unable to read command input.\n", stderr);
            return 1;
        }

        if (parse_command(line, &command) != 0) {
            fputs("Error: malformed command input (check quotes or argument count).\n",
                  stderr);
            continue;
        }

        if (command.argc == 0) {
            continue;
        }

        char *input_file = NULL;
        char *output_file = NULL;

        if (parse_input_redirection(&command, &input_file) != 0) {
            continue;
        }

        if (parse_output_redirection(&command, &output_file) != 0) {
            continue;
        }

        if (command.argc == 0) {
            continue;
        }

        /*
         * Built-ins must run in the parent process so that cd and
         * exit retain their expected shell behavior. Redirection
         * is temporarily applied and then restored.
         */
        if (strcmp(command.argv[0], "exit") == 0 ||
            strcmp(command.argv[0], "echo") == 0 ||
            strcmp(command.argv[0], "pwd") == 0 ||
            strcmp(command.argv[0], "cd") == 0) {
            (void)execute_builtin(
                &command, input_file, output_file);
            continue;
        }

        char exe_path[EXE_PATH_MAX];

        ResolveResult rc = resolve_command(
            command.argv[0], exe_path, sizeof(exe_path));

        if (rc == RESOLVE_NOT_FOUND) {
            fprintf(stderr, "Error: command '%s' not found.\n",
                    command.argv[0]);
            continue;
        }

        if (rc != RESOLVE_OK) {
            fprintf(stderr, "Error: unable to resolve command '%s'.\n",
                    command.argv[0]);
            continue;
        }

        execute_external(
            exe_path, &command, input_file, output_file);
    }

    return 0;
}
