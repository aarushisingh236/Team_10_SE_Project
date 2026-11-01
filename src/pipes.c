#define _POSIX_C_SOURCE 200809L

#include "pipes.h"
#include "process_exec.h"
#include "errors.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int has_pipe_operator(const Command *command)
{
    int i;

    if (command == NULL) {
        return 0;
    }

    for (i = 0; i < command->argc; i++) {
        const char *token = command->argv[i];

        if (!command->quoted[i] && token[0] == '|') {
            int j = 0;

            while (token[j] == '|') {
                j++;
            }

            if (token[j] == '\0') {
                return 1;
            }
        }
    }

    return 0;
}

void execute_pipeline(Command *command)
{
    Command commands[SHELL_MAX_ARGS] = {0};
    char paths[SHELL_MAX_ARGS][EXE_PATH_MAX];
    int pipes[SHELL_MAX_ARGS - 1][2];
    pid_t pids[SHELL_MAX_ARGS];
    int command_count = 0;
    int i, j;
    int started = 0;
    sigset_t block_set, old_set;

    if (command == NULL) {
        return;
    }

    /* Split input into individual commands. */
    for (i = 0; i < command->argc; i++) {
        if (!command->quoted[i] && command->argv[i][0] == '|') {
            if (strcmp(command->argv[i], "|") != 0) {
                fputs("Error: malformed command input (invalid pipe syntax).\n",
                    stderr);
                return;
            }
            if (commands[command_count].argc == 0 ||
                command_count >= SHELL_MAX_ARGS - 1) {
                fputs("Error: malformed command input (invalid pipe syntax).\n",
                    stderr);
                return;
            }

            commands[command_count].argv[
                commands[command_count].argc] = NULL;
            command_count++;
        } else {
            if (commands[command_count].argc >= SHELL_MAX_ARGS) {
                fputs("Error: too many arguments in pipeline.\n", stderr);
                return;
            }

            int arg_index = commands[command_count].argc;

            commands[command_count].argv[arg_index] = command->argv[i];
            commands[command_count].quoted[arg_index] = command->quoted[i];
            commands[command_count].argc++;
        }
    }

    if (commands[command_count].argc == 0) {
        fputs("Error: malformed command input (invalid pipe syntax).\n",
              stderr);
        return;
    }

    commands[command_count].argv[
        commands[command_count].argc] = NULL;
    command_count++;

    /* Resolve and validate every command. */
    for (i = 0; i < command_count; i++) {
        for (j = 0; j < commands[i].argc; j++) {
            if (!commands[i].quoted[j] &&
                (strcmp(commands[i].argv[j], "<") == 0 ||
                strcmp(commands[i].argv[j], ">") == 0 ||
                strcmp(commands[i].argv[j], ">>") == 0)) {
                fputs("Error: malformed command input (invalid pipe syntax).\n",
                      stderr);
                return;
            }
        }

        ResolveResult rc = resolve_command(
            commands[i].argv[0], paths[i], sizeof(paths[i]));

        if (rc == RESOLVE_NOT_FOUND) {
            print_error_command_not_found(commands[i].argv[0]);
            return;
        }

        if (rc == RESOLVE_NOT_EXECUTABLE) {
            print_error_command_not_executable(
                commands[i].argv[0], "permission denied");
            return;
        }

        if (rc != RESOLVE_OK) {
            fprintf(stderr,
                    "Error: Could not resolve pipeline command '%s'.\n",
                    commands[i].argv[0]);
            return;
        }
    }

    /* Block SIGCHLD while managing pipeline children. */
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGCHLD);

    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        perror("sigprocmask");
        return;
    }

    /* Create pipes. */
    for (i = 0; i < command_count - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");

            for (j = 0; j < i; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            sigprocmask(SIG_SETMASK, &old_set, NULL);
            return;
        }
    }

    /* Fork each command in the pipeline. */
    for (i = 0; i < command_count; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork");
            break;
        }

        if (pids[i] == 0) {
            sigprocmask(SIG_SETMASK, &old_set, NULL);
            struct sigaction default_sa;
            memset(&default_sa, 0, sizeof(default_sa));
            default_sa.sa_handler = SIG_DFL;
            sigemptyset(&default_sa.sa_mask);

            if (sigaction(SIGINT, &default_sa, NULL) < 0) {
                _exit(1);
            }

            if (i > 0 &&
                dup2(pipes[i - 1][0], STDIN_FILENO) < 0) {
                perror("dup2 stdin");
                _exit(1);
            }

            if (i < command_count - 1 &&
                dup2(pipes[i][1], STDOUT_FILENO) < 0) {
                perror("dup2 stdout");
                _exit(1);
            }

            for (j = 0; j < command_count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(paths[i], commands[i].argv);

            {
                int exec_errno = errno;

                if (exec_errno == ENOENT) {
                    print_error_command_not_found(commands[i].argv[0]);
                } else if (exec_errno == EACCES) {
                    print_error_command_not_executable(
                        commands[i].argv[0], strerror(exec_errno));
                } else {
                    fprintf(stderr,
                            "Error: Could not execute pipeline command '%s': %s\n",
                            commands[i].argv[0], strerror(exec_errno));
                }

                _exit(exec_errno == ENOENT ? 127 : 126);
            }
        }

        started++;
    }

    /* Close parent copies of all pipe descriptors. */
    for (i = 0; i < command_count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    /* Stop already-started children if a fork failed. */
    if (started < command_count) {
        for (i = 0; i < started; i++) {
            kill(pids[i], SIGTERM);
        }
    }

    /* Wait for all pipeline processes. */
    for (i = 0; i < started; i++) {
        int status;
        pid_t result;

        do {
            result = waitpid(pids[i], &status, 0);
        } while (result < 0 && errno == EINTR);

        if (result < 0 && errno != ECHILD) {
            perror("waitpid");
        }
    }

    sigprocmask(SIG_SETMASK, &old_set, NULL);
}
