#ifndef SIGNALS_H
#define SIGNALS_H

/**
 * @brief Configures signal handling for the shell.
 *
 * Installs a SIGCHLD handler with SA_RESTART to reap completed background
 * child processes and prevent zombie process accumulation.
 *
 * @return 0 on success, -1 on error.
 */
int setup_signal_handlers(void);

/**
 * @brief Signal handler for SIGCHLD to reap terminated background children.
 *
 * @param signal_number Signal identifier.
 */
void handle_sigchld(int signal_number);

#endif /* SIGNALS_H */
