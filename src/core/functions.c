#include "functions.h"
#include "precision.h"
#include <stdio.h>
#include <string.h>

// Function evaluation state
static int strict_domain_mode = 0;
static char last_error[256] = {0};

void functions_init(void)
{
    strict_domain_mode = 0;
    last_error[0] = '\0';
}

int functions_eval(mpfr_t result, TokenType func_type, mpfr_t args[], int arg_count)
{
    functions_clear_error();

    // Check domain first if in strict mode
    if (strict_domain_mode && functions_check_domain(func_type, args, arg_count))
    {
        mpfr_set_nan(result);
        return 0;
    }

    switch (func_type)
    {
    // Trigonometric functions
    case TOKEN_SIN:
        if (arg_count != 1)
            goto arg_error;
        mpfr_sin(result, args[0], global_rounding);
        return 1;

    case TOKEN_COS:
        if (arg_count != 1)
            goto arg_error;
        mpfr_cos(result, args[0], global_rounding);
        return 1;

    case TOKEN_TAN:
        if (arg_count != 1)
            goto arg_error;
        mpfr_tan(result, args[0], global_rounding);
        return 1;

    // Inverse trigonometric functions
    case TOKEN_ASIN:
        if (arg_count != 1)
            goto arg_error;
        if (mpfr_cmp_d(args[0], -1.0) < 0 || mpfr_cmp_d(args[0], 1.0) > 0)
        {
            snprintf(last_error, sizeof(last_error), "asin domain error: argument must be in [-1,1]");
            mpfr_set_d(result, 0.0, global_rounding);
            return 0;
        }
        mpfr_asin(result, args[0], global_rounding);
        return 1;

    case TOKEN_ACOS:
        if (arg_count != 1)
            goto arg_error;
        if (mpfr_cmp_d(args[0], -1.0) < 0 || mpfr_cmp_d(args[0], 1.0) > 0)
        {
            snprintf(last_error, sizeof(last_error), "acos domain error: argument must be in [-1,1]");
            mpfr_set_d(result, 0.0, global_rounding);
            return 0;
        }
        mpfr_acos(result, args[0], global_rounding);
        return 1;

    case TOKEN_ATAN:
        if (arg_count != 1)
            goto arg_error;
        mpfr_atan(result, args[0], global_rounding);
        return 1;

    case TOKEN_ATAN2:
        if (arg_count != 2)
            goto arg_error;
        mpfr_atan2(result, args[0], args[1], global_rounding);
        return 1;

    // Hyperbolic functions
    case TOKEN_SINH:
        if (arg_count != 1)
            goto arg_error;
        mpfr_sinh(result, args[0], global_rounding);
        return 1;

    case TOKEN_COSH:
        if (arg_count != 1)
            goto arg_error;
        mpfr_cosh(result, args[0], global_rounding);
        return 1;

    case TOKEN_TANH:
        if (arg_count != 1)
            goto arg_error;
        mpfr_tanh(result, args[0], global_rounding);
        return 1;

    // Inverse hyperbolic functions
    case TOKEN_ASINH:
        if (arg_count != 1)
            goto arg_error;
        mpfr_asinh(result, args[0], global_rounding);
        return 1;

    case TOKEN_ACOSH:
        if (arg_count != 1)
            goto arg_error;
        if (mpfr_cmp_d(args[0], 1.0) < 0)
        {
            snprintf(last_error, sizeof(last_error), "acosh domain error: argument must be >= 1");
            mpfr_set_d(result, 0.0, global_rounding);
            return 0;
        }
        mpfr_acosh(result, args[0], global_rounding);
        return 1;

    case TOKEN_ATANH:
        if (arg_count != 1)
            goto arg_error;
        if (mpfr_cmp_d(args[0], -1.0) <= 0 || mpfr_cmp_d(args[0], 1.0) >= 0)
        {
            snprintf(last_error, sizeof(last_error), "atanh domain error: argument must be in (-1,1)");
            mpfr_set_d(result, 0.0, global_rounding);
            return 0;
        }
        mpfr_atanh(result, args[0], global_rounding);
        return 1;

    // Other mathematical functions
    case TOKEN_SQRT:
        if (arg_count != 1)
            goto arg_error;
        if (mpfr_cmp_d(args[0], 0.0) < 0)
        {
            snprintf(last_error, sizeof(last_error), "sqrt domain error: argument must be >= 0");
            mpfr_set_d(result, 0.0, global_rounding);
            return 0;
        }
        mpfr_sqrt(result, args[0], global_rounding);
        return 1;

    case TOKEN_LOG:
        if (arg_count != 1)
            goto arg_error;
        if (mpfr_cmp_d(args[0], 0.0) <= 0)
        {
            snprintf(last_error, sizeof(last_error), "log domain error: argument must be > 0");
            mpfr_set_d(result, 0.0, global_rounding);
            return 0;
        }
        mpfr_log(result, args[0], global_rounding);
        return 1;

    case TOKEN_LOG10:
        if (arg_count != 1)
            goto arg_error;
        if (mpfr_cmp_d(args[0], 0.0) <= 0)
        {
            snprintf(last_error, sizeof(last_error), "log10 domain error: argument must be > 0");
            mpfr_set_d(result, 0.0, global_rounding);
            return 0;
        }
        mpfr_log10(result, args[0], global_rounding);
        return 1;

    case TOKEN_EXP:
        if (arg_count != 1)
            goto arg_error;
        mpfr_exp(result, args[0], global_rounding);
        return 1;

    case TOKEN_ABS:
        if (arg_count != 1)
            goto arg_error;
        mpfr_abs(result, args[0], global_rounding);
        return 1;

    case TOKEN_FLOOR:
        if (arg_count != 1)
            goto arg_error;
        mpfr_floor(result, args[0]);
        return 1;

    case TOKEN_CEIL:
        if (arg_count != 1)
            goto arg_error;
        mpfr_ceil(result, args[0]);
        return 1;

    case TOKEN_POW:
        if (arg_count != 2)
            goto arg_error;
        mpfr_pow(result, args[0], args[1], global_rounding);
        return 1;

    default:
        snprintf(last_error, sizeof(last_error), "Unknown function");
        mpfr_set_d(result, 0.0, global_rounding);
        return 0;
    }

arg_error:
    snprintf(last_error, sizeof(last_error), "Wrong number of arguments for function");
    mpfr_set_d(result, 0.0, global_rounding);
    return 0;
}

int functions_check_domain(TokenType func_type, mpfr_t args[], int arg_count)
{
    switch (func_type)
    {
    case TOKEN_ASIN:
    case TOKEN_ACOS:
        if (arg_count != 1)
            return 1;
        return (mpfr_cmp_d(args[0], -1.0) < 0 || mpfr_cmp_d(args[0], 1.0) > 0);

    case TOKEN_ACOSH:
        if (arg_count != 1)
            return 1;
        return (mpfr_cmp_d(args[0], 1.0) < 0);

    case TOKEN_ATANH:
        if (arg_count != 1)
            return 1;
        return (mpfr_cmp_d(args[0], -1.0) <= 0 || mpfr_cmp_d(args[0], 1.0) >= 0);

    case TOKEN_SQRT:
        if (arg_count != 1)
            return 1;
        return (mpfr_cmp_d(args[0], 0.0) < 0);

    case TOKEN_LOG:
    case TOKEN_LOG10:
        if (arg_count != 1)
            return 1;
        return (mpfr_cmp_d(args[0], 0.0) <= 0);

    // Functions with no domain restrictions
    case TOKEN_SIN:
    case TOKEN_COS:
    case TOKEN_TAN:
    case TOKEN_ATAN:
    case TOKEN_ATAN2:
    case TOKEN_SINH:
    case TOKEN_COSH:
    case TOKEN_TANH:
    case TOKEN_ASINH:
    case TOKEN_EXP:
    case TOKEN_ABS:
    case TOKEN_FLOOR:
    case TOKEN_CEIL:
    case TOKEN_POW:
        return 0;

    default:
        return 1; // Unknown function
    }
}

const char *functions_get_last_error(void)
{
    return strlen(last_error) > 0 ? last_error : NULL;
}

void functions_clear_error(void)
{
    last_error[0] = '\0';
}

void functions_set_strict_domain(int strict_domain)
{
    strict_domain_mode = strict_domain;
}

void functions_cleanup(void)
{
    functions_clear_error();
}