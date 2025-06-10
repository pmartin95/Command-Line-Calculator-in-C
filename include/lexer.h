#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#ifndef __LEXER__
#define __LEXER__

typedef enum
{
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_CARET, // ^
    TOKEN_EQ,    // ==
    TOKEN_NEQ,   // !=
    TOKEN_LT,    // <
    TOKEN_LTE,   // <=
    TOKEN_GT,    // >
    TOKEN_GTE,   // >=
    TOKEN_EOF,
    TOKEN_INVALID
} TokenType;

typedef struct
{
    TokenType type;
    union
    {
        int int_value;
        double float_value;
    };

} Token;

typedef struct
{
    const char *text;
    size_t pos;
    char current_char;
} Lexer;

void init_lexer(Lexer *lexer, const char *input);
void advance(Lexer *lexer);
char peek(Lexer *lexer);
void skip_whitespace(Lexer *lexer);
Token lex_number(Lexer *lexer);
Token get_next_token(Lexer *lexer);
const char *token_type_str(TokenType type);
#endif