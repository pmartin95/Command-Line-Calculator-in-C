#include "commands.h"
#include "precision.h"
#include "constants.h"
#include "functions.h"
#include "formatter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Command definitions
typedef struct
{
    const char *name;
    CommandType type;
    const char *description;
    const char *usage;
} CommandDef;

static const CommandDef command_table[] = {
    {"quit", CMD_QUIT, "Exit the calculator", "quit"},
    {"exit", CMD_EXIT, "Exit the calculator", "exit"},
    {"help", CMD_HELP, "Show help information", "help [command]"},
    {"precision", CMD_PRECISION, "Show current precision", "precision"},
    {"test", CMD_TEST, "Run precision tests", "test"},
    {"clear", CMD_CLEAR, "Clear screen", "clear"},
    {"history", CMD_HISTORY, "Show command history", "history"},
    {"version", CMD_VERSION, "Show version information", "version"},
    {"mode", CMD_MODE, "Show current display mode", "mode"},
    {"scientific", CMD_SET_MODE, "Set scientific notation mode", "scientific"},
    {"normal", CMD_SET_MODE, "Set normal notation mode", "normal"},
    {NULL, CMD_UNKNOWN, NULL, NULL}};

static char *trim_whitespace(char *str);
static CommandType find_command_type(const char *name);

Command commands_parse(const char *input)
{
    Command cmd = {CMD_UNKNOWN, NULL};

    if (!input)
    {
        return cmd;
    }

    // Make a copy to work with
    char *input_copy = malloc(strlen(input) + 1);
    if (!input_copy)
    {
        return cmd;
    }
    strcpy(input_copy, input);

    // Trim whitespace
    char *trimmed = trim_whitespace(input_copy);

    // Split on first space
    char *space = strchr(trimmed, ' ');
    if (space)
    {
        *space = '\0';
        char *arg = trim_whitespace(space + 1);
        if (strlen(arg) > 0)
        {
            cmd.argument = malloc(strlen(arg) + 1);
            if (cmd.argument)
            {
                strcpy(cmd.argument, arg);
            }
        }
    }

    // Look up command
    cmd.type = find_command_type(trimmed);

    // Special case for precision setting
    if (cmd.type == CMD_PRECISION && cmd.argument)
    {
        cmd.type = CMD_SET_PRECISION;
    }
    if (cmd.type == CMD_SET_MODE)
    {
        // For "scientific" and "normal" commands, store the command name as the argument
        if (!cmd.argument)
        {
            cmd.argument = malloc(strlen(trimmed) + 1);
            if (cmd.argument)
            {
                strcpy(cmd.argument, trimmed);
            }
        }
    }
    free(input_copy);
    return cmd;
}

int commands_execute(const Command *cmd)
{
    if (!cmd)
    {
        return -1;
    }

    switch (cmd->type)
    {
    case CMD_QUIT:
    case CMD_EXIT:
        printf("Goodbye!\n");
        return 1; // Signal exit

    case CMD_HELP:
        if (cmd->argument)
        {
            commands_print_command_help(cmd->argument);
        }
        else
        {
            commands_print_help();
        }
        return 0;

    case CMD_PRECISION:
        print_precision_info();
        return 0;

    case CMD_SET_PRECISION:
        if (cmd->argument)
        {
            long new_prec = strtol(cmd->argument, NULL, 10);
            if (new_prec > 0)
            {
                set_precision((mpfr_prec_t)new_prec);
                print_precision_info();
                // Clear cached constants when precision changes
                constants_clear_cache();
            }
            else
            {
                printf("Invalid precision value: %s\n", cmd->argument);
            }
        }
        return 0;
    case CMD_MODE:
        formatter_print_current_mode();
        return 0;

    case CMD_SET_MODE:
        if (cmd->argument)
        {
            if (strcmp(cmd->argument, "scientific") == 0)
            {
                formatter_set_default_mode(FORMAT_SCIENTIFIC);
                printf("Display mode set to scientific notation\n");
            }
            else if (strcmp(cmd->argument, "normal") == 0)
            {
                formatter_set_default_mode(FORMAT_SMART);
                printf("Display mode set to normal notation\n");
            }
            else
            {
                printf("Unknown mode: %s (use 'scientific' or 'normal')\n", cmd->argument);
            }
        }
        else
        {
            printf("No mode specified. Use 'scientific' or 'normal'\n");
        }
        return 0;

    case CMD_TEST:
        printf("Testing high precision arithmetic:\n");

        mpfr_t one, small, result_test;
        mpfr_init2(one, global_precision);
        mpfr_init2(small, global_precision);
        mpfr_init2(result_test, global_precision);

        // Test 1: 1 + 1e-30
        mpfr_set_d(one, 1.0, global_rounding);
        mpfr_set_str(small, "1e-30", 10, global_rounding);
        mpfr_add(result_test, one, small, global_rounding);

        long decimal_digits = get_decimal_digits();
        if (decimal_digits < 35)
            decimal_digits = 35;

        printf("1 + 1e-30 = ");
        mpfr_printf("%.*Rf\n", (int)decimal_digits, result_test);

        // Test 2: High precision pi
        constants_get_pi(result_test);
        printf("π = ");
        mpfr_printf("%.*Rf\n", (int)decimal_digits, result_test);

        // Test 3: High precision e
        constants_get_e(result_test);
        printf("e = ");
        mpfr_printf("%.*Rf\n", (int)decimal_digits, result_test);

        mpfr_clear(one);
        mpfr_clear(small);
        mpfr_clear(result_test);

        return 0;

    case CMD_CLEAR:
        // ANSI escape sequence to clear screen
        printf("\033[2J\033[H");
        return 0;

    case CMD_HISTORY:
        printf("Command history not yet implemented\n");
        return 0;

    case CMD_VERSION:
        printf("High-Precision Calculator v1.0\n");
        printf("Built with MPFR for arbitrary precision arithmetic\n");
        printf("Supports functions, constants, and complex expressions\n");
        return 0;

    case CMD_UNKNOWN:
    default:
        printf("Unknown command. Type 'help' for available commands.\n");
        return 0;
    }
}

int commands_is_command(const char *input)
{
    if (!input)
    {
        return 0;
    }

    // Skip whitespace
    while (isspace(*input))
    {
        input++;
    }

    // Empty input is not a command
    if (*input == '\0')
    {
        return 0;
    }

    // Check if it starts with a known command
    for (int i = 0; command_table[i].name != NULL; i++)
    {
        size_t cmd_len = strlen(command_table[i].name);
        if (strncmp(input, command_table[i].name, cmd_len) == 0)
        {
            // Make sure it's followed by space or end of string
            char next_char = input[cmd_len];
            if (next_char == '\0' || isspace(next_char))
            {
                return 1;
            }
        }
    }

    return 0;
}

void commands_print_help(void)
{
    printf("High-Precision Mathematical Calculator with Function Support\n\n");

    printf("Commands:\n");
    for (int i = 0; command_table[i].name != NULL; i++)
    {
        printf("  %-10s - %s\n", command_table[i].name, command_table[i].description);
    }
    printf("  precision <bits> - Set precision (53-8192 bits)\n");
    printf("\n");

    printf("Display mode commands:\n");
    printf("  mode              -> Show current display mode\n");
    printf("  scientific        -> Always use scientific notation (1.23e+05)\n");
    printf("  normal            -> Use normal notation when appropriate\n\n");

    printf("Display mode examples:\n");
    printf("  Normal mode:      12345 -> 12345, 0.00001 -> 1e-05\n");
    printf("  Scientific mode:  12345 -> 1.2345e+04, 0.00001 -> 1e-05\n\n");

    printf("Basic operations:\n");
    printf("  2+3*4         -> 14\n");
    printf("  2(3+4)        -> 14 (implicit multiplication)\n");
    printf("  (3+4)(2+1)    -> 21\n");
    printf("  2^3^2         -> 512 (right-associative)\n");
    printf("  -5+3          -> -2\n\n");

    printf("High precision examples:\n");
    printf("  1+1e-30       -> Shows tiny differences\n");
    printf("  pi*e^100      -> Very large precise calculations\n");
    printf("  sqrt(2)       -> High precision square root\n\n");

    printf("Scientific notation:\n");
    printf("  1.5e10        -> 15000000000\n");
    printf("  2.3e-5        -> 2.3e-05\n");
    printf("  6.02e+23      -> 6.02e+23 (Avogadro's number)\n\n");

    printf("Trigonometric functions (radians):\n");
    printf("  sin(pi/2)     -> 1\n");
    printf("  cos(0)        -> 1\n");
    printf("  tan(pi/4)     -> 1\n");
    printf("  asin(1)       -> pi/2\n");
    printf("  acos(1)       -> 0\n");
    printf("  atan(1)       -> pi/4\n");
    printf("  atan2(1,1)    -> pi/4\n\n");

    printf("Hyperbolic functions:\n");
    printf("  sinh(1)       -> Hyperbolic sine\n");
    printf("  cosh(0)       -> 1\n");
    printf("  tanh(0)       -> 0\n");
    printf("  asinh(1)      -> Inverse hyperbolic sine\n");
    printf("  acosh(2)      -> Inverse hyperbolic cosine\n");
    printf("  atanh(0.5)    -> Inverse hyperbolic tangent\n\n");

    printf("Other functions:\n");
    printf("  sqrt(16)      -> 4\n");
    printf("  log(e)        -> 1 (natural log)\n");
    printf("  ln(e)         -> 1 (natural log alias)\n");
    printf("  log10(100)    -> 2\n");
    printf("  exp(1)        -> e\n");
    printf("  abs(-5)       -> 5\n");
    printf("  floor(3.7)    -> 3\n");
    printf("  ceil(3.2)     -> 4\n");
    printf("  pow(2,3)      -> 8\n\n");

    printf("Constants:\n");
    printf("  pi            -> High precision π\n");
    printf("  e             -> High precision e\n\n");

    printf("Comparison operators:\n");
    printf("  5>3           -> 1 (true)\n");
    printf("  2==2          -> 1 (true)\n");
    printf("  3!=4          -> 1 (true)\n");
    printf("  2<=2          -> 1 (true)\n");
    printf("  5>=3          -> 1 (true)\n\n");

    printf("Examples combining functions:\n");
    printf("  sin(pi/6)*2   -> 1\n");
    printf("  sqrt(pow(3,2)+pow(4,2)) -> 5\n");
    printf("  log(exp(2))   -> 2\n");
    printf("  2*pi*sqrt(2)  -> High precision result\n");
    printf("  1.5e10/3e8    -> 50 (scientific notation)\n\n");
}

void commands_print_command_help(const char *cmd_name)
{
    for (int i = 0; command_table[i].name != NULL; i++)
    {
        if (strcmp(command_table[i].name, cmd_name) == 0)
        {
            printf("Command: %s\n", command_table[i].name);
            printf("Description: %s\n", command_table[i].description);
            printf("Usage: %s\n", command_table[i].usage);
            return;
        }
    }

    // Special case for precision setting
    if (strcmp(cmd_name, "precision") == 0)
    {
        printf("Command: precision\n");
        printf("Description: Show or set calculation precision\n");
        printf("Usage: precision [bits]\n");
        printf("  precision        - Show current precision\n");
        printf("  precision 128    - Set precision to 128 bits\n");
        printf("  precision 512    - Set precision to 512 bits\n");
        printf("  precision 1024   - Set precision to 1024 bits\n");
        printf("\nValid range: %d to %d bits\n", MIN_PRECISION, MAX_PRECISION);
        printf("Note: Higher precision uses more memory and is slower\n");
        return;
    }

    printf("Unknown command: %s\n", cmd_name);
    printf("Type 'help' to see all available commands.\n");
}

int commands_get_completions(const char *partial, char **matches, int max_matches)
{
    if (!partial || !matches)
    {
        return 0;
    }

    int count = 0;
    size_t partial_len = strlen(partial);

    for (int i = 0; command_table[i].name != NULL && count < max_matches; i++)
    {
        if (strncmp(command_table[i].name, partial, partial_len) == 0)
        {
            matches[count] = malloc(strlen(command_table[i].name) + 1);
            if (matches[count])
            {
                strcpy(matches[count], command_table[i].name);
                count++;
            }
        }
    }

    return count;
}

// Helper functions
static char *trim_whitespace(char *str)
{
    if (!str)
        return str;

    // Trim leading whitespace
    while (isspace(*str))
    {
        str++;
    }

    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
    {
        *end = '\0';
        end--;
    }

    return str;
}

static CommandType find_command_type(const char *name)
{
    if (!name)
    {
        return CMD_UNKNOWN;
    }

    for (int i = 0; command_table[i].name != NULL; i++)
    {
        if (strcmp(command_table[i].name, name) == 0)
        {
            return command_table[i].type;
        }
    }

    return CMD_UNKNOWN;
}