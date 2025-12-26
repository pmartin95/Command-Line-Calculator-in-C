#ifndef SYMBOLIC_H
#define SYMBOLIC_H

#include "ast.h"

/**
 * Symbolic evaluation - simplifies expressions without numeric evaluation
 * Returns a new simplified AST
 * @param node AST to evaluate symbolically
 * @return Simplified AST (caller must free with ast_free)
 */
ASTNode *symbolic_eval(const ASTNode *node);

/**
 * Clone an AST node (deep copy)
 * @param node Node to clone
 * @return New node (caller must free with ast_free)
 */
ASTNode *symbolic_clone(const ASTNode *node);

/**
 * Check if two AST nodes are structurally equal
 * @param a First node
 * @param b Second node
 * @return 1 if equal, 0 otherwise
 */
int symbolic_equals(const ASTNode *a, const ASTNode *b);

/**
 * Check if node represents zero
 * @param node Node to check
 * @return 1 if zero, 0 otherwise
 */
int symbolic_is_zero(const ASTNode *node);

/**
 * Check if node represents one
 * @param node Node to check
 * @return 1 if one, 0 otherwise
 */
int symbolic_is_one(const ASTNode *node);

/**
 * Check if node represents an integer
 * @param node Node to check
 * @return 1 if integer, 0 otherwise
 */
int symbolic_is_integer(const ASTNode *node);

/**
 * Get last symbolic evaluation error
 * @return Error string or NULL if no error
 */
const char *symbolic_get_last_error(void);

/**
 * Clear symbolic evaluation error
 */
void symbolic_clear_error(void);

#endif // SYMBOLIC_H
