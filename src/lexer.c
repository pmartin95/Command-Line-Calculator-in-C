#include "lexer.h"
#include <errno.h>
#include <limits.h>

// Maximum input length to prevent DoS attacks
#define MAX_INPUT_LENGTH 1024

// Function and constant lookup table
static const FunctionInfo function_table[] = {
    // Trigonometric functions
    {"sin", TOKEN_SIN, 1},
    {"cos", TOKEN_COS, 1},
    {"tan", TOKEN_TAN, 1},

    // Inverse trigonometric functions
    {"asin", TOKEN_ASIN, 1},
    {"arcsin", TOKEN_ASIN, 1},
    {"acos", TOKEN_ACOS, 1},
    {"arccos", TOKEN_ACOS, 1},
    {"atan", TOKEN_ATAN, 1},
    {"arctan", TOKEN_ATAN, 1},
    {"atan2", TOKEN_ATAN2, 2},
    {"arctan2", TOKEN_ATAN2, 2},

    // Hyperbolic functions
    {"sinh", TOKEN_SINH, 1},
    {"cosh", TOKEN_COSH, 1},
    {"tanh", TOKEN_TANH, 1},

    // Inverse hyperbolic functions
    {"asinh", TOKEN_ASINH, 1},
    {"arcsinh", TOKEN_ASINH, 1},
    {"acosh", TOKEN_ACOSH, 1},
    {"arccosh", TOKEN_ACOSH, 1},
    {"atanh", TOKEN_ATANH, 1},
    {"arctanh", TOKEN_ATANH, 1},

    // Other mathematical functions
    {"sqrt", TOKEN_SQRT, 1},
    {"log", TOKEN_LOG, 1},     // Natural logarithm
    {"ln", TOKEN_LOG, 1},      // Natural logarithm (alias)
    {"log10", TOKEN_LOG10, 1}, // Base-10 logarithm
    {"exp", TOKEN_EXP, 1},
    {"abs", TOKEN_ABS, 1},
    {"floor", TOKEN_FLOOR, 1},
    {"ceil", TOKEN_CEIL, 1},
    {"pow", TOKEN_POW, 2},

    // Mathematical constants
    {"pi", TOKEN_PI, -1},
    {"PI", TOKEN_PI, -1},
    {"e", TOKEN_E, -1},
    {"E", TOKEN_E, -1},

    {NULL, TOKEN_INVALID, 0} // Sentinel
};

const FunctionInfo *lookup_function(const char *name)
{
    if (!name)
        return NULL;

    for (int i = 0; function_table[i].name != NULL; i++)
    {
        if (strcmp(function_table[i].name, name) == 0)
        {
            return &function_table[i];
        }
    }
    return NULL;
}

int get_function_arg_count(TokenType type)
{
    for (int i = 0; function_table[i].name != NULL; i++)
    {
        if (function_table[i].token == type)
        {
            return function_table[i].arg_count;
        }
    }
    return 0;
}

const char *get_function_name(TokenType type)
{
    for (int i = 0; function_table[i].name != NULL; i++)
    {
        if (function_table[i].token == type)
        {
            return function_table[i].name;
        }
    }
    return "unknown";
}

void init_lexer(Lexer *lexer, const char *input)
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

    lexer->current_char = input[0];
}

void advance(Lexer *lexer)
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

char peek(Lexer *lexer)
{
    if (!lexer || lexer->pos + 1 >= lexer->input_length)
    {
        return '\0';
    }
    return lexer->text[lexer->pos + 1];
}

char peek_ahead(Lexer *lexer, int offset)
{
    if (!lexer || lexer->pos + offset >= lexer->input_length)
    {
        return '\0';
    }
    return lexer->text[lexer->pos + offset];
}

void skip_whitespace(Lexer *lexer)
{
    if (!lexer)
        return;

    while (lexer->current_char != '\0' && isspace(lexer->current_char))
    {
        advance(lexer);
    }
}

Token lex_number(Lexer *lexer)
{
    if (!lexer)
    {
        return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
    }

    size_t start_pos = lexer->pos;
    int has_dot = 0;
    int has_exponent = 0;
    char buffer[256]; // Increased buffer size for high precision
    size_t buf_idx = 0;
    int digit_count = 0;

    // Handle edge case: single dot without digits
    if (lexer->current_char == '.' && !isdigit(peek(lexer)))
    {
        return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
    }

    // Parse the integer/fractional part
    while ((isdigit(lexer->current_char) || lexer->current_char == '.') &&
           lexer->current_char != '\0')
    {
        if (lexer->current_char == '.')
        {
            if (has_dot)
            {
                // Multiple dots: invalid number format
                return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
            }

            // Check for '.' not followed by digit (unless we already have digits)
            char next = peek(lexer);
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

        // Prevent buffer overflow
        if (buf_idx >= sizeof(buffer) - 1)
        {
            return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
        }

        buffer[buf_idx++] = lexer->current_char;
        advance(lexer);
    }

    // Must have at least one digit so far
    if (digit_count == 0)
    {
        return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
    }

    // Check for scientific notation (e or E)
    if (lexer->current_char == 'e' || lexer->current_char == 'E')
    {
        char next = peek(lexer);
        char after_next = peek_ahead(lexer, 2);

        // Valid exponent patterns: e123, e+123, e-123
        if (isdigit(next) ||
            ((next == '+' || next == '-') && isdigit(after_next)))
        {

            has_exponent = 1;

            // Prevent buffer overflow
            if (buf_idx >= sizeof(buffer) - 1)
            {
                return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
            }

            // Add the 'e' or 'E'
            buffer[buf_idx++] = lexer->current_char;
            advance(lexer);

            // Handle optional sign
            if (lexer->current_char == '+' || lexer->current_char == '-')
            {
                if (buf_idx >= sizeof(buffer) - 1)
                {
                    return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
                }
                buffer[buf_idx++] = lexer->current_char;
                advance(lexer);
            }

            // Parse exponent digits
            int exp_digit_count = 0;
            while (isdigit(lexer->current_char) && lexer->current_char != '\0')
            {
                if (buf_idx >= sizeof(buffer) - 1)
                {
                    return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
                }
                buffer[buf_idx++] = lexer->current_char;
                exp_digit_count++;
                advance(lexer);
            }

            // Must have at least one digit in exponent
            if (exp_digit_count == 0)
            {
                return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
            }
        }
    }

    buffer[buf_idx] = '\0';

    // Create a copy of the number string for MPFR
    char *number_str = malloc(strlen(buffer) + 1);
    if (!number_str)
    {
        return (Token){.type = TOKEN_INVALID, .int_value = 0, .number_string = NULL};
    }
    strcpy(number_str, buffer);

    // If we have a decimal point or exponent, it's a float
    if (has_dot || has_exponent)
    {
        errno = 0;
        double val = strtod(buffer, NULL);

        // Check for overflow/underflow in double (for backwards compatibility)
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

        // For very large integers, we'll still store as string for MPFR
        if (errno == ERANGE)
        {
            // Treat as float for MPFR processing
            return (Token){.type = TOKEN_FLOAT, .float_value = 0.0, .number_string = number_str};
        }

        // Check if it fits in int for backwards compatibility
        if (val > INT_MAX || val < INT_MIN)
        {
            return (Token){.type = TOKEN_FLOAT, .float_value = (double)val, .number_string = number_str};
        }

        return (Token){.type = TOKEN_INT, .int_value = (int)val, .number_string = number_str};
    }
}

Token lex_identifier(Lexer *lexer)
{
    if (!lexer)
    {
        return (Token){.type = TOKEN_INVALID, .int_value = 0};
    }

    char buffer[64];
    size_t buf_idx = 0;

    // Read alphanumeric characters and underscores
    while ((isalnum(lexer->current_char) || lexer->current_char == '_') &&
           lexer->current_char != '\0' && buf_idx < sizeof(buffer) - 1)
    {
        buffer[buf_idx++] = lexer->current_char;
        advance(lexer);
    }

    buffer[buf_idx] = '\0';

    // Look up the identifier in the function table
    const FunctionInfo *func_info = lookup_function(buffer);
    if (func_info)
    {
        return (Token){.type = func_info->token, .int_value = 0};
    }

    // Unknown identifier - create a copy of the string
    char *str_copy = malloc(strlen(buffer) + 1);
    if (!str_copy)
    {
        return (Token){.type = TOKEN_INVALID, .int_value = 0};
    }
    strcpy(str_copy, buffer);

    return (Token){.type = TOKEN_IDENTIFIER, .string_value = str_copy};
}

void free_token(Token *token)
{
    if (token && token->type == TOKEN_IDENTIFIER && token->string_value)
    {
        free(token->string_value);
        token->string_value = NULL;
    }
}

Token get_next_token(Lexer *lexer)
{
    if (!lexer)
    {
        return (Token){.type = TOKEN_INVALID, .int_value = 0};
    }

    while (lexer->current_char != '\0')
    {
        if (isspace(lexer->current_char))
        {
            skip_whitespace(lexer);
            continue;
        }

        // Handle numbers and standalone dots
        if (isdigit(lexer->current_char))
        {
            return lex_number(lexer);
        }

        // Handle dots that might start numbers
        if (lexer->current_char == '.')
        {
            if (isdigit(peek(lexer)))
            {
                return lex_number(lexer);
            }
            else
            {
                // Standalone dot is invalid
                advance(lexer);
                return (Token){.type = TOKEN_INVALID, .int_value = 0};
            }
        }

        // Handle identifiers (function names, constants)
        if (isalpha(lexer->current_char) || lexer->current_char == '_')
        {
            return lex_identifier(lexer);
        }

        switch (lexer->current_char)
        {
        case '+':
            advance(lexer);
            return (Token){.type = TOKEN_PLUS, .int_value = 0};
        case '-':
            advance(lexer);
            return (Token){.type = TOKEN_MINUS, .int_value = 0};
        case '*':
            advance(lexer);
            return (Token){.type = TOKEN_STAR, .int_value = 0};
        case '/':
            advance(lexer);
            return (Token){.type = TOKEN_SLASH, .int_value = 0};
        case '^':
            advance(lexer);
            return (Token){.type = TOKEN_CARET, .int_value = 0};
        case '(':
            advance(lexer);
            return (Token){.type = TOKEN_LPAREN, .int_value = 0};
        case ')':
            advance(lexer);
            return (Token){.type = TOKEN_RPAREN, .int_value = 0};
        case ',':
            advance(lexer);
            return (Token){.type = TOKEN_COMMA, .int_value = 0};
        case '=':
            if (peek(lexer) == '=')
            {
                advance(lexer);
                advance(lexer);
                return (Token){.type = TOKEN_EQ, .int_value = 0};
            }
            // Single '=' is invalid
            advance(lexer);
            return (Token){.type = TOKEN_INVALID, .int_value = 0};
        case '!':
            if (peek(lexer) == '=')
            {
                advance(lexer);
                advance(lexer);
                return (Token){.type = TOKEN_NEQ, .int_value = 0};
            }
            // Single '!' is invalid
            advance(lexer);
            return (Token){.type = TOKEN_INVALID, .int_value = 0};
        case '<':
            if (peek(lexer) == '=')
            {
                advance(lexer);
                advance(lexer);
                return (Token){.type = TOKEN_LTE, .int_value = 0};
            }
            advance(lexer);
            return (Token){.type = TOKEN_LT, .int_value = 0};
        case '>':
            if (peek(lexer) == '=')
            {
                advance(lexer);
                advance(lexer);
                return (Token){.type = TOKEN_GTE, .int_value = 0};
            }
            advance(lexer);
            return (Token){.type = TOKEN_GT, .int_value = 0};
        default:
            // Unknown character
            advance(lexer);
            return (Token){.type = TOKEN_INVALID, .int_value = 0};
        }
    }

    return (Token){.type = TOKEN_EOF, .int_value = 0};
}

const char *token_type_str(TokenType type)
{
    switch (type)
    {
    case TOKEN_INT:
        return "INT";
    case TOKEN_FLOAT:
        return "FLOAT";
    case TOKEN_PLUS:
        return "PLUS";
    case TOKEN_MINUS:
        return "MINUS";
    case TOKEN_STAR:
        return "STAR";
    case TOKEN_SLASH:
        return "SLASH";
    case TOKEN_LPAREN:
        return "LPAREN";
    case TOKEN_RPAREN:
        return "RPAREN";
    case TOKEN_CARET:
        return "CARET";
    case TOKEN_EQ:
        return "EQ";
    case TOKEN_NEQ:
        return "NEQ";
    case TOKEN_LT:
        return "LT";
    case TOKEN_LTE:
        return "LTE";
    case TOKEN_GT:
        return "GT";
    case TOKEN_GTE:
        return "GTE";
    case TOKEN_COMMA:
        return "COMMA";
    case TOKEN_SIN:
        return "SIN";
    case TOKEN_COS:
        return "COS";
    case TOKEN_TAN:
        return "TAN";
    case TOKEN_ASIN:
        return "ASIN";
    case TOKEN_ACOS:
        return "ACOS";
    case TOKEN_ATAN:
        return "ATAN";
    case TOKEN_ATAN2:
        return "ATAN2";
    case TOKEN_SINH:
        return "SINH";
    case TOKEN_COSH:
        return "COSH";
    case TOKEN_TANH:
        return "TANH";
    case TOKEN_ASINH:
        return "ASINH";
    case TOKEN_ACOSH:
        return "ACOSH";
    case TOKEN_ATANH:
        return "ATANH";
    case TOKEN_SQRT:
        return "SQRT";
    case TOKEN_LOG:
        return "LOG";
    case TOKEN_LOG10:
        return "LOG10";
    case TOKEN_EXP:
        return "EXP";
    case TOKEN_ABS:
        return "ABS";
    case TOKEN_FLOOR:
        return "FLOOR";
    case TOKEN_CEIL:
        return "CEIL";
    case TOKEN_POW:
        return "POW";
    case TOKEN_PI:
        return "PI";
    case TOKEN_E:
        return "E";
    case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_INVALID:
        return "INVALID";
    default:
        return "UNKNOWN";
    }
}