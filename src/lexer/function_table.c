#include "function_table.h"
#include <string.h>
#include <stddef.h>

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
    {"ln2", TOKEN_LN2, -1},
    {"LN2", TOKEN_LN2, -1},
    {"ln10", TOKEN_LN10, -1},
    {"LN10", TOKEN_LN10, -1},
    {"gamma", TOKEN_GAMMA, -1},
    {"GAMMA", TOKEN_GAMMA, -1},

    {NULL, TOKEN_INVALID, 0} // Sentinel
};

void function_table_init(void)
{
    // No dynamic initialization needed for static table
}

const FunctionInfo *function_table_lookup(const char *name)
{
    if (!name) {
        return NULL;
    }

    for (int i = 0; function_table[i].name != NULL; i++) {
        if (strcmp(function_table[i].name, name) == 0) {
            return &function_table[i];
        }
    }
    return NULL;
}

int function_table_get_arg_count(TokenType type)
{
    for (int i = 0; function_table[i].name != NULL; i++) {
        if (function_table[i].token == type) {
            return function_table[i].arg_count;
        }
    }
    return 0;
}

const char *function_table_get_name(TokenType type)
{
    for (int i = 0; function_table[i].name != NULL; i++) {
        if (function_table[i].token == type) {
            return function_table[i].name;
        }
    }
    return "unknown";
}

int function_table_needs_parentheses(TokenType type)
{
    // Constants don't need parentheses, functions do
    const FunctionInfo *info = NULL;
    for (int i = 0; function_table[i].name != NULL; i++) {
        if (function_table[i].token == type) {
            info = &function_table[i];
            break;
        }
    }
    
    if (!info) {
        return 0;
    }
    
    // If arg_count is -1, it's a constant
    return info->arg_count >= 0;
}