#ifndef REPL_H
#define REPL_H

typedef enum
{
    REPL_CONTINUE, // Continue processing input
    REPL_EXIT,     // Exit the REPL
    REPL_ERROR     // Error occurred
} ReplResult;

/**
 * Initialize the REPL system
 * @return 0 on success, non-zero on failure
 */
int repl_init(void);

/**
 * Run the main REPL loop
 * @return Exit code
 */
int repl_run(void);

/**
 * Process a single line of input
 * @param input Input string to process
 * @return REPL result indicating next action
 */
ReplResult repl_process_line(const char *input);

/**
 * Set REPL prompt string
 * @param prompt New prompt string
 */
void repl_set_prompt(const char *prompt);

/**
 * Enable or disable command echoing
 * @param echo 1 to enable echo, 0 to disable
 */
void repl_set_echo(int echo);

/**
 * Add a line to command history
 * @param line Line to add to history
 */
void repl_add_history(const char *line);

/**
 * Clean up REPL resources
 */
void repl_cleanup(void);

#endif // REPL_H