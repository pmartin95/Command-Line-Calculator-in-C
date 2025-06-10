#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "tokens.h"
#include "ast.h"

typedef struct Parser
{
    Lexer *lexer;
    Token current_token;
    Token previous_token;
    int recursion_depth;
    int max_depth;
    int error_occurred;
} Parser;

/**
 * Initialize parser with lexer
 * @param parser Parser instance
 * @param lexer Lexer instance
 */
void parser_init(Parser *parser, Lexer *lexer);

/**
 * Advance to next token
 * @param parser Parser instance
 */
void parser_advance(Parser *parser);

/**
 * Parse a complete expression
 * @param parser Parser instance
 * @return AST root node or NULL on error
 */
ASTNode *parser_parse_expression(Parser *parser);

/**
 * Parse a comparison expression
 * @param parser Parser instance
 * @return AST node or NULL on error
 */
ASTNode *parser_parse_comparison(Parser *parser);

/**
 * Parse a term (addition/subtraction)
 * @param parser Parser instance
 * @return AST node or NULL on error
 */
ASTNode *parser_parse_term(Parser *parser);

/**
 * Parse a factor (multiplication/division)
 * @param parser Parser instance
 * @return AST node or NULL on error
 */
ASTNode *parser_parse_factor(Parser *parser);

/**
 * Parse a power expression
 * @param parser Parser instance
 * @return AST node or NULL on error
 */
ASTNode *parser_parse_power(Parser *parser);

/**
 * Parse a unary expression
 * @param parser Parser instance
 * @return AST node or NULL on error
 */
ASTNode *parser_parse_unary(Parser *parser);

/**
 * Parse a primary expression (numbers, identifiers, parentheses)
 * @param parser Parser instance
 * @return AST node or NULL on error
 */
ASTNode *parser_parse_primary(Parser *parser);

/**
 * Parse a function call
 * @param parser Parser instance
 * @param func_type Function token type
 * @return AST node or NULL on error
 */
ASTNode *parser_parse_function_call(Parser *parser, TokenType func_type);

/**
 * Check if parser has encountered an error
 * @param parser Parser instance
 * @return 1 if error occurred, 0 otherwise
 */
int parser_has_error(Parser *parser);

/**
 * Get current recursion depth
 * @param parser Parser instance
 * @return Current recursion depth
 */
int parser_get_recursion_depth(Parser *parser);

/**
 * Set maximum recursion depth
 * @param parser Parser instance
 * @param max_depth Maximum allowed recursion depth
 */
void parser_set_max_recursion_depth(Parser *parser, int max_depth);

/**
 * Reset parser error state
 * @param parser Parser instance
 */
void parser_clear_error(Parser *parser);

/**
 * Get error message if an error occurred
 * @param parser Parser instance
 * @return Error message or NULL if no error
 */
const char *parser_get_error_message(Parser *parser);

/**
 * Check if current position is at end of expression
 * @param parser Parser instance
 * @return 1 if at end, 0 otherwise
 */
int parser_at_end(Parser *parser);

/**
 * Get current token type
 * @param parser Parser instance
 * @return Current token type
 */
TokenType parser_current_token_type(Parser *parser);

/**
 * Get previous token type
 * @param parser Parser instance
 * @return Previous token type
 */
TokenType parser_previous_token_type(Parser *parser);

/**
 * Peek at current token without consuming it
 * @param parser Parser instance
 * @return Pointer to current token (do not modify or free)
 */
const Token *parser_peek_token(Parser *parser);

/**
 * Check if current token matches expected type
 * @param parser Parser instance
 * @param expected Expected token type
 * @return 1 if matches, 0 otherwise
 */
int parser_match_token(Parser *parser, TokenType expected);

/**
 * Consume current token if it matches expected type
 * @param parser Parser instance
 * @param expected Expected token type
 * @return 1 if consumed, 0 if no match
 */
int parser_consume_token(Parser *parser, TokenType expected);

/**
 * Synchronize parser after error (for error recovery)
 * @param parser Parser instance
 */
void parser_synchronize(Parser *parser);

/**
 * Set parser into panic mode for error recovery
 * @param parser Parser instance
 * @param error_msg Error message
 */
void parser_panic(Parser *parser, const char *error_msg);

/**
 * Check if parser is in panic mode
 * @param parser Parser instance
 * @return 1 if in panic mode, 0 otherwise
 */
int parser_is_panicking(Parser *parser);

#endif // PARSER_H