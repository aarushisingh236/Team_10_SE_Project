#define _POSIX_C_SOURCE 200809L

#include "builtins.h"
#include "command_input.h"
#include "parser.h"
#include <signal.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

#define EXE_PATH_MAX 4096
#define DEFAULT_PATH "/usr/bin:/bin"


#define MAX_EXPANDED_ARG 4096

static int expand_environment_variables(Command *command)
{
    static char expanded[SHELL_MAX_ARGS][MAX_EXPANDED_ARG];
    int i;

    for (i = 0; i < command->argc; i++) {
        const char *src = command->argv[i];
        size_t used = 0;

        while (*src != '\0') {
            if (*src == '$' &&
                (isalpha((unsigned char)src[1]) || src[1] == '_')) {
                char name[ MAX_EXPANDED_ARG ];
                size_t name_len = 0;
                const char *value;
                size_t value_len;

                src++;

                while (isalnum((unsigned char)*src) || *src == '_') {
                    if (name_len >= sizeof(name) - 1) {
                        return -1;
                    }
                    name[name_len++] = *src++;
                }

                name[name_len] = '\0';
                value = getenv(name);

                if (value == NULL) {
                    value = "";
                }

                value_len = strlen(value);

                if (value_len >= MAX_EXPANDED_ARG - used) {
                    return -1;
                }

                memcpy(expanded[i] + used, value, value_len);
                used += value_len;
            } else {
                if (used >= MAX_EXPANDED_ARG - 1) {
                    return -1;
                }

                expanded[i][used++] = *src++;
            }
        }

        expanded[i][used] = '\0';
        command->argv[i] = expanded[i];
    }

    return 0;
}


typedef enum {
    RESOLVE_OK,
    RESOLVE_NOT_FOUND,
    RESOLVE_ERROR
} ResolveResult;

/* Detect standalone pipe operators in commands. */
static int has_pipe_operator(const Command *command)
{
    int i;

    for (i = 0; i < command->argc; i++) {
        const char *token = command->argv[i];

        if (token[0] == '|') {
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
                        "Error: Use only one input redirection (<) in a command.\n");
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
        strcmp(command->argv[redirect_index + 1], ">") == 0 ||
        strcmp(command->argv[redirect_index + 1], ">>") == 0) {
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
                "Error: Add a command before the input filename.\n");
        return -1;
    }

    return 0;
}

/*
 * Extract one output redirection.
 * '>' overwrites; '>>' appends.
 */
static int parse_output_redirection(
    Command *command,
    char **output_file,
    int *append_output)
{
    int i;
    int redirect_index = -1;

    *output_file = NULL;
    *append_output = 0;

    for (i = 0; i < command->argc; i++) {
        if (strcmp(command->argv[i], ">") == 0 ||
            strcmp(command->argv[i], ">>") == 0) {
            if (redirect_index != -1) {
                fprintf(stderr,
                        "Error: Use only one output redirection (>, or >>) in a command.\n");
                return -1;
            }

            redirect_index = i;
            *append_output =
                strcmp(command->argv[i], ">>") == 0;
        }
    }

    if (redirect_index == -1) {
        return 0;
    }

    if (redirect_index + 1 >= command->argc ||
        command->argv[redirect_index + 1][0] == '\0' ||
        strcmp(command->argv[redirect_index + 1], ">") == 0 ||
        strcmp(command->argv[redirect_index + 1], ">>") == 0 ||
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
                "Error: Add a command before the output filename.\n");
        return -1;
    }

    return 0;
}

/* Open an output file for overwrite or append. */
static int open_output_file(
    const char *output_file, int append_output)
{
    int flags = O_WRONLY | O_CREAT;

    if (append_output) {
        flags |= O_APPEND;
    } else {
        flags |= O_TRUNC;
    }

    return open(output_file, flags, 0666);
}

/*
 * Execute a built-in with optional redirection.
 * Restore standard input/output after the built-in finishes.
 */
static int execute_builtin(
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
            fprintf(stderr,
                    "Error: cannot open input file '%s': %s\n",
                    input_file, strerror(errno));
            return 1;
        }
    }

    if (output_file != NULL) {
        output_fd = open_output_file(output_file, append_output);

        if (output_fd < 0) {
            fprintf(stderr,
                    "Error: cannot open output file '%s': %s\n",
                    output_file, strerror(errno));

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

/* Reap completed child processes to prevent zombie accumulation. */
static void handle_sigchld(int signal_number)
{
    int saved_errno = errno;
    pid_t result;

    (void)signal_number;

    for (;;) {
        result = waitpid(-1, NULL, WNOHANG);

        if (result > 0) {
            continue;
        }

        if (result < 0 && errno == EINTR) {
            continue;
        }

        break;
    }

    errno = saved_errno;
}

static void execute_external(
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
            fprintf(stderr,
                    "Error: cannot open input file '%s': %s\n",
                    input_file, strerror(errno));
            return;
        }
    }

    if (output_file != NULL) {
        output_fd = open_output_file(output_file, append_output);

        if (output_fd < 0) {
            fprintf(stderr,
                    "Error: cannot open output file '%s': %s\n",
                    output_file, strerror(errno));

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



static void execute_pipeline(Command *command)
{
    Command commands[SHELL_MAX_ARGS] = {0};
    char paths[SHELL_MAX_ARGS][EXE_PATH_MAX];
    int pipes[SHELL_MAX_ARGS - 1][2];
    pid_t pids[SHELL_MAX_ARGS];
    int command_count = 0;
    int i, j;
    int started = 0;
    sigset_t block_set, old_set;

    /* Split input into individual commands. */
    for (i = 0; i < command->argc; i++) {
        if (command->argv[i][0] == '|') {
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

            commands[command_count].argv[
                commands[command_count].argc++] = command->argv[i];
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
            if (strcmp(commands[i].argv[j], "<") == 0 ||
                strcmp(commands[i].argv[j], ">") == 0 ||
                strcmp(commands[i].argv[j], ">>") == 0) {
                fputs("Error: malformed command input (invalid pipe syntax).\n",
                      stderr);
                return;
            }
        }

        if (resolve_command(commands[i].argv[0], paths[i],
                            sizeof(paths[i])) != RESOLVE_OK) {
            fprintf(stderr, "Error: command not found in pipeline: %s\n",
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
            perror("execvp");
            _exit(127);
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

int main(void)
{
    char line[SHELL_MAX_LINE];
    char original_line[SHELL_MAX_LINE];
    Command command;
    char history[1000][SHELL_MAX_LINE];
    const char *history_ptrs[1000];
    int history_count = 0;

    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigchld;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        fprintf(stderr,
                "Error: Could not configure child process cleanup: %s\n",
                strerror(errno));
        return 1;
    }

    for (;;) {
        InputResult result = read_command_line(line, sizeof(line));

        if (result == INPUT_EOF) {
            putchar('\n');
            break;
        }

        strncpy(original_line, line, SHELL_MAX_LINE - 1);
        original_line[SHELL_MAX_LINE - 1] = '\0';

        if (result == INPUT_TOO_LONG) {
            fputs(
                "Error: command line is too long. Shorten it and try again.\n",
                stderr);
            continue;
        }

        if (result == INPUT_ERROR) {
            fputs(
                "Error: Could not read your command. Please try again.\n",
                stderr);
            return 1;
        }

        if (parse_command(line, &command) != 0) {
            fputs(
                "Error: malformed command input. Check your quotes and number of arguments.\n",
                stderr);
            continue;
        }

        if (command.argc == 0) {
            continue;
        }
        if (expand_environment_variables(&command) != 0) {
            fputs("Error: environment variable expansion failed.\n", stderr);
            continue;
        }
        if (strcmp(command.argv[0], "history") != 0 &&
            history_count < 1000) {
            strncpy(history[history_count], original_line, SHELL_MAX_LINE - 1);
            history[history_count][SHELL_MAX_LINE - 1] = '\0';
            history_ptrs[history_count] = history[history_count];
            history_count++;
        }

        if (has_pipe_operator(&command)) {
            execute_pipeline(&command);
            continue;
        }

        char *input_file = NULL;
        char *output_file = NULL;
        int append_output = 0;
        int background = 0;

        /* Detect a trailing '&' to request background execution. */
        if (command.argc > 0 &&
            strcmp(command.argv[command.argc - 1], "&") == 0) {
            background = 1;
            command.argc--;
            command.argv[command.argc] = NULL;

            if (command.argc == 0) {
                fputs("Error: Add a command before '&'.\n", stderr);
                continue;
            }
        }

        if (parse_input_redirection(&command, &input_file) != 0) {
            continue;
        }

        if (parse_output_redirection(
                &command, &output_file, &append_output) != 0) {
            continue;
        }

        if (command.argc == 0) {
            continue;
        }
 if (strcmp(command.argv[0], "exit") == 0 ||
    strcmp(command.argv[0], "echo") == 0 ||
    strcmp(command.argv[0], "pwd") == 0 ||
    strcmp(command.argv[0], "help") == 0 ||
    strcmp(command.argv[0], "history") == 0 ||
    strcmp(command.argv[0], "cd") == 0) {
    (void)execute_builtin(
        &command, input_file, output_file, append_output,
        history_ptrs, history_count);
    continue;
}

        char exe_path[EXE_PATH_MAX];

        ResolveResult rc = resolve_command(
            command.argv[0], exe_path, sizeof(exe_path));

        if (rc == RESOLVE_NOT_FOUND) {
            fprintf(stderr,
                    "Error: The command '%s' was not found. Check its spelling.\n",
                    command.argv[0]);
            continue;
        }

        if (rc != RESOLVE_OK) {
            fprintf(stderr,
                    "Error: Could not locate the program '%s' because of an internal error.\n",
                    command.argv[0]);
            continue;
        }

        execute_external(
            exe_path, &command, input_file, output_file, append_output,
            background);
    }

    return 0;
}
