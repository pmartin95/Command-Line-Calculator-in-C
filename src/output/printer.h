#ifndef PRINTER_H
#define PRINTER_H

#include "ast.h"
#include "tokens.h"

/**
 * Print AST structure with indentation for debugging
 * @param node Root node to print
 * @param depth Current indentation depth
 */
void printer_print_ast(const ASTNode *node, int depth);

/**
 * Print AST in a compact, single-line format
 * @param node Root node to print
 */
void printer_print_ast_compact(const ASTNode *node);

/**
 * Print AST in mathematical notation (infix)
 * @param node Root node to print
 */
void printer_print_ast_infix(const ASTNode *node);

/**
 * Print token information for debugging
 * @param token Token to print
 */
void printer_print_token(const Token *token);

/**
 * Print lexer state for debugging
 * @param lexer Lexer instance
 */
void printer_print_lexer_state(const void *lexer);

/**
 * Print parser state for debugging
 * @param parser Parser instance
 */
void printer_print_parser_state(const void *parser);

/**
 * Set debug printing level
 * @param level 0=none, 1=basic, 2=verbose, 3=everything
 */
void printer_set_debug_level(int level);

/**
 * Get current debug printing level
 * @return Current debug level
 */
int printer_get_debug_level(void);

#endif // PRINTER_H