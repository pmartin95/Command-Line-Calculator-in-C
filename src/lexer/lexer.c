#include "lexer.h"
#include "function_table.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

// Maximum input length to prevent DoS attacks
#define MAX_INPUT_LENGTH 1024

static void lexer_advance(Lexer *lexer);
static void lexer_skip_whitespace(Lexer *lexer);
static Token lexer_lex_number(Lexer *lexer);
static Token lexer_lex_identifier(Lexer *lexer);

void lexer_init(Lexer *lexer, const char *input)
{
    if (!lexer || !input)
    {
        if (lexer)
        {
            lexer->text = "";
            lexer->pos = 0;
            lexer->current_char = '\0';
            lexer->input_length = 0;
        }
        return;
    }

    lexer->text = input;
    lexer->pos = 0;
    lexer->input_length = strlen(input);

    // Reject overly long input
    if (lexer->input_length > MAX_INPUT_LENGTH)
    {
        lexer->text = "";
        lexer->pos = 0;
        lexer->current_char = '\0';
        lexer->input_length = 0;
        return;
    }

    lexer->current_char = lexer->input_length > 0 ? input[0] : '\0';
}

static void lexer_advance(Lexer *lexer)
{
    if (!lexer || lexer->pos >= lexer->input_length)
    {
        return;
    }

    lexer->pos++;
    if (lexer->pos >= lexer->input_length)
    {
        lexer->current_char = '\0';
    }
    else
    {
        lexer->current_char = lexer->text[lexer->pos];
    }
}

char lexer_peek(Lexer *lexer)
{
    if (!lexer || lexer->pos + 1 >= lexer->input_length)
    {
        return '\0';
    }
    return lexer->text[lexer->pos + 1];
}

char lexer_peek_ahead(Lexer *lexer, int offset)
{
    if (!lexer || offset < 0 || lexer->pos + offset >= lexer->input_length)
    {
        return '\0';
    }
    return lexer->text[lexer->pos + offset];
}

static void lexer_skip_whitespace(Lexer *lexer)
{
    if (!lexer)
        return;

    while (lexer->current_char != '\0' && isspace(lexer->current_char))
    {
        lexer_advance(lexer);
    }
}

static Token lexer_lex_number(Lexer *lexer)
{
    if (!lexer)
    {
        return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
    }

    char buffer[256];
    size_t buf_idx = 0;
    int has_dot = 0;
    int has_exponent = 0;
    int digit_count = 0;

    // Handle edge case: single dot without digits
    if (lexer->current_char == '.' && !isdigit(lexer_peek(lexer)))
    {
        return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
    }

    // Parse the integer/fractional part
    while ((isdigit(lexer->current_char) || lexer->current_char == '.') &&
           lexer->current_char != '\0' && buf_idx < sizeof(buffer) - 1)
    {

        if (lexer->current_char == '.')
        {
            if (has_dot)
            {
                // Multiple dots: invalid
                return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
            }

            // Check for '.' not followed by digit (unless we already have digits)
            char next = lexer_peek(lexer);
            if (!isdigit(next) && digit_count == 0)
            {
                return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
            }

            has_dot = 1;
        }
        else
        {
            digit_count++;
        }

        buffer[buf_idx++] = lexer->current_char;
        lexer_advance(lexer);
    }

    // Must have at least one digit
    if (digit_count == 0)
    {
        return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
    }

    // Check for scientific notation
    if (lexer->current_char == 'e' || lexer->current_char == 'E')
    {
        char next = lexer_peek(lexer);
        char after_next = lexer_peek_ahead(lexer, 2);

        if (isdigit(next) || ((next == '+' || next == '-') && isdigit(after_next)))
        {
            has_exponent = 1;

            if (buf_idx >= sizeof(buffer) - 1)
            {
                return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
            }

            // Add the 'e' or 'E'
            buffer[buf_idx++] = lexer->current_char;
            lexer_advance(lexer);

            // Handle optional sign
            if (lexer->current_char == '+' || lexer->current_char == '-')
            {
                if (buf_idx >= sizeof(buffer) - 1)
                {
                    return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
                }
                buffer[buf_idx++] = lexer->current_char;
                lexer_advance(lexer);
            }

            // Parse exponent digits
            int exp_digit_count = 0;
            while (isdigit(lexer->current_char) && lexer->current_char != '\0' &&
                   buf_idx < sizeof(buffer) - 1)
            {
                buffer[buf_idx++] = lexer->current_char;
                exp_digit_count++;
                lexer_advance(lexer);
            }

            if (exp_digit_count == 0)
            {
                return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
            }
        }
    }

    buffer[buf_idx] = '\0';

    // Create a copy of the number string
    char *number_str = malloc(strlen(buffer) + 1);
    if (!number_str)
    {
        return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
    }
    strcpy(number_str, buffer);

    // Determine token type
    if (has_dot || has_exponent)
    {
        errno = 0;
        double val = strtod(buffer, NULL);
        if (errno == ERANGE || isinf(val) || isnan(val))
        {
            free(number_str);
            return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
        }
        return (Token){.type = TOKEN_FLOAT, .float_value = val, .number_string = number_str};
    }
    else
    {
        errno = 0;
        long val = strtol(buffer, NULL, 10);
        if (errno == ERANGE)
        {
            // Treat as float for MPFR processing
            return (Token){.type = TOKEN_FLOAT, .float_value = 0.0, .number_string = number_str};
        }
        if (val > INT_MAX || val < INT_MIN)
        {
            return (Token){.type = TOKEN_FLOAT, .float_value = (double)val, .number_string = number_str};
        }
        return (Token){.type = TOKEN_INT, .int_value = (int)val, .number_string = number_str};
    }
}

static Token lexer_lex_identifier(Lexer *lexer)
{
    if (!lexer)
    {
        return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
    }

    char buffer[64];
    size_t buf_idx = 0;

    // Read alphanumeric characters and underscores
    while ((isalnum(lexer->current_char) || lexer->current_char == '_') &&
           lexer->current_char != '\0' && buf_idx < sizeof(buffer) - 1)
    {
        buffer[buf_idx++] = lexer->current_char;
        lexer_advance(lexer);
    }

    buffer[buf_idx] = '\0';

    // Look up in function table
    const FunctionInfo *func_info = function_table_lookup(buffer);
    if (func_info)
    {
        // For constants, we need to store the name in string_value
        if (func_info->token == TOKEN_CONSTANT)
        {
            char *str_copy = malloc(strlen(buffer) + 1);
            if (!str_copy)
            {
                return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
            }
            strcpy(str_copy, buffer);
            return (Token){.type = func_info->token, .string_value = str_copy, .number_string = NULL};
        }
        return (Token){.type = func_info->token, .int_value = 0, .number_string = NULL};
    }

    // Unknown identifier - create a copy of the string
    char *str_copy = malloc(strlen(buffer) + 1);
    if (!str_copy)
    {
        return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
    }
    strcpy(str_copy, buffer);

    return (Token){.type = TOKEN_IDENTIFIER, .string_value = str_copy, .number_string = NULL};
}

Token lexer_get_next_token(Lexer *lexer)
{
    if (!lexer)
    {
        return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
    }

    while (lexer->current_char != '\0')
    {
        if (isspace(lexer->current_char))
        {
            lexer_skip_whitespace(lexer);
            continue;
        }

        // Handle numbers and standalone dots
        if (isdigit(lexer->current_char))
        {
            return lexer_lex_number(lexer);
        }

        // Handle dots that might start numbers
        if (lexer->current_char == '.')
        {
            if (isdigit(lexer_peek(lexer)))
            {
                return lexer_lex_number(lexer);
            }
            else
            {
                // Standalone dot is invalid
                lexer_advance(lexer);
                return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
            }
        }

        // Handle identifiers (function names, constants)
        if (isalpha(lexer->current_char) || lexer->current_char == '_')
        {
            return lexer_lex_identifier(lexer);
        }

        // Handle operators and punctuation
        switch (lexer->current_char)
        {
        case '+':
            lexer_advance(lexer);
            return (Token){.type = TOKEN_PLUS, .int_value = 0, .number_string = NULL};
        case '-':
            lexer_advance(lexer);
            return (Token){.type = TOKEN_MINUS, .int_value = 0, .number_string = NULL};
        case '*':
            lexer_advance(lexer);
            return (Token){.type = TOKEN_STAR, .int_value = 0, .number_string = NULL};
        case '/':
            lexer_advance(lexer);
            return (Token){.type = TOKEN_SLASH, .int_value = 0, .number_string = NULL};
        case '^':
            lexer_advance(lexer);
            return (Token){.type = TOKEN_CARET, .int_value = 0, .number_string = NULL};
        case '(':
            lexer_advance(lexer);
            return (Token){.type = TOKEN_LPAREN, .int_value = 0, .number_string = NULL};
        case ')':
            lexer_advance(lexer);
            return (Token){.type = TOKEN_RPAREN, .int_value = 0, .number_string = NULL};
        case ',':
            lexer_advance(lexer);
            return (Token){.type = TOKEN_COMMA, .int_value = 0, .number_string = NULL};
        case '=':
            if (lexer_peek(lexer) == '=')
            {
                lexer_advance(lexer);
                lexer_advance(lexer);
                return (Token){.type = TOKEN_EQ, .int_value = 0, .number_string = NULL};
            }
            // Single '=' is invalid
            lexer_advance(lexer);
            return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
        case '!':
            if (lexer_peek(lexer) == '=')
            {
                lexer_advance(lexer);
                lexer_advance(lexer);
                return (Token){.type = TOKEN_NEQ, .int_value = 0, .number_string = NULL};
            }
            // Single '!' is invalid
            lexer_advance(lexer);
            return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
        case '<':
            if (lexer_peek(lexer) == '=')
            {
                lexer_advance(lexer);
                lexer_advance(lexer);
                return (Token){.type = TOKEN_LTE, .int_value = 0, .number_string = NULL};
            }
            lexer_advance(lexer);
            return (Token){.type = TOKEN_LT, .int_value = 0, .number_string = NULL};
        case '>':
            if (lexer_peek(lexer) == '=')
            {
                lexer_advance(lexer);
                lexer_advance(lexer);
                return (Token){.type = TOKEN_GTE, .int_value = 0, .number_string = NULL};
            }
            lexer_advance(lexer);
            return (Token){.type = TOKEN_GT, .int_value = 0, .number_string = NULL};
        default:
            // Unknown character
            lexer_advance(lexer);
            return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
        }
    }

    return (Token){.type = TOKEN_EOF, .int_value = 0, .number_string = NULL};
}

int lexer_at_end(Lexer *lexer)
{
    return !lexer || lexer->current_char == '\0';
}

size_t lexer_get_position(Lexer *lexer)
{
    return lexer ? lexer->pos : 0;
}

size_t lexer_remaining_length(Lexer *lexer)
{
    if (!lexer || lexer->pos >= lexer->input_length)
    {
        return 0;
    }
    return lexer->input_length - lexer->pos;
}