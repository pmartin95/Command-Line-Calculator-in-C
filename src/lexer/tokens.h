#ifndef TOKENS_H
#define TOKENS_H

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

    // Mathematical constants (unified token type)
    TOKEN_CONSTANT,

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
    char *number_string; // Store original number string for MPFR parsing
} Token;

/**
 * Get string representation of token type
 * @param type Token type
 * @return String name of the token type
 */
const char *token_type_str(TokenType type);

/**
 * Check if token represents a function
 * @param type Token type
 * @return 1 if function, 0 otherwise
 */
int token_is_function(TokenType type);

/**
 * Check if token represents a constant
 * @param type Token type
 * @return 1 if constant, 0 otherwise
 */
int token_is_constant(TokenType type);

/**
 * Check if token represents a binary operator
 * @param type Token type
 * @return 1 if binary operator, 0 otherwise
 */
int token_is_binary_op(TokenType type);

/**
 * Check if token represents a unary operator
 * @param type Token type
 * @return 1 if unary operator, 0 otherwise
 */
int token_is_unary_op(TokenType type);

/**
 * Check if token represents a comparison operator
 * @param type Token type
 * @return 1 if comparison operator, 0 otherwise
 */
int token_is_comparison_op(TokenType type);

/**
 * Get operator precedence for binary operators
 * @param type Token type
 * @return Precedence level (higher = tighter binding)
 */
int token_get_precedence(TokenType type);

/**
 * Check if operator is right-associative
 * @param type Token type
 * @return 1 if right-associative, 0 if left-associative
 */
int token_is_right_associative(TokenType type);

/**
 * Free token memory (for tokens with allocated strings)
 * @param token Token to free
 */
void token_free(Token *token);

#endif // TOKENS_H