#include "precision.h"
#include "constants.h"
#include "evaluator.h"
#include "parser.h"
#include "functions.h"
#include "function_table.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>

#define TEST_ASSERT(condition, message)         \
    do                                          \
    {                                           \
        if (!(condition))                       \
        {                                       \
            printf("  ❌ FAIL: %s\n", message); \
            return 0;                           \
        }                                       \
    } while (0)

#define MPFR_EPSILON 1e-15
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

// Helper function to evaluate expression with MPFR
int eval_expression_mpfr(const char *expr, mpfr_t result)
{
    Lexer lexer;
    lexer_init(&lexer, expr);

    Parser parser;
    parser_init(&parser, &lexer);

    ASTNode *ast = parser_parse_expression(&parser);

    if (!ast || parser_has_error(&parser) || parser.current_token.type != TOKEN_EOF)
    {
        ast_free(ast);
        return 0;
    }

    evaluator_eval(result, ast);
    ast_free(ast);
    return 1;
}

int test_precision_setting(void)
{
    printf("Testing precision setting...\n");

    // Test default precision
    TEST_ASSERT(get_precision() == DEFAULT_PRECISION, "Should start with default precision");

    // Test setting valid precision
    set_precision(128);
    TEST_ASSERT(get_precision() == 128, "Should set precision to 128");

    set_precision(512);
    TEST_ASSERT(get_precision() == 512, "Should set precision to 512");

    // Test clamping to valid range
    set_precision(1); // Too small
    TEST_ASSERT(get_precision() == MIN_PRECISION, "Should clamp to minimum");

    set_precision(100000); // Too large
    TEST_ASSERT(get_precision() == MAX_PRECISION, "Should clamp to maximum");

    // Reset to default
    set_precision(DEFAULT_PRECISION);

    printf("  ✅ Precision setting tests passed\n");
    return 1;
}

int test_precision_decimal_digits(void)
{
    printf("Testing decimal digit calculation...\n");

    set_precision(53); // Double precision
    long digits = get_decimal_digits();
    TEST_ASSERT(digits >= 15 && digits <= 17, "53 bits should give ~16 decimal digits");

    set_precision(128);
    digits = get_decimal_digits();
    TEST_ASSERT(digits >= 38 && digits <= 40, "128 bits should give ~39 decimal digits");

    set_precision(256);
    digits = get_decimal_digits();
    TEST_ASSERT(digits >= 77 && digits <= 78, "256 bits should give ~77 decimal digits");

    // Reset to default
    set_precision(DEFAULT_PRECISION);

    printf("  ✅ Decimal digit calculation tests passed\n");
    return 1;
}

int test_high_precision_parsing(void)
{
    printf("Testing high precision number parsing...\n");

    // Set higher precision for these tests
    mpfr_prec_t old_precision = get_precision();
    set_precision(256);

    // Test that very precise numbers are stored correctly
    const char *precise_numbers[] = {
        "1.000000000000000000000000000001",
        "1.23456789012345678901234567890",
        "1e-50",
        "1.234567890123456789e-25"};

    for (int i = 0; i < 4; i++)
    {
        mpfr_t result, expected;
        mpfr_init2(result, global_precision);
        mpfr_init2(expected, global_precision);

        // Parse with MPFR directly
        mpfr_set_str(expected, precise_numbers[i], 10, global_rounding);

        // Parse with our system
        int success = eval_expression_mpfr(precise_numbers[i], result);
        TEST_ASSERT(success, "Should parse successfully");

        // Compare results
        TEST_ASSERT_MPFR_EQ(result, expected, "High precision parsing");

        mpfr_clear(result);
        mpfr_clear(expected);
    }

    // Restore original precision
    set_precision(old_precision);

    printf("  ✅ High precision parsing tests passed\n");
    return 1;
}

int test_high_precision_arithmetic(void)
{
    printf("Testing high precision arithmetic...\n");

    // Set higher precision for these tests
    mpfr_prec_t old_precision = get_precision();
    set_precision(256);

    mpfr_t result, expected, one, tiny;
    mpfr_init2(result, global_precision);
    mpfr_init2(expected, global_precision);
    mpfr_init2(one, global_precision);
    mpfr_init2(tiny, global_precision);

    // Test 1: 1 + 1e-30
    mpfr_set_d(one, 1.0, global_rounding);
    mpfr_set_str(tiny, "1e-30", 10, global_rounding);
    mpfr_add(expected, one, tiny, global_rounding);

    int success = eval_expression_mpfr("1+1e-30", result);
    TEST_ASSERT(success, "Should parse 1+1e-30");
    TEST_ASSERT_MPFR_EQ(result, expected, "1+1e-30 high precision");

    // Test 2: Very precise pi calculation
    constants_get_pi(expected);
    success = eval_expression_mpfr("pi", result);
    TEST_ASSERT(success, "Should parse pi");
    TEST_ASSERT_MPFR_EQ(result, expected, "High precision pi");

    // Test 3: High precision square root
    mpfr_set_d(one, 2.0, global_rounding);
    mpfr_sqrt(expected, one, global_rounding);
    success = eval_expression_mpfr("sqrt(2)", result);
    TEST_ASSERT(success, "Should parse sqrt(2)");
    TEST_ASSERT_MPFR_EQ(result, expected, "High precision sqrt(2)");

    mpfr_clear(result);
    mpfr_clear(expected);
    mpfr_clear(one);
    mpfr_clear(tiny);

    // Restore original precision
    set_precision(old_precision);

    printf("  ✅ High precision arithmetic tests passed\n");
    return 1;
}

int test_precision_constants_cache(void)
{
    printf("Testing precision constant computation...\n");

    // --- Step 1: Set initial precision to 128 and compute pi ---
    set_precision(128);

    mpfr_t pi1;
    mpfr_init2(pi1, global_precision); // Use 128-bit precision
    constants_get_pi(pi1);

    // --- Step 2: Change precision to 256 and compute pi again ---
    set_precision(256);

    mpfr_t pi2;
    mpfr_init2(pi2, global_precision); // Use 256-bit precision
    constants_get_pi(pi2);

    // --- Step 3: Confirm that pi1 and pi2 differ ---
    // (pi2 should have more precision than pi1)
    TEST_ASSERT(mpfr_cmp(pi1, pi2) != 0, "Pi values should differ at different precisions");

    // --- Step 4: Verify that both are reasonable approximations of pi ---
    TEST_ASSERT(mpfr_cmp_d(pi1, 3.0) > 0 && mpfr_cmp_d(pi1, 3.2) < 0,
                "Pi at 128-bit should be approximately 3.14159...");
    TEST_ASSERT(mpfr_cmp_d(pi2, 3.0) > 0 && mpfr_cmp_d(pi2, 3.2) < 0,
                "Pi at 256-bit should be approximately 3.14159...");

    // --- Step 5: Cleanup ---
    mpfr_clear(pi1);
    mpfr_clear(pi2);

    // --- Step 6: Reset global precision ---
    set_precision(DEFAULT_PRECISION);

    printf("  ✅ Constant computation tests passed\n");
    return 1;
}

int test_precision_changes_during_evaluation(void)
{
    printf("Testing precision changes during evaluation...\n");

    // Test different precisions
    mpfr_prec_t test_precisions[] = {53, 128, 256, 512};

    for (int i = 0; i < 4; i++)
    {
        set_precision(test_precisions[i]);
        TEST_ASSERT(get_precision() == test_precisions[i], "Precision should be set correctly");

        // Test that calculations work at this precision
        mpfr_t result;
        mpfr_init2(result, global_precision);

        int success = eval_expression_mpfr("pi", result);
        TEST_ASSERT(success, "Should calculate pi at any precision");

        // Verify the precision of the result
        TEST_ASSERT(mpfr_get_prec(result) == test_precisions[i], "Result should have correct precision");

        mpfr_clear(result);
    }

    // Reset to default
    set_precision(DEFAULT_PRECISION);

    printf("  ✅ Precision change tests passed\n");
    return 1;
}

int test_precision_edge_cases(void)
{
    printf("Testing precision edge cases...\n");

    // Test very small numbers
    set_precision(512); // High precision for this test

    mpfr_t result;
    mpfr_init2(result, global_precision);

    int success = eval_expression_mpfr("1e-100", result);
    TEST_ASSERT(success, "Should handle very small numbers");
    TEST_ASSERT(!mpfr_zero_p(result), "Very small number should not be zero");

    // Test very large numbers
    success = eval_expression_mpfr("1e100", result);
    TEST_ASSERT(success, "Should handle very large numbers");
    TEST_ASSERT(!mpfr_inf_p(result), "Very large number should not be infinity");

    // Test precision near machine limits
    success = eval_expression_mpfr("1 + 1e-150", result);
    TEST_ASSERT(success, "Should handle numbers near precision limits");

    mpfr_clear(result);

    // Reset precision
    set_precision(DEFAULT_PRECISION);

    printf("  ✅ Precision edge case tests passed\n");
    return 1;
}

int run_precision_tests(void)
{
    printf("Running Precision Test Suite\n");
    printf("============================\n\n");

    // Initialize required systems
    precision_init();
    constants_init();
    functions_init();
    function_table_init();

    int passed = 0;
    int total = 0;

    total++;
    if (test_precision_setting())
        passed++;
    total++;
    if (test_precision_decimal_digits())
        passed++;
    total++;
    if (test_high_precision_parsing())
        passed++;
    total++;
    if (test_high_precision_arithmetic())
        passed++;
    total++;
    if (test_precision_constants_cache())
        passed++;
    total++;
    if (test_precision_changes_during_evaluation())
        passed++;
    total++;
    if (test_precision_edge_cases())
        passed++;

    printf("\n============================\n");
    printf("Precision Tests: %d/%d passed\n", passed, total);

    // Cleanup
    constants_cleanup();
    functions_cleanup();
    precision_cleanup();

    return (passed == total) ? 0 : 1;
}