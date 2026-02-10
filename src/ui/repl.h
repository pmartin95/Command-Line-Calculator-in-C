#ifndef REPL_H
#define REPL_H

typedef enum
{
    REPL_CONTINUE, // Continue processing input
    REPL_EXIT,     // Exit the REPL
    REPL_ERROR     // Error occurred
} ReplResult;

typedef enum
{
    EVAL_MODE_NUMERIC,  // Numeric evaluation (default)
    EVAL_MODE_SYMBOLIC  // Symbolic evaluation/simplification
} EvalMode;

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

/**
 * Set evaluation mode
 * @param mode Evaluation mode to use (numeric or symbolic)
 */
void repl_set_eval_mode(EvalMode mode);

/**
 * Get current evaluation mode
 * @return Current evaluation mode
 */
EvalMode repl_get_eval_mode(void);

#endif // REPL_H