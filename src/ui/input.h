#ifndef INPUT_H
#define INPUT_H

/**
 * Initialize input system
 * @return 0 on success, non-zero on failure
 */
int input_init(void);

/**
 * Read a line of input from user
 * @param prompt Prompt string to display
 * @return Allocated string with user input (caller must free) or NULL on EOF/error
 */
char *input_read_line(const char *prompt);

/**
 * Add a line to input history
 * @param line Line to add to history
 */
void input_add_to_history(const char *line);

/**
 * Clear input history
 */
void input_clear_history(void);

/**
 * Save history to file
 * @param filename File to save history to
 * @return 0 on success, non-zero on failure
 */
int input_save_history(const char *filename);

/**
 * Load history from file
 * @param filename File to load history from
 * @return 0 on success, non-zero on failure
 */
int input_load_history(const char *filename);

/**
 * Set up tab completion for commands and functions
 * @param enable 1 to enable, 0 to disable
 */
void input_set_completion(int enable);

/**
 * Check if readline support is available
 * @return 1 if available, 0 otherwise
 */
int input_has_readline_support(void);

/**
 * Cleanup input system
 */
void input_cleanup(void);

#endif // INPUT_H