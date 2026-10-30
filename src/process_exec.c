#define _POSIX_C_SOURCE 200809L

#include "process_exec.h"
#include "builtins.h"
#include "environment.h"
#include "errors.h"
#include "redirection.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int is_executable_file(const char *path)
{
    struct stat st;

    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) {
        return 0;
    }

    return access(path, X_OK) == 0;
}

ResolveResult resolve_command(
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

    p = environment_get_path();

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

int execute_builtin(
    Command *command,
    const char *input_file,
    const char *output_file,
    int append_output,
    const char *history[],
    int history_count)
{
    int saved_stdin = -1;
    int saved_stdout = -1;
    int input_fd = -1;
    int output_fd = -1;
    int builtin_result;

    if (input_file == NULL && output_file == NULL) {
        return handle_builtin(command, history, history_count);
    }

    if (strcmp(command->argv[0], "exit") == 0) {
        fprintf(stderr,
                "Error: The exit command cannot be used with input or output redirection.\n");
        return 1;
    }

    if (input_file != NULL) {
        input_fd = open(input_file, O_RDONLY);

        if (input_fd < 0) {
            print_error_input_file(input_file, strerror(errno));
            return 1;
        }
    }

    if (output_file != NULL) {
        output_fd = open_output_file(output_file, append_output);

        if (output_fd < 0) {
            print_error_output_file(output_file, strerror(errno));

            if (input_fd >= 0) {
                close(input_fd);
            }

            return 1;
        }
    }

    if (input_fd >= 0) {
        saved_stdin = dup(STDIN_FILENO);

        if (saved_stdin < 0 ||
            dup2(input_fd, STDIN_FILENO) < 0) {
            fprintf(stderr,
                    "Error: Could not redirect input: %s\n",
                    strerror(errno));

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

        if (saved_stdout < 0 ||
            dup2(output_fd, STDOUT_FILENO) < 0) {
            fprintf(stderr,
                    "Error: Could not redirect output: %s\n",
                    strerror(errno));

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

    builtin_result = handle_builtin(command, history, history_count);

    if (fflush(stdout) == EOF) {
        fprintf(stderr,
                "Error: Could not finish writing command output: %s\n",
                strerror(errno));
    }

    if (saved_stdout >= 0) {
        if (dup2(saved_stdout, STDOUT_FILENO) < 0) {
            fprintf(stderr,
                    "Error: Could not restore normal output: %s\n",
                    strerror(errno));
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
            fprintf(stderr,
                    "Error: Could not restore normal input: %s\n",
                    strerror(errno));
            close(saved_stdin);
            return 1;
        }

        close(saved_stdin);
    }

    return builtin_result;
}

void execute_external(
    const char *path,
    Command *command,
    const char *input_file,
    const char *output_file,
    int append_output,
    int background)
{
    int input_fd = -1;
    int output_fd = -1;

    if (input_file != NULL) {
        input_fd = open(input_file, O_RDONLY);

        if (input_fd < 0) {
            print_error_input_file(input_file, strerror(errno));
            return;
        }
    }

    if (output_file != NULL) {
        output_fd = open_output_file(output_file, append_output);

        if (output_fd < 0) {
            print_error_output_file(output_file, strerror(errno));

            if (input_fd >= 0) {
                close(input_fd);
            }

            return;
        }
    }

    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr,
                "Error: Could not start the command: %s\n",
                strerror(errno));

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
            fprintf(stderr,
                    "Error: Could not set up command input: %s\n",
                    strerror(errno));
            _exit(1);
        }

        if (output_fd >= 0 &&
            dup2(output_fd, STDOUT_FILENO) < 0) {
            fprintf(stderr,
                    "Error: Could not set up command output: %s\n",
                    strerror(errno));
            _exit(1);
        }

        if (input_fd >= 0) {
            close(input_fd);
        }

        if (output_fd >= 0) {
            close(output_fd);
        }

        execvp(path, command->argv);

        {
            int exec_errno = errno;

            fprintf(stderr,
                    "Error: Could not run the program '%s': %s\n",
                    path, strerror(exec_errno));
            _exit(exec_errno == ENOENT ? 127 : 126);
        }
    }

    if (input_fd >= 0) {
        close(input_fd);
    }

    if (output_fd >= 0) {
        close(output_fd);
    }

    if (background) {
        printf("[Background process started: PID %ld]\n", (long)pid);
        return;
    }

    int status;
    pid_t result;

    do {
        result = waitpid(pid, &status, 0);
    } while (result < 0 && errno == EINTR);

    if (result < 0 && errno != ECHILD) {
        fprintf(stderr,
                "Error: Could not wait for the command to finish: %s\n",
                strerror(errno));
    }
}
