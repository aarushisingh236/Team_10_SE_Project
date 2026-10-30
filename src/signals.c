#define _POSIX_C_SOURCE 200809L

#include "signals.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void handle_sigchld(int signal_number)
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

int setup_signal_handlers(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigchld;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        fprintf(stderr,
                "Error: Could not configure child process cleanup: %s\n",
                strerror(errno));
        return -1;
    }

    return 0;
}
