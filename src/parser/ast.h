#ifndef AST_H
#define AST_H

#include "tokens.h"
#include <mpfr.h>

typedef enum
{
    NODE_NUMBER,
    NODE_BINOP,
    NODE_UNARY,
    NODE_FUNCTION,
    NODE_CONSTANT
} NodeType;

typedef struct ASTNode
{
    NodeType type;
    union
    {
        struct
        {
            mpfr_t value; // High precision number
            int is_int;   // Track if originally an integer
        } number;
        struct
        {
            TokenType op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binop;
        struct
        {
            TokenType op;
            struct ASTNode *operand;
        } unary;
        struct
        {
            TokenType func_type;
            struct ASTNode **args;
            int arg_count;
        } function;
        struct
        {
            char *name;  // Constant name (e.g., "pi", "e", "sqrt2")
        } constant;
    };
} ASTNode;

/**
 * Create a number node from string representation
 * @param str String representation of the number
 * @param is_int Whether the original input was an integer
 * @return New AST node or NULL on failure
 */
ASTNode *ast_create_number(const char *str, int is_int);

/**
 * Create a binary operation node
 * @param op Operation token type
 * @param left Left operand (takes ownership)
 * @param right Right operand (takes ownership)
 * @return New AST node or NULL on failure
 */
ASTNode *ast_create_binop(TokenType op, ASTNode *left, ASTNode *right);

/**
 * Create a unary operation node
 * @param op Operation token type
 * @param operand Operand (takes ownership)
 * @return New AST node or NULL on failure
 */
ASTNode *ast_create_unary(TokenType op, ASTNode *operand);

/**
 * Create a function call node
 * @param func_type Function token type
 * @param args Array of argument nodes (takes ownership)
 * @param arg_count Number of arguments
 * @return New AST node or NULL on failure
 */
ASTNode *ast_create_function(TokenType func_type, ASTNode **args, int arg_count);

/**
 * Create a constant node
 * @param name Constant name (e.g., "pi", "e", "sqrt2")
 * @return New AST node or NULL on failure
 */
ASTNode *ast_create_constant(const char *name);

/**
 * Free an AST and all its children
 * @param node Root node to free
 */
void ast_free(ASTNode *node);

/**
 * Print AST structure for debugging
 * @param node Root node to print
 * @param depth Current indentation depth
 */
void ast_print(const ASTNode *node, int depth);

#endif // AST_H