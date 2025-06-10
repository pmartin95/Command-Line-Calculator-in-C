#include "input.h"
#include "commands.h"
#include "function_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

static int completion_enabled = 1;

#ifdef HAVE_READLINE
// Tab completion function for readline
static char **input_completion(const char *text, int start, int end);
static char *input_command_generator(const char *text, int state);
static char *input_function_generator(const char *text, int state);
#endif

int input_init(void)
{
#ifdef HAVE_READLINE
    // Set up readline completion
    if (completion_enabled)
    {
        rl_attempted_completion_function = input_completion;
    }

    // Load history if it exists
    input_load_history(".calculator_history");

    return 0;
#else
    return 0; // Always succeeds without readline
#endif
}

char *input_read_line(const char *prompt)
{
#ifdef HAVE_READLINE
    char *line = readline(prompt ? prompt : "> ");

    // Add non-empty lines to history
    if (line && *line)
    {
        add_history(line);
    }

    return line;
#else
    // Fallback to basic getline
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    if (prompt)
    {
        printf("%s", prompt);
        fflush(stdout);
    }

    read = getline(&line, &len, stdin);
    if (read == -1)
    {
        free(line);
        return NULL;
    }

    // Remove newline if present
    if (read > 0 && line[read - 1] == '\n')
    {
        line[read - 1] = '\0';
    }

    return line;
#endif
}

void input_add_to_history(const char *line)
{
    if (!line || !*line)
    {
        return;
    }

#ifdef HAVE_READLINE
    add_history(line);
#else
    // Without readline, we can't maintain history
    (void)line; // Suppress unused warning
#endif
}

void input_clear_history(void)
{
#ifdef HAVE_READLINE
    clear_history();
#endif
}

int input_save_history(const char *filename)
{
#ifdef HAVE_READLINE
    if (!filename)
    {
        filename = ".calculator_history";
    }
    return write_history(filename);
#else
    (void)filename; // Suppress unused warning
    return -1;      // Not supported without readline
#endif
}

int input_load_history(const char *filename)
{
#ifdef HAVE_READLINE
    if (!filename)
    {
        filename = ".calculator_history";
    }
    // read_history returns 0 on success, but we don't want to fail
    // if the history file doesn't exist yet
    read_history(filename);
    return 0;
#else
    (void)filename; // Suppress unused warning
    return -1;      // Not supported without readline
#endif
}

void input_set_completion(int enable)
{
    completion_enabled = enable;

#ifdef HAVE_READLINE
    if (enable)
    {
        rl_attempted_completion_function = input_completion;
    }
    else
    {
        rl_attempted_completion_function = NULL;
    }
#endif
}

int input_has_readline_support(void)
{
#ifdef HAVE_READLINE
    return 1;
#else
    return 0;
#endif
}

void input_cleanup(void)
{
#ifdef HAVE_READLINE
    // Save history before cleanup
    input_save_history(".calculator_history");

    // Clear history
    clear_history();
#endif
}

#ifdef HAVE_READLINE
static char **input_completion(const char *text, int start, int end)
{
    char **matches = NULL;

    (void)end; // Suppress unused warning

    // If we're at the beginning of the line, complete commands
    if (start == 0)
    {
        matches = rl_completion_matches(text, input_command_generator);
    }
    else
    {
        // Otherwise, complete function names
        matches = rl_completion_matches(text, input_function_generator);
    }

    return matches;
}

static char *input_command_generator(const char *text, int state)
{
    static char *completions[32]; // Max completions
    static int completion_count;
    static int completion_index;

    // Initialize on first call
    if (state == 0)
    {
        completion_index = 0;
        completion_count = commands_get_completions(text, completions,
                                                    sizeof(completions) / sizeof(completions[0]));
    }

    // Return next completion
    if (completion_index < completion_count)
    {
        return completions[completion_index++];
    }

    return NULL;
}

static char *input_function_generator(const char *text, int state)
{
    // Static list of function names for completion
    static const char *functions[] = {
        "sin", "cos", "tan", "asin", "acos", "atan", "atan2",
        "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
        "sqrt", "log", "ln", "log10", "exp", "abs", "floor", "ceil", "pow",
        "pi", "PI", "e", "E",
        NULL};

    static int function_index;
    static size_t text_len;

    // Initialize on first call
    if (state == 0)
    {
        function_index = 0;
        text_len = strlen(text);
    }

    // Find next matching function
    while (functions[function_index])
    {
        const char *func_name = functions[function_index++];
        if (strncmp(func_name, text, text_len) == 0)
        {
            // Return a copy of the function name
            char *match = malloc(strlen(func_name) + 1);
            if (match)
            {
                strcpy(match, func_name);
            }
            return match;
        }
    }

    return NULL;
}
#endif