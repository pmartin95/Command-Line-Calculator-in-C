#ifndef FUNCTION_TABLE_H
#define FUNCTION_TABLE_H

#include "tokens.h"

typedef struct
{
    const char *name;
    TokenType token;
    int arg_count; // Number of arguments (-1 for constants)
} FunctionInfo;

/**
 * Initialize function lookup table
 */
void function_table_init(void);

/**
 * Look up function or constant by name
 * @param name Function/constant name
 * @return Function info or NULL if not found
 */
const FunctionInfo *function_table_lookup(const char *name);

/**
 * Get argument count for a function token
 * @param type Function token type
 * @return Number of arguments, or 0 if not a function
 */
int function_table_get_arg_count(TokenType type);

/**
 * Get function name from token type
 * @param type Function token type
 * @return Function name or "unknown"
 */
const char *function_table_get_name(TokenType type);

/**
 * Check if a token type represents a function that needs parentheses
 * @param type Token type
 * @return 1 if function needs parentheses, 0 otherwise
 */
int function_table_needs_parentheses(TokenType type);

#endif // FUNCTION_TABLE_H