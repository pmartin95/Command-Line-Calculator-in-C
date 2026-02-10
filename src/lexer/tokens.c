#include "tokens.h"
#include <stdlib.h>

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
    case TOKEN_CONSTANT:
        return "CONSTANT";
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

int token_is_function(TokenType type)
{
    return (type >= TOKEN_SIN && type <= TOKEN_POW);
}

int token_is_constant(TokenType type)
{
    return (type == TOKEN_CONSTANT);
}

int token_is_binary_op(TokenType type)
{
    switch (type)
    {
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_STAR:
    case TOKEN_SLASH:
    case TOKEN_CARET:
        return 1;
    default:
        return 0;
    }
}

int token_is_unary_op(TokenType type)
{
    return (type == TOKEN_PLUS || type == TOKEN_MINUS);
}

int token_is_comparison_op(TokenType type)
{
    switch (type)
    {
    case TOKEN_EQ:
    case TOKEN_NEQ:
    case TOKEN_LT:
    case TOKEN_LTE:
    case TOKEN_GT:
    case TOKEN_GTE:
        return 1;
    default:
        return 0;
    }
}

int token_get_precedence(TokenType type)
{
    switch (type)
    {
    case TOKEN_EQ:
    case TOKEN_NEQ:
    case TOKEN_LT:
    case TOKEN_LTE:
    case TOKEN_GT:
    case TOKEN_GTE:
        return 1; // Lowest precedence
    case TOKEN_PLUS:
    case TOKEN_MINUS:
        return 2;
    case TOKEN_STAR:
    case TOKEN_SLASH:
        return 3;
    case TOKEN_CARET:
        return 4; // Highest precedence
    default:
        return 0; // No precedence
    }
}

int token_is_right_associative(TokenType type)
{
    return (type == TOKEN_CARET); // Only power operator is right-associative
}

void token_free(Token *token)
{
    if (token && (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_CONSTANT) && token->string_value)
    {
        free(token->string_value);
        token->string_value = NULL;
    }
    if (token && token->number_string)
    {
        free(token->number_string);
        token->number_string = NULL;
    }
}