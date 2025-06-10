#include "lexer.h"
#include <errno.h>
#include <limits.h>

// Maximum input length to prevent DoS attacks
#define MAX_INPUT_LENGTH 1024

void init_lexer(Lexer *lexer, const char *input)
{
    if (!lexer || !input) {
        if (lexer) {
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
    if (lexer->input_length > MAX_INPUT_LENGTH) {
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
    if (!lexer || lexer->pos >= lexer->input_length) {
        return;
    }
    
    lexer->pos++;
    if (lexer->pos >= lexer->input_length) {
        lexer->current_char = '\0';
    } else {
        lexer->current_char = lexer->text[lexer->pos];
    }
}

char peek(Lexer *lexer)
{
    if (!lexer || lexer->pos + 1 >= lexer->input_length) {
        return '\0';
    }
    return lexer->text[lexer->pos + 1];
}

void skip_whitespace(Lexer *lexer)
{
    if (!lexer) return;
    
    while (lexer->current_char != '\0' && isspace(lexer->current_char))
    {
        advance(lexer);
    }
}

Token lex_number(Lexer *lexer)
{
    if (!lexer) {
        return (Token){.type = TOKEN_INVALID, .int_value = 0};
    }
    
    size_t start_pos = lexer->pos;
    int has_dot = 0;
    char buffer[128];  // Increased buffer size
    size_t buf_idx = 0;
    int digit_count = 0;

    // Handle edge case: single dot without digits
    if (lexer->current_char == '.' && !isdigit(peek(lexer))) {
        return (Token){.type = TOKEN_INVALID, .int_value = 0};
    }

    while ((isdigit(lexer->current_char) || lexer->current_char == '.') && 
           lexer->current_char != '\0')
    {
        if (lexer->current_char == '.')
        {
            if (has_dot)
            {
                // Multiple dots: invalid number format
                return (Token){.type = TOKEN_INVALID, .int_value = 0};
            }

            // Check for '.' not followed by digit (unless we already have digits)
            char next = peek(lexer);
            if (!isdigit(next) && digit_count == 0) {
                return (Token){.type = TOKEN_INVALID, .int_value = 0};
            }

            has_dot = 1;
        } else {
            digit_count++;
        }

        // Prevent buffer overflow
        if (buf_idx >= sizeof(buffer) - 1) {
            return (Token){.type = TOKEN_INVALID, .int_value = 0};
        }
        
        buffer[buf_idx++] = lexer->current_char;
        advance(lexer);
    }

    // Must have at least one digit
    if (digit_count == 0) {
        return (Token){.type = TOKEN_INVALID, .int_value = 0};
    }

    buffer[buf_idx] = '\0';

    if (has_dot)
    {
        errno = 0;
        double val = strtod(buffer, NULL);
        
        // Check for overflow/underflow
        if (errno == ERANGE || isinf(val) || isnan(val)) {
            return (Token){.type = TOKEN_INVALID, .int_value = 0};
        }
        
        return (Token){.type = TOKEN_FLOAT, .float_value = val};
    }
    else
    {
        errno = 0;
        long val = strtol(buffer, NULL, 10);
        
        // Check for overflow/underflow
        if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
            return (Token){.type = TOKEN_INVALID, .int_value = 0};
        }
        
        return (Token){.type = TOKEN_INT, .int_value = (int)val};
    }
}

Token get_next_token(Lexer *lexer)
{
    if (!lexer) {
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
            if (isdigit(peek(lexer))) {
                return lex_number(lexer);
            } else {
                // Standalone dot is invalid
                advance(lexer);
                return (Token){.type = TOKEN_INVALID, .int_value = 0};
            }
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
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_INVALID:
        return "INVALID";
    default:
        return "UNKNOWN";
    }
}