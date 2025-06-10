// tests/test_calculator.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpfr.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "evaluator.h"
#include "tokens.h"
#include "precision.h"

#define EPSILON 1e-9
#define MPFR_EPSILON 1e-15
#define TEST_ASSERT(condition, message)         \
    do                                          \
    {                                           \
        if (!(condition))                       \
        {                                       \
            printf("  ❌ FAIL: %s\n", message); \
            return 0;                           \
        }                                       \
    } while (0)

#define TEST_ASSERT_DOUBLE_EQ(actual, expected, message)                                \
    do                                                                                  \
    {                                                                                   \
        if (fabs((actual) - (expected)) >= EPSILON)                                     \
        {                                                                               \
            printf("  ❌ FAIL: %s (expected %g, got %g)\n", message, expected, actual); \
            return 0;                                                                   \
        }                                                                               \
    } while (0)

#define TEST_ASSERT_MPFR_EQ(actual, expected, message)     \
    do                                                     \
    {                                                      \
        mpfr_t diff, abs_diff;                             \
        mpfr_init2(diff, global_precision);                \
        mpfr_init2(abs_diff, global_precision);            \
        mpfr_sub(diff, actual, expected, global_rounding); \
        mpfr_abs(abs_diff, diff, global_rounding);         \
        if (mpfr_cmp_d(abs_diff, MPFR_EPSILON) >= 0)       \
        {                                                  \
            printf("  ❌ FAIL: %s\n", message);            \
            printf("    Expected: ");                      \
            mpfr_printf("%.15Rf\n", expected);             \
            printf("    Actual:   ");                      \
            mpfr_printf("%.15Rf\n", actual);               \
            mpfr_clear(diff);                              \
            mpfr_clear(abs_diff);                          \
            return 0;                                      \
        }                                                  \
        mpfr_clear(diff);                                  \
        mpfr_clear(abs_diff);                              \
    } while (0)

// Helper function to test expression evaluation with MPFR
int test_expression_mpfr(const char *expr, mpfr_t result)
{
    Lexer lexer;
    lexer_init(&lexer, expr);

    Parser parser;
    parser_init(&parser, &lexer);

    ASTNode *ast = parser_parse_expression(&parser);

    if (!ast || parser_has_error(&parser) || parser_current_token_type(&parser) != TOKEN_EOF)
    {
        ast_free(ast);
        return 0;
    }

    evaluator_eval(result, ast);
    ast_free(ast);
    return 1;
}

// Helper function to test expression evaluation (backwards compatibility)
double test_expression(const char *expr, int *parse_success)
{
    mpfr_t result;
    mpfr_init2(result, global_precision);

    *parse_success = test_expression_mpfr(expr, result);

    double double_result = mpfr_get_d(result, global_rounding);
    mpfr_clear(result);
    return double_result;
}

int test_lexer_basic()
{
    printf("Testing lexer basic functionality...\n");

    Lexer lexer;
    lexer_init(&lexer, "123 + 45.67");

    Token token = lexer_get_next_token(&lexer);
    TEST_ASSERT(token.type == TOKEN_INT, "First token should be INT");
    TEST_ASSERT(token.int_value == 123, "First token value should be 123");
    TEST_ASSERT(token.number_string != NULL, "Should have number string");
    TEST_ASSERT(strcmp(token.number_string, "123") == 0, "Number string should match");
    token_free(&token);

    token = lexer_get_next_token(&lexer);
    TEST_ASSERT(token.type == TOKEN_PLUS, "Second token should be PLUS");
    token_free(&token);

    token = lexer_get_next_token(&lexer);
    TEST_ASSERT(token.type == TOKEN_FLOAT, "Third token should be FLOAT");
    TEST_ASSERT(fabs(token.float_value - 45.67) < EPSILON, "Third token value should be 45.67");
    TEST_ASSERT(token.number_string != NULL, "Should have number string");
    TEST_ASSERT(strcmp(token.number_string, "45.67") == 0, "Number string should match");
    token_free(&token);

    token = lexer_get_next_token(&lexer);
    TEST_ASSERT(token.type == TOKEN_EOF, "Should reach EOF");
    token_free(&token);

    printf("  ✅ Basic lexer tests passed\n");
    return 1;
}

int test_lexer_functions()
{
    printf("Testing lexer function recognition...\n");

    Lexer lexer;
    lexer_init(&lexer, "sin cos tan sqrt log pi e");

    struct
    {
        TokenType expected;
        const char *name;
    } expected_tokens[] = {
        {TOKEN_SIN, "sin"},
        {TOKEN_COS, "cos"},
        {TOKEN_TAN, "tan"},
        {TOKEN_SQRT, "sqrt"},
        {TOKEN_LOG, "log"},
        {TOKEN_PI, "pi"},
        {TOKEN_E, "e"},
        {TOKEN_EOF, "EOF"}};

    for (int i = 0; i < 8; i++)
    {
        Token token = lexer_get_next_token(&lexer);
        TEST_ASSERT(token.type == expected_tokens[i].expected,
                    expected_tokens[i].name);
        token_free(&token);
    }

    printf("  ✅ Function recognition tests passed\n");
    return 1;
}

int test_lexer_scientific_notation()
{
    printf("Testing lexer scientific notation...\n");

    struct
    {
        const char *input;
        double expected;
    } tests[] = {
        {"1e3", 1000.0},
        {"2.5e2", 250.0},
        {"1.5e-2", 0.015},
        {"6.02e+23", 6.02e23},
        {"1e0", 1.0}};

    for (int i = 0; i < 5; i++)
    {
        Lexer lexer;
        lexer_init(&lexer, tests[i].input);

        Token token = lexer_get_next_token(&lexer);
        TEST_ASSERT(token.type == TOKEN_FLOAT, "Should parse as FLOAT");
        TEST_ASSERT_DOUBLE_EQ(token.float_value, tests[i].expected,
                              "Scientific notation value");
        TEST_ASSERT(token.number_string != NULL, "Should have number string");

        token_free(&token);
    }

    printf("  ✅ Scientific notation tests passed\n");
    return 1;
}

int test_basic_arithmetic()
{
    printf("Testing basic arithmetic...\n");

    struct
    {
        const char *expr;
        double expected;
    } tests[] = {
        {"2+3", 5.0},
        {"5-2", 3.0},
        {"3*4", 12.0},
        {"8/2", 4.0},
        {"2+3*4", 14.0},   // Precedence
        {"(2+3)*4", 20.0}, // Parentheses
        {"2^3", 8.0},      // Exponentiation
        {"2^3^2", 512.0},  // Right associative
        {"-5", -5.0},      // Unary minus
        {"+7", 7.0},       // Unary plus
        {"--3", 3.0},      // Double negative
        {"5-3-1", 1.0},    // Left associative subtraction
        {"2++3", 5.0},     // Plus followed by unary plus (2 + (+3))
        {"2+-3", -1.0},    // Plus followed by unary minus (2 + (-3))
        {"2--3", 5.0},     // Minus followed by unary minus (2 - (-3))
    };

    for (int i = 0; i < 15; i++)
    {
        int success;
        double result = test_expression(tests[i].expr, &success);
        TEST_ASSERT(success, "Expression should parse successfully");
        TEST_ASSERT_DOUBLE_EQ(result, tests[i].expected, tests[i].expr);
        printf("  ✅ %s = %g\n", tests[i].expr, result);
    }

    return 1;
}

int test_implicit_multiplication()
{
    printf("Testing implicit multiplication...\n");

    struct
    {
        const char *expr;
        double expected;
    } tests[] = {
        {"2(3+4)", 14.0},
        {"(2+1)(3+1)", 12.0},
        {"2pi", 2 * M_PI},
        {"3e", 3 * M_E},
        {"pi(2)", 2 * M_PI},
        {"(2)(3)(4)", 24.0},
        {"2sin(0)", 0.0}, // 2 * sin(0)
        {"2 3 4", 24.0},  // Implicit multiplication with spaces
    };

    for (int i = 0; i < 8; i++)
    {
        int success;
        double result = test_expression(tests[i].expr, &success);
        TEST_ASSERT(success, "Expression should parse successfully");
        TEST_ASSERT_DOUBLE_EQ(result, tests[i].expected, tests[i].expr);
        printf("  ✅ %s = %g\n", tests[i].expr, result);
    }

    return 1;
}

int test_trigonometric_functions()
{
    printf("Testing trigonometric functions...\n");

    struct
    {
        const char *expr;
        double expected;
    } tests[] = {
        {"sin(0)", 0.0},
        {"cos(0)", 1.0},
        {"tan(0)", 0.0},
        {"sin(pi/2)", 1.0},
        {"cos(pi)", -1.0},
        {"asin(1)", M_PI / 2},
        {"acos(1)", 0.0},
        {"atan(1)", M_PI / 4},
        {"atan2(1,1)", M_PI / 4},
    };

    for (int i = 0; i < 9; i++)
    {
        int success;
        double result = test_expression(tests[i].expr, &success);
        TEST_ASSERT(success, "Expression should parse successfully");
        TEST_ASSERT_DOUBLE_EQ(result, tests[i].expected, tests[i].expr);
        printf("  ✅ %s = %g\n", tests[i].expr, result);
    }

    return 1;
}

int test_logarithmic_functions()
{
    printf("Testing logarithmic and exponential functions...\n");

    struct
    {
        const char *expr;
        double expected;
    } tests[] = {
        {"log(e)", 1.0},
        {"log10(100)", 2.0},
        {"exp(0)", 1.0},
        {"exp(1)", M_E},
        {"sqrt(16)", 4.0},
        {"sqrt(2)", sqrt(2)},
        {"pow(2,3)", 8.0},
        {"pow(4,0.5)", 2.0},
    };

    for (int i = 0; i < 8; i++)
    {
        int success;
        double result = test_expression(tests[i].expr, &success);
        TEST_ASSERT(success, "Expression should parse successfully");
        TEST_ASSERT_DOUBLE_EQ(result, tests[i].expected, tests[i].expr);
        printf("  ✅ %s = %g\n", tests[i].expr, result);
    }

    return 1;
}

int test_other_functions()
{
    printf("Testing other mathematical functions...\n");

    struct
    {
        const char *expr;
        double expected;
    } tests[] = {
        {"abs(-5)", 5.0},
        {"abs(3.2)", 3.2},
        {"floor(3.7)", 3.0},
        {"floor(-2.3)", -3.0},
        {"ceil(3.2)", 4.0},
        {"ceil(-2.8)", -2.0},
        {"sinh(0)", 0.0},
        {"cosh(0)", 1.0},
        {"tanh(0)", 0.0},
    };

    for (int i = 0; i < 9; i++)
    {
        int success;
        double result = test_expression(tests[i].expr, &success);
        TEST_ASSERT(success, "Expression should parse successfully");
        TEST_ASSERT_DOUBLE_EQ(result, tests[i].expected, tests[i].expr);
        printf("  ✅ %s = %g\n", tests[i].expr, result);
    }

    return 1;
}

int test_comparison_operators()
{
    printf("Testing comparison operators...\n");

    struct
    {
        const char *expr;
        double expected;
    } tests[] = {
        {"5 > 3", 1.0},
        {"2 < 1", 0.0},
        {"4 == 4", 1.0},
        {"3 != 3", 0.0},
        {"5 >= 5", 1.0},
        {"3 <= 2", 0.0},
        {"(2+2) == 4", 1.0},
        {"sin(0) == 0", 1.0},
    };

    for (int i = 0; i < 8; i++)
    {
        int success;
        double result = test_expression(tests[i].expr, &success);
        TEST_ASSERT(success, "Expression should parse successfully");
        TEST_ASSERT_DOUBLE_EQ(result, tests[i].expected, tests[i].expr);
        printf("  ✅ %s = %g\n", tests[i].expr, result);
    }

    return 1;
}

int test_complex_expressions()
{
    printf("Testing complex expressions...\n");

    struct
    {
        const char *expr;
        double expected;
    } tests[] = {
        {"sqrt(pow(3,2) + pow(4,2))", 5.0},   // Pythagorean theorem
        {"2*pi*sqrt(2)", 2 * M_PI * sqrt(2)}, // Using constants
        {"log(exp(2))", 2.0},                 // log(exp(x)) = x
        {"sin(pi/6) * 2", 1.0},               // sin(30°) * 2
        {"abs(-3) + sqrt(16)", 7.0},          // Multiple functions
        {"(1+2)*(3+4)/(5+6)", 21.0 / 11.0},   // Complex arithmetic
    };

    for (int i = 0; i < 6; i++)
    {
        int success;
        double result = test_expression(tests[i].expr, &success);
        TEST_ASSERT(success, "Expression should parse successfully");
        TEST_ASSERT_DOUBLE_EQ(result, tests[i].expected, tests[i].expr);
        printf("  ✅ %s = %g\n", tests[i].expr, result);
    }

    return 1;
}

int test_error_cases()
{
    printf("Testing error cases...\n");

    const char *invalid_exprs[] = {
        "(2+3",       // Unmatched parenthesis
        "2+3)",       // Unmatched parenthesis
        "sin()",      // Missing argument
        "atan2(1)",   // Wrong number of arguments
        "unknown(1)", // Unknown function
        "",           // Empty expression
        ".",          // Just a dot
        "2+",         // Incomplete expression
        "*5",         // Starting with operator (multiply)
        "/5",         // Starting with operator (divide)
        "^5",         // Starting with operator (power)
        "2*/3",       // Invalid operator sequence
        "sin cos",    // Function without parentheses followed by another function
    };

    for (int i = 0; i < 13; i++)
    {
        int success;
        test_expression(invalid_exprs[i], &success);

        if (success)
        {
            printf("  ⚠️  Expression '%s' unexpectedly parsed successfully\n", invalid_exprs[i]);
            // Let's see what it evaluates to for debugging
            mpfr_t result;
            mpfr_init2(result, global_precision);
            test_expression_mpfr(invalid_exprs[i], result);
            printf("      Result: ");
            mpfr_printf("%.6Rf\n", result);
            mpfr_clear(result);
        }

        TEST_ASSERT(!success, "Invalid expression should fail to parse");
        printf("  ✅ Correctly rejected: '%s'\n", invalid_exprs[i]);
    }

    return 1;
}

int test_domain_errors()
{
    printf("Testing domain error handling...\n");

    // These should parse but evaluate to 0 with error messages
    const char *domain_error_exprs[] = {
        "sqrt(-1)", // Square root of negative
        "log(0)",   // Log of zero
        "log(-1)",  // Log of negative
        "asin(2)",  // asin out of domain
        "acos(-2)", // acos out of domain
        "acosh(0)", // acosh out of domain
        "atanh(1)", // atanh out of domain
        "1/0",      // Division by zero
    };

    for (int i = 0; i < 8; i++)
    {
        int success;
        double result = test_expression(domain_error_exprs[i], &success);
        TEST_ASSERT(success, "Should parse successfully");
        // Note: We don't check the result value since error handling returns 0
        // but we verify it doesn't crash
        (void)result; // Suppress unused warning
        printf("  ✅ Handled domain error: '%s'\n", domain_error_exprs[i]);
    }

    return 1;
}

int test_precision_changes()
{
    printf("Testing precision changes...\n");

    // Save current precision
    mpfr_prec_t original_precision = global_precision;

    // Test different precisions
    mpfr_prec_t test_precisions[] = {53, 128, 256, 512};

    for (int i = 0; i < 4; i++)
    {
        set_precision(test_precisions[i]);
        TEST_ASSERT(get_precision() == test_precisions[i], "Precision should be set correctly");

        // Test that calculations work at this precision
        mpfr_t result;
        mpfr_init2(result, global_precision);

        int success = test_expression_mpfr("pi", result);
        TEST_ASSERT(success, "Should calculate pi at any precision");

        // Verify the precision of the result
        TEST_ASSERT(mpfr_get_prec(result) == test_precisions[i], "Result should have correct precision");

        printf("  ✅ Precision %ld bits working correctly\n", (long)test_precisions[i]);

        mpfr_clear(result);
    }

    // Restore original precision
    set_precision(original_precision);

    return 1;
}
