#include "evaluator.h"
#include "precision.h"
#include "constants.h"
#include "functions.h"
#include <stdio.h>
#include <string.h>

// Evaluation state
static int strict_mode = 0;
static char last_error[256] = {0};

// Forward declarations for static functions
static void evaluator_eval_constant(mpfr_t result, TokenType const_type);
static void evaluator_eval_binop(mpfr_t result, const ASTNode *node);
static void evaluator_eval_unary(mpfr_t result, const ASTNode *node);
static void evaluator_eval_function(mpfr_t result, const ASTNode *node);

void evaluator_eval(mpfr_t result, const ASTNode *node)
{
    evaluator_clear_error();

    if (!node)
    {
        mpfr_set_d(result, 0.0, global_rounding);
        return;
    }

    switch (node->type)
    {
    case NODE_NUMBER:
        mpfr_set(result, node->number.value, global_rounding);
        break;

    case NODE_CONSTANT:
        evaluator_eval_constant(result, node->constant.const_type);
        break;

    case NODE_BINOP:
        evaluator_eval_binop(result, node);
        break;

    case NODE_UNARY:
        evaluator_eval_unary(result, node);
        break;

    case NODE_FUNCTION:
        evaluator_eval_function(result, node);
        break;

    default:
        snprintf(last_error, sizeof(last_error), "Unknown node type");
        mpfr_set_d(result, 0.0, global_rounding);
    }
}

static void evaluator_eval_constant(mpfr_t result, TokenType const_type)
{
    switch (const_type)
    {
    case TOKEN_PI:
        constants_get_pi(result);
        break;
    case TOKEN_E:
        constants_get_e(result);
        break;
    case TOKEN_LN2:
        constants_get_ln2(result);
        break;
    case TOKEN_LN10:
        constants_get_ln10(result);
        break;
    case TOKEN_GAMMA:
        constants_get_gamma(result);
        break;
    default:
        snprintf(last_error, sizeof(last_error), "Unknown constant");
        mpfr_set_d(result, 0.0, global_rounding);
    }
}

static void evaluator_eval_binop(mpfr_t result, const ASTNode *node)
{
    mpfr_t left, right;
    mpfr_init2(left, global_precision);
    mpfr_init2(right, global_precision);

    evaluator_eval(left, node->binop.left);
    evaluator_eval(right, node->binop.right);

    switch (node->binop.op)
    {
    case TOKEN_PLUS:
        mpfr_add(result, left, right, global_rounding);
        break;
    case TOKEN_MINUS:
        mpfr_sub(result, left, right, global_rounding);
        break;
    case TOKEN_STAR:
        mpfr_mul(result, left, right, global_rounding);
        break;
    case TOKEN_SLASH:
        if (mpfr_zero_p(right))
        {
            snprintf(last_error, sizeof(last_error), "Division by zero");
            mpfr_set_d(result, 0.0, global_rounding);
        }
        else
        {
            mpfr_div(result, left, right, global_rounding);
        }
        break;
    case TOKEN_CARET:
        mpfr_pow(result, left, right, global_rounding);
        break;
    case TOKEN_EQ:
        mpfr_set_d(result, mpfr_equal_p(left, right) ? 1.0 : 0.0, global_rounding);
        break;
    case TOKEN_NEQ:
        mpfr_set_d(result, !mpfr_equal_p(left, right) ? 1.0 : 0.0, global_rounding);
        break;
    case TOKEN_LT:
        mpfr_set_d(result, mpfr_less_p(left, right) ? 1.0 : 0.0, global_rounding);
        break;
    case TOKEN_LTE:
        mpfr_set_d(result, mpfr_lessequal_p(left, right) ? 1.0 : 0.0, global_rounding);
        break;
    case TOKEN_GT:
        mpfr_set_d(result, mpfr_greater_p(left, right) ? 1.0 : 0.0, global_rounding);
        break;
    case TOKEN_GTE:
        mpfr_set_d(result, mpfr_greaterequal_p(left, right) ? 1.0 : 0.0, global_rounding);
        break;
    default:
        snprintf(last_error, sizeof(last_error), "Unknown binary operator");
        mpfr_set_d(result, 0.0, global_rounding);
    }

    mpfr_clear(left);
    mpfr_clear(right);
}

static void evaluator_eval_unary(mpfr_t result, const ASTNode *node)
{
    mpfr_t operand;
    mpfr_init2(operand, global_precision);
    evaluator_eval(operand, node->unary.operand);

    switch (node->unary.op)
    {
    case TOKEN_PLUS:
        mpfr_set(result, operand, global_rounding);
        break;
    case TOKEN_MINUS:
        mpfr_neg(result, operand, global_rounding);
        break;
    default:
        snprintf(last_error, sizeof(last_error), "Unknown unary operator");
        mpfr_set_d(result, 0.0, global_rounding);
    }

    mpfr_clear(operand);
}

static void evaluator_eval_function(mpfr_t result, const ASTNode *node)
{
    mpfr_t args[2]; // Max 2 args for current functions
    for (int i = 0; i < node->function.arg_count; i++)
    {
        mpfr_init2(args[i], global_precision);
        evaluator_eval(args[i], node->function.args[i]);
    }

    // Delegate to functions module
    int success = functions_eval(result, node->function.func_type, args, node->function.arg_count);

    if (!success && strict_mode)
    {
        snprintf(last_error, sizeof(last_error), "Function evaluation failed: %s",
                 functions_get_last_error());
    }

    for (int i = 0; i < node->function.arg_count; i++)
    {
        mpfr_clear(args[i]);
    }
}

// TODO
int evaluator_check_domain(const ASTNode *node)
{
    // This would recursively check if evaluation would cause domain errors
    // Implementation depends on specific domain checking requirements
    (void)node; // Suppress unused warning for now
    return 0;
}

void evaluator_set_strict_mode(int strict)
{
    strict_mode = strict;
}

const char *evaluator_get_last_error(void)
{
    return strlen(last_error) > 0 ? last_error : NULL;
}

void evaluator_clear_error(void)
{
    last_error[0] = '\0';
}