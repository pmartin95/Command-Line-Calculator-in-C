#ifndef COMMANDS_H
#define COMMANDS_H

typedef enum
{
    CMD_UNKNOWN,
    CMD_QUIT,
    CMD_EXIT,
    CMD_HELP,
    CMD_PRECISION,
    CMD_SET_PRECISION,
    CMD_TEST,
    CMD_CLEAR,
    CMD_HISTORY,
    CMD_VERSION
} CommandType;

typedef struct
{
    CommandType type;
    char *argument; // Optional argument (caller must free)
} Command;

/**
 * Parse a command from input string
 * @param input Input string
 * @return Parsed command (caller must free argument if present)
 */
Command commands_parse(const char *input);

/**
 * Execute a parsed command
 * @param cmd Command to execute
 * @return 0 on success, non-zero on failure
 */
int commands_execute(const Command *cmd);

/**
 * Check if input string is a command (starts with command prefix)
 * @param input Input string
 * @return 1 if command, 0 if expression
 */
int commands_is_command(const char *input);

/**
 * Print help text for all commands
 */
void commands_print_help(void);

/**
 * Print help for a specific command
 * @param cmd_name Command name
 */
void commands_print_command_help(const char *cmd_name);

/**
 * Get list of available commands for tab completion
 * @param partial Partial command to match
 * @param matches Output array of matching commands
 * @param max_matches Maximum number of matches
 * @return Number of matches found
 */
int commands_get_completions(const char *partial, char **matches, int max_matches);

#endif // COMMANDS_H