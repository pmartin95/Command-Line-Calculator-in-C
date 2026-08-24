#include "ast.h"
#include "precision.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASTNode *ast_create_number(const char *str, int is_int)
{
    if (!str)
    {
        return NULL;
    }

    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    node->type = NODE_NUMBER;
    node->number.is_int = is_int;

    // Initialize MPFR number with current precision
    mpfr_init2(node->number.value, global_precision);

    // Parse the string with MPFR
    int ret = mpfr_set_str(node->number.value, str, 10, global_rounding);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to parse number: %s\n", str);
        mpfr_clear(node->number.value);
        free(node);
        return NULL;
    }

    return node;
}

ASTNode *ast_create_binop(TokenType op, ASTNode *left, ASTNode *right)
{
    if (!left || !right)
    {
        ast_free(left);
        ast_free(right);
        return NULL;
    }

    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node)
    {
        fprintf(stderr, "Memory allocation failed\n");
        ast_free(left);
        ast_free(right);
        return NULL;
    }

    node->type = NODE_BINOP;
    node->binop.op = op;
    node->binop.left = left;
    node->binop.right = right;
    return node;
}

ASTNode *ast_create_unary(TokenType op, ASTNode *operand)
{
    if (!operand)
    {
        return NULL;
    }

    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node)
    {
        fprintf(stderr, "Memory allocation failed\n");
        ast_free(operand);
        return NULL;
    }

    node->type = NODE_UNARY;
    node->unary.op = op;
    node->unary.operand = operand;
    return node;
}

ASTNode *ast_create_function(TokenType func_type, ASTNode **args, int arg_count)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node)
    {
        fprintf(stderr, "Memory allocation failed\n");
        if (args)
        {
            for (int i = 0; i < arg_count; i++)
            {
                ast_free(args[i]);
            }
            free(args);
        }
        return NULL;
    }

    node->type = NODE_FUNCTION;
    node->function.func_type = func_type;
    node->function.args = args;
    node->function.arg_count = arg_count;
    return node;
}

ASTNode *ast_create_constant(const char *name)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    node->type = NODE_CONSTANT;
    node->constant.name = strdup(name);
    if (!node->constant.name)
    {
        fprintf(stderr, "Memory allocation failed for constant name\n");
        free(node);
        return NULL;
    }
    return node;
}

void ast_free(ASTNode *node)
{
    if (!node)
    {
        return;
    }

    switch (node->type)
    {
    case NODE_NUMBER:
        mpfr_clear(node->number.value);
        break;
    case NODE_CONSTANT:
        free(node->constant.name);
        break;
    case NODE_BINOP:
        ast_free(node->binop.left);
        ast_free(node->binop.right);
        break;
    case NODE_UNARY:
        ast_free(node->unary.operand);
        break;
    case NODE_FUNCTION:
        if (node->function.args)
        {
            for (int i = 0; i < node->function.arg_count; i++)
            {
                ast_free(node->function.args[i]);
            }
            free(node->function.args);
        }
        break;
    }
    free(node);
}

ASTNode *ast_clone(const ASTNode *node)
{
    if (!node)
    {
        return NULL;
    }

    switch (node->type)
    {
    case NODE_NUMBER:
    {
        ASTNode *clone = malloc(sizeof(ASTNode));
        if (!clone)
            return NULL;
        clone->type = NODE_NUMBER;
        mpfr_init2(clone->number.value, mpfr_get_prec(node->number.value));
        mpfr_set(clone->number.value, node->number.value, MPFR_RNDN);
        clone->number.is_int = node->number.is_int;
        return clone;
    }

    case NODE_CONSTANT:
        return ast_create_constant(node->constant.name);

    case NODE_BINOP:
    {
        ASTNode *left = ast_clone(node->binop.left);
        ASTNode *right = ast_clone(node->binop.right);
        return ast_create_binop(node->binop.op, left, right);
    }

    case NODE_UNARY:
    {
        ASTNode *operand = ast_clone(node->unary.operand);
        return ast_create_unary(node->unary.op, operand);
    }

    case NODE_FUNCTION:
    {
        ASTNode **args = malloc(sizeof(ASTNode *) * (size_t)node->function.arg_count);
        if (!args)
            return NULL;
        for (int i = 0; i < node->function.arg_count; i++)
        {
            args[i] = ast_clone(node->function.args[i]);
        }
        return ast_create_function(node->function.func_type, args, node->function.arg_count);
    }

    default:
        return NULL;
    }
}
