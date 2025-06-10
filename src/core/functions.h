#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "tokens.h"
#include <mpfr.h>

/**
 * Initialize functions system
 */
void functions_init(void);

/**
 * Evaluate a mathematical function
 * @param result Output variable for result
 * @param func_type Function token type
 * @param args Array of argument values
 * @param arg_count Number of arguments
 * @return 1 on success, 0 on error
 */
int functions_eval(mpfr_t result, TokenType func_type, mpfr_t args[], int arg_count);

/**
 * Check if function evaluation would cause domain error
 * @param func_type Function token type
 * @param args Array of argument values
 * @param arg_count Number of arguments
 * @return 1 if domain error would occur, 0 otherwise
 */
int functions_check_domain(TokenType func_type, mpfr_t args[], int arg_count);

/**
 * Get last function evaluation error message
 * @return Error message or NULL if no error
 */
const char *functions_get_last_error(void);

/**
 * Clear any stored function error state
 */
void functions_clear_error(void);

/**
 * Set function evaluation mode
 * @param strict_domain If 1, domain errors return NaN; if 0, return 0 with error
 */
void functions_set_strict_domain(int strict_domain);

/**
 * Cleanup functions system
 */
void functions_cleanup(void);

#endif // FUNCTIONS_H