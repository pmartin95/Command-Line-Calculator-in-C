#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

// Forward declaration to avoid circular includes
typedef struct Parser Parser;

#include "tokens.h"

/**
 * Check if implicit multiplication should be inserted between tokens
 * @param parser Parser state
 * @return 1 if multiplication should be inserted, 0 otherwise
 */
int parser_utils_should_insert_multiplication(Parser *parser);

/**
 * Check if the current token sequence is valid
 * @param parser Parser state
 * @return 1 if valid, 0 if invalid
 */
int parser_utils_validate_token_sequence(Parser *parser);

/**
 * Skip to the next statement boundary (for error recovery)
 * @param parser Parser state
 */
void parser_utils_skip_to_statement_boundary(Parser *parser);

/**
 * Check if we're at the end of an expression
 * @param parser Parser state
 * @return 1 if at end, 0 otherwise
 */
int parser_utils_at_expression_end(Parser *parser);

/**
 * Get current parsing context for error messages
 * @param parser Parser state
 * @return String describing current context
 */
const char *parser_utils_get_context(Parser *parser);

#endif // PARSER_UTILS_H