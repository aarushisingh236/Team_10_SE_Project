#ifndef SIGNALS_H
#define SIGNALS_H

/**
 * @brief Configures signal handling for the shell.
 *
 * Installs a SIGCHLD handler with SA_RESTART to reap completed background
 * child processes, and a SIGINT handler so pressing Ctrl+C does not terminate
 * the interactive shell.
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

/**
 * @brief Signal handler for SIGINT (Ctrl+C).
 *
 * @param signal_number Signal identifier.
 */
void handle_sigint(int signal_number);

#endif /* SIGNALS_H */
