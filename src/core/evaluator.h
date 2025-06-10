#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "ast.h"
#include <mpfr.h>

/**
 * Evaluate an AST and store result in MPFR variable
 * @param result Output variable for result
 * @param node AST node to evaluate
 */
void evaluator_eval(mpfr_t result, const ASTNode *node);

/**
 * Check if evaluation would cause domain error without actually evaluating
 * @param node AST node to check
 * @return 1 if domain error would occur, 0 otherwise
 */
int evaluator_check_domain(const ASTNode *node);

/**
 * Set evaluation options
 * @param strict_mode If 1, domain errors abort evaluation; if 0, return NaN
 */
void evaluator_set_strict_mode(int strict_mode);

/**
 * Get last evaluation error message
 * @return Error message or NULL if no error
 */
const char *evaluator_get_last_error(void);

/**
 * Clear any stored error state
 */
void evaluator_clear_error(void);

#endif // EVALUATOR_H