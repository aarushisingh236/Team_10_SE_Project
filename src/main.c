
#define _POSIX_C_SOURCE 200809L

#include "builtins.h"
#include "command_input.h"
#include "parser.h"

#include <errno.h>
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

static void execute_external(const char *path, Command *command)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("simple-shell: fork");
        return;
    }

    if (pid == 0) {
        execv(path, command->argv);

        fprintf(stderr, "simple-shell: cannot execute '%s': %s\n",
                path, strerror(errno));
        _exit(errno == ENOENT ? 127 : 126);
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

        if (handle_builtin(&command)) {
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

        execute_external(exe_path, &command);
    }

    return 0;
}
