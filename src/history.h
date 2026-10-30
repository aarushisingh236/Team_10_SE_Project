#ifndef HISTORY_H
#define HISTORY_H

#include "command_input.h"
#include <stddef.h>

#ifndef SHELL_HISTORY_CAPACITY
#define SHELL_HISTORY_CAPACITY 50
#endif

/**
 * @brief Initializes the command history module.
 */
void history_init(void);

/**
 * @brief Records a command into the bounded history.
 *
 * If the history capacity is reached, the oldest command is discarded
 * to maintain only the most recent commands in chronological order.
 * Blank commands and the 'history' command itself are excluded.
 *
 * @param line The raw input command line.
 */
void history_add(const char *line);

/**
 * @brief Returns the number of currently recorded history entries.
 *
 * @return Count of recorded commands (0 to SHELL_HISTORY_CAPACITY).
 */
int history_get_count(void);

/**
 * @brief Returns an array of pointers to the recorded command strings.
 *
 * @return Pointer to array of command string pointers.
 */
const char **history_get_entries(void);

/**
 * @brief Prints the recorded history to stdout, numbered in entry order.
 */
void history_print(void);

#endif /* HISTORY_H */
