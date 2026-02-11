#include "evaluator.h"
#include "precision.h"
#include "constants.h"
#include "functions.h"
#include "function_table.h"
#include "calc_limits.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Evaluation state
static int strict_mode = 0;
static char last_error[ERROR_BUFFER_SIZE] = {0};

// Forward declarations for static functions
static void evaluator_eval_constant(mpfr_t result, const char *const_name);
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
        evaluator_eval_constant(result, node->constant.name);
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

static void evaluator_eval_constant(mpfr_t result, const char *const_name)
{
    // Use the metadata-driven lookup!
    if (!constants_get_by_name(result, const_name))
    {
        snprintf(last_error, sizeof(last_error), "Unknown constant: %s", const_name);
        mpfr_set_d(result, 0.0, global_rounding);
    }
}

static void evaluator_eval_binop(mpfr_t result, const ASTNode *node)
{
    // Use higher precision for intermediate calculations
    mpfr_prec_t high_prec = global_precision + PRECISION_BOOST;

    mpfr_t left, right, high_prec_result;
    mpfr_init2(left, high_prec);
    mpfr_init2(right, high_prec);
    mpfr_init2(high_prec_result, high_prec);

    evaluator_eval(left, node->binop.left);
    evaluator_eval(right, node->binop.right);

    switch (node->binop.op)
    {
    case TOKEN_PLUS:
        mpfr_add(high_prec_result, left, right, global_rounding);
        break;
    case TOKEN_MINUS:
        mpfr_sub(high_prec_result, left, right, global_rounding);
        break;
    case TOKEN_STAR:
        mpfr_mul(high_prec_result, left, right, global_rounding);
        break;
    case TOKEN_SLASH:
        if (mpfr_zero_p(right))
        {
            snprintf(last_error, sizeof(last_error), "Division by zero");
            mpfr_set_d(high_prec_result, 0.0, global_rounding);
        }
        else
        {
            mpfr_div(high_prec_result, left, right, global_rounding);
        }
        break;
    case TOKEN_CARET:
        mpfr_pow(high_prec_result, left, right, global_rounding);
        break;
    case TOKEN_EQ:
        mpfr_set_d(high_prec_result, mpfr_equal_p(left, right) ? 1.0 : 0.0, global_rounding);
        break;
    case TOKEN_NEQ:
        mpfr_set_d(high_prec_result, !mpfr_equal_p(left, right) ? 1.0 : 0.0, global_rounding);
        break;
    case TOKEN_LT:
        mpfr_set_d(high_prec_result, mpfr_less_p(left, right) ? 1.0 : 0.0, global_rounding);
        break;
    case TOKEN_LTE:
        mpfr_set_d(high_prec_result, mpfr_lessequal_p(left, right) ? 1.0 : 0.0, global_rounding);
        break;
    case TOKEN_GT:
        mpfr_set_d(high_prec_result, mpfr_greater_p(left, right) ? 1.0 : 0.0, global_rounding);
        break;
    case TOKEN_GTE:
        mpfr_set_d(high_prec_result, mpfr_greaterequal_p(left, right) ? 1.0 : 0.0, global_rounding);
        break;
    default:
        snprintf(last_error, sizeof(last_error), "Unknown binary operator");
        mpfr_set_d(high_prec_result, 0.0, global_rounding);
    }

    // Round result back to user's precision
    mpfr_set(result, high_prec_result, global_rounding);

    mpfr_clear(high_prec_result);
    mpfr_clear(left);
    mpfr_clear(right);
}

static void evaluator_eval_unary(mpfr_t result, const ASTNode *node)
{
    // Use higher precision for intermediate calculations
    mpfr_prec_t high_prec = global_precision + PRECISION_BOOST;

    mpfr_t operand, high_prec_result;
    mpfr_init2(operand, high_prec);
    mpfr_init2(high_prec_result, high_prec);
    evaluator_eval(operand, node->unary.operand);

    switch (node->unary.op)
    {
    case TOKEN_PLUS:
        mpfr_set(high_prec_result, operand, global_rounding);
        break;
    case TOKEN_MINUS:
        mpfr_neg(high_prec_result, operand, global_rounding);
        break;
    default:
        snprintf(last_error, sizeof(last_error), "Unknown unary operator");
        mpfr_set_d(high_prec_result, 0.0, global_rounding);
    }

    // Round result back to user's precision
    mpfr_set(result, high_prec_result, global_rounding);

    mpfr_clear(high_prec_result);
    mpfr_clear(operand);
}

static void evaluator_eval_function(mpfr_t result, const ASTNode *node)
{
    int arg_count = node->function.arg_count;

    if (arg_count <= 0 || arg_count > MAX_FUNCTION_ARGS)
    {
        snprintf(last_error, sizeof(last_error),
                 "Invalid argument count: %d (max %d)", arg_count, MAX_FUNCTION_ARGS);
        mpfr_set_d(result, 0.0, global_rounding);
        return;
    }

    // Use higher precision for function arguments to reduce cumulative error
    mpfr_prec_t high_prec = global_precision + PRECISION_BOOST;

    mpfr_t *args = malloc(sizeof(mpfr_t) * (size_t)arg_count);
    if (!args)
    {
        snprintf(last_error, sizeof(last_error), "Memory allocation failed for function args");
        mpfr_set_d(result, 0.0, global_rounding);
        return;
    }

    for (int i = 0; i < arg_count; i++)
    {
        mpfr_init2(args[i], high_prec);
        evaluator_eval(args[i], node->function.args[i]);
    }

    // Compute function at high precision then round to user's precision
    mpfr_t high_prec_result;
    mpfr_init2(high_prec_result, high_prec);

    // Delegate to functions module
    int success = functions_eval(high_prec_result, node->function.func_type, args, arg_count);

    if (!success && strict_mode)
    {
        snprintf(last_error, sizeof(last_error), "Function evaluation failed: %s",
                 functions_get_last_error());
    }

    // Round result to user's precision
    mpfr_set(result, high_prec_result, global_rounding);

    // Round very small values to zero to handle floating-point artifacts
    // If |result| < 2^(-global_precision - 10), treat as zero
    mpfr_t epsilon;
    mpfr_init2(epsilon, global_precision);
    mpfr_set_ui_2exp(epsilon, 1, -(long)(global_precision + 10), global_rounding);

    if (mpfr_cmpabs(result, epsilon) < 0)
    {
        mpfr_set_zero(result, 0);
    }

    mpfr_clear(epsilon);
    mpfr_clear(high_prec_result);
    for (int i = 0; i < arg_count; i++)
    {
        mpfr_clear(args[i]);
    }
    free(args);
}

int evaluator_check_domain(const ASTNode *node)
{
    if (!node)
        return 0;

    switch (node->type)
    {
    case NODE_NUMBER:
    case NODE_CONSTANT:
        return 0;

    case NODE_BINOP:
    {
        // Check children first
        if (evaluator_check_domain(node->binop.left) ||
            evaluator_check_domain(node->binop.right))
            return 1;

        // Check division by zero
        if (node->binop.op == TOKEN_SLASH)
        {
            mpfr_t right_val;
            mpfr_init2(right_val, global_precision);
            evaluator_eval(right_val, node->binop.right);
            int is_zero = mpfr_zero_p(right_val);
            mpfr_clear(right_val);
            if (is_zero)
            {
                snprintf(last_error, sizeof(last_error), "Division by zero");
                return 1;
            }
        }
        return 0;
    }

    case NODE_UNARY:
        return evaluator_check_domain(node->unary.operand);

    case NODE_FUNCTION:
    {
        int arg_count = node->function.arg_count;

        // Check children first
        for (int i = 0; i < arg_count; i++)
        {
            if (evaluator_check_domain(node->function.args[i]))
                return 1;
        }

        // Evaluate arguments to check function domain
        if (arg_count <= 0 || arg_count > MAX_FUNCTION_ARGS)
            return 1;

        mpfr_t *args = malloc(sizeof(mpfr_t) * (size_t)arg_count);
        if (!args)
            return 1;

        for (int i = 0; i < arg_count; i++)
        {
            mpfr_init2(args[i], global_precision);
            evaluator_eval(args[i], node->function.args[i]);
        }

        int domain_error = functions_check_domain(
            node->function.func_type, args, arg_count);

        for (int i = 0; i < arg_count; i++)
            mpfr_clear(args[i]);
        free(args);

        if (domain_error)
        {
            snprintf(last_error, sizeof(last_error),
                     "Domain error in %s()",
                     function_table_get_name(node->function.func_type));
        }
        return domain_error;
    }

    default:
        return 0;
    }
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