#include "lexer.h"

void init_lexer(Lexer *lexer, const char *input)
{
    lexer->text = input;
    lexer->pos = 0;
    lexer->current_char = input[0];
}

void advance(Lexer *lexer)
{
    lexer->pos++;
    lexer->current_char = lexer->text[lexer->pos];
}

char peek(Lexer *lexer)
{
    return lexer->text[lexer->pos + 1];
}

void skip_whitespace(Lexer *lexer)
{
    while (isspace(lexer->current_char))
    {
        advance(lexer);
    }
}

Token lex_number(Lexer *lexer)
{
    size_t start_pos = lexer->pos;
    int has_dot = 0;
    char buffer[64];
    size_t buf_idx = 0;

    while (isdigit(lexer->current_char) || lexer->current_char == '.')
    {
        if (lexer->current_char == '.')
        {
            if (has_dot)
            {
                // Multiple dots: invalid number format
                return (Token){.type = TOKEN_INVALID, .int_value = 0};
            }

            // Check for '.' not followed by digit
            char next = lexer->text[lexer->pos + 1];
            if (!isdigit(next))
            {
                return (Token){.type = TOKEN_INVALID, .int_value = 0};
            }

            has_dot = 1;
        }

        if (buf_idx < sizeof(buffer) - 1)
        {
            buffer[buf_idx++] = lexer->current_char;
        }

        advance(lexer);
    }

    buffer[buf_idx] = '\0';

    if (has_dot)
    {
        return (Token){.type = TOKEN_FLOAT, .float_value = atof(buffer)};
    }
    else
    {
        return (Token){.type = TOKEN_INT, .int_value = atoi(buffer)};
    }
}

Token get_next_token(Lexer *lexer)
{
    while (lexer->current_char != '\0')
    {
        if (isspace(lexer->current_char))
        {
            skip_whitespace(lexer);
            continue;
        }

        if (isdigit(lexer->current_char) || lexer->current_char == '.')
        {
            return lex_number(lexer);
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
            break; // Fall through to INVALID
        case '!':
            if (peek(lexer) == '=')
            {
                advance(lexer);
                advance(lexer);
                return (Token){.type = TOKEN_NEQ, .int_value = 0};
            }
            break;
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
        }

        // If we got here, it was an unknown token
        advance(lexer);
        return (Token){.type = TOKEN_INVALID, .int_value = 0};
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
