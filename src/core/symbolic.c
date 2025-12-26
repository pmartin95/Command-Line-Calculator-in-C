#include "symbolic.h"
#include "precision.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Error handling
static char last_error[256] = {0};

// Forward declarations
static ASTNode *symbolic_simplify_binop(TokenType op, ASTNode *left, ASTNode *right);
static ASTNode *symbolic_simplify_unary(TokenType op, ASTNode *operand);
static ASTNode *symbolic_simplify_function(TokenType func_type, ASTNode **args, int arg_count);

ASTNode *symbolic_eval(const ASTNode *node)
{
    symbolic_clear_error();

    if (!node)
    {
        return NULL;
    }

    switch (node->type)
    {
    case NODE_NUMBER:
    case NODE_CONSTANT:
        // Numbers and constants stay as-is
        return symbolic_clone(node);

    case NODE_BINOP:
    {
        // Recursively simplify children
        ASTNode *left = symbolic_eval(node->binop.left);
        ASTNode *right = symbolic_eval(node->binop.right);

        // Try to simplify the operation
        ASTNode *result = symbolic_simplify_binop(node->binop.op, left, right);

        return result;
    }

    case NODE_UNARY:
    {
        // Recursively simplify operand
        ASTNode *operand = symbolic_eval(node->unary.operand);

        // Try to simplify
        ASTNode *result = symbolic_simplify_unary(node->unary.op, operand);

        return result;
    }

    case NODE_FUNCTION:
    {
        // Recursively simplify arguments
        ASTNode **args = malloc(sizeof(ASTNode *) * node->function.arg_count);
        for (int i = 0; i < node->function.arg_count; i++)
        {
            args[i] = symbolic_eval(node->function.args[i]);
        }

        // Try to simplify function
        ASTNode *result = symbolic_simplify_function(node->function.func_type, args, node->function.arg_count);

        return result;
    }

    default:
        snprintf(last_error, sizeof(last_error), "Unknown node type in symbolic evaluation");
        return symbolic_clone(node);
    }
}

static ASTNode *symbolic_simplify_binop(TokenType op, ASTNode *left, ASTNode *right)
{
    // For now, just create the binary operation node
    // Simplification rules will be added in Phase 2
    return ast_create_binop(op, left, right);
}

static ASTNode *symbolic_simplify_unary(TokenType op, ASTNode *operand)
{
    // For now, just create the unary operation node
    // Simplification rules will be added in Phase 2
    return ast_create_unary(op, operand);
}

static ASTNode *symbolic_simplify_function(TokenType func_type, ASTNode **args, int arg_count)
{
    // For now, just create the function node
    // Simplification rules will be added in Phase 2
    return ast_create_function(func_type, args, arg_count);
}

ASTNode *symbolic_clone(const ASTNode *node)
{
    if (!node)
    {
        return NULL;
    }

    switch (node->type)
    {
    case NODE_NUMBER:
    {
        // Create new number node with same value
        ASTNode *clone = malloc(sizeof(ASTNode));
        if (!clone)
            return NULL;

        clone->type = NODE_NUMBER;
        mpfr_init2(clone->number.value, mpfr_get_prec(node->number.value));
        mpfr_set(clone->number.value, node->number.value, global_rounding);
        clone->number.is_int = node->number.is_int;

        return clone;
    }

    case NODE_CONSTANT:
    {
        // Create new constant node with same name
        return ast_create_constant(node->constant.name);
    }

    case NODE_BINOP:
    {
        // Recursively clone children
        ASTNode *left = symbolic_clone(node->binop.left);
        ASTNode *right = symbolic_clone(node->binop.right);
        return ast_create_binop(node->binop.op, left, right);
    }

    case NODE_UNARY:
    {
        // Recursively clone operand
        ASTNode *operand = symbolic_clone(node->unary.operand);
        return ast_create_unary(node->unary.op, operand);
    }

    case NODE_FUNCTION:
    {
        // Recursively clone arguments
        ASTNode **args = malloc(sizeof(ASTNode *) * node->function.arg_count);
        for (int i = 0; i < node->function.arg_count; i++)
        {
            args[i] = symbolic_clone(node->function.args[i]);
        }
        return ast_create_function(node->function.func_type, args, node->function.arg_count);
    }

    default:
        return NULL;
    }
}

int symbolic_equals(const ASTNode *a, const ASTNode *b)
{
    if (!a && !b)
        return 1;
    if (!a || !b)
        return 0;

    if (a->type != b->type)
        return 0;

    switch (a->type)
    {
    case NODE_NUMBER:
        return mpfr_equal_p(a->number.value, b->number.value) &&
               (a->number.is_int == b->number.is_int);

    case NODE_CONSTANT:
        return strcmp(a->constant.name, b->constant.name) == 0;

    case NODE_BINOP:
        return (a->binop.op == b->binop.op) &&
               symbolic_equals(a->binop.left, b->binop.left) &&
               symbolic_equals(a->binop.right, b->binop.right);

    case NODE_UNARY:
        return (a->unary.op == b->unary.op) &&
               symbolic_equals(a->unary.operand, b->unary.operand);

    case NODE_FUNCTION:
        if (a->function.func_type != b->function.func_type)
            return 0;
        if (a->function.arg_count != b->function.arg_count)
            return 0;
        for (int i = 0; i < a->function.arg_count; i++)
        {
            if (!symbolic_equals(a->function.args[i], b->function.args[i]))
                return 0;
        }
        return 1;

    default:
        return 0;
    }
}

int symbolic_is_zero(const ASTNode *node)
{
    if (!node)
        return 0;

    if (node->type == NODE_NUMBER)
    {
        return mpfr_zero_p(node->number.value);
    }

    return 0;
}

int symbolic_is_one(const ASTNode *node)
{
    if (!node)
        return 0;

    if (node->type == NODE_NUMBER)
    {
        return mpfr_cmp_ui(node->number.value, 1) == 0;
    }

    return 0;
}

int symbolic_is_integer(const ASTNode *node)
{
    if (!node)
        return 0;

    if (node->type == NODE_NUMBER)
    {
        return node->number.is_int && mpfr_integer_p(node->number.value);
    }

    return 0;
}

const char *symbolic_get_last_error(void)
{
    return strlen(last_error) > 0 ? last_error : NULL;
}

void symbolic_clear_error(void)
{
    last_error[0] = '\0';
}
