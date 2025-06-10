#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
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
    TOKEN_COMMA, // , for multi-argument functions

    // Mathematical functions
    TOKEN_SIN,
    TOKEN_COS,
    TOKEN_TAN,
    TOKEN_ASIN,
    TOKEN_ACOS,
    TOKEN_ATAN,
    TOKEN_ATAN2,
    TOKEN_SINH,
    TOKEN_COSH,
    TOKEN_TANH,
    TOKEN_ASINH,
    TOKEN_ACOSH,
    TOKEN_ATANH,
    TOKEN_SQRT,
    TOKEN_LOG,
    TOKEN_LOG10,
    TOKEN_EXP,
    TOKEN_ABS,
    TOKEN_FLOOR,
    TOKEN_CEIL,
    TOKEN_POW,

    // Mathematical constants
    TOKEN_PI,
    TOKEN_E,

    TOKEN_IDENTIFIER, // For unrecognized function names
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
        char *string_value; // For identifiers
    };
} Token;

typedef struct
{
    const char *text;
    size_t pos;
    size_t input_length;
    char current_char;
} Lexer;

// Function to check if a string is a known function or constant
typedef struct
{
    const char *name;
    TokenType token;
    int arg_count; // Number of arguments (-1 for constants)
} FunctionInfo;

void init_lexer(Lexer *lexer, const char *input);
void advance(Lexer *lexer);
char peek(Lexer *lexer);
char peek_ahead(Lexer *lexer, int offset);
void skip_whitespace(Lexer *lexer);
Token lex_number(Lexer *lexer);
Token lex_identifier(Lexer *lexer);
Token get_next_token(Lexer *lexer);
const char *token_type_str(TokenType type);
void free_token(Token *token);

// Function lookup
const FunctionInfo *lookup_function(const char *name);
int get_function_arg_count(TokenType type);
const char *get_function_name(TokenType type);

#endif