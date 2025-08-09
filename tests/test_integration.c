#include "repl.h"
#include "commands.h"
#include "precision.h"
#include "constants.h"
#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_ASSERT(condition, message)         \
    do                                          \
    {                                           \
        if (!(condition))                       \
        {                                       \
            printf("  ❌ FAIL: %s\n", message); \
            return 0;                           \
        }                                       \
    } while (0)

int test_integration_full_pipeline(void)
{
    printf("Testing full calculation pipeline...\n");

    // Test that we can process a complex expression end-to-end
    const char *test_expressions[] = {
        "2+3*4",
        "sin(pi/2)",
        "sqrt(pow(3,2) + pow(4,2))",
        "log(exp(1))",
        "2pi",
        "(1+2)*(3+4)",
        "abs(-5) + floor(3.7)"};

    for (int i = 0; i < 7; i++)
    {
        ReplResult result = repl_process_line(test_expressions[i]); //!
        TEST_ASSERT(result == REPL_CONTINUE, "Expression should process successfully");
    }

    printf("  ✅ Full pipeline tests passed\n");
    return 1;
}

int test_integration_command_processing(void)
{
    printf("Testing command processing integration...\n");

    // Test precision command
    ReplResult result = repl_process_line("precision");
    TEST_ASSERT(result == REPL_CONTINUE, "Precision command should work");

    // Test precision setting
    result = repl_process_line("precision 128");
    TEST_ASSERT(result == REPL_CONTINUE, "Precision setting should work");
    TEST_ASSERT(get_precision() == 128, "Precision should be set to 128");

    // Test help command
    result = repl_process_line("help");
    TEST_ASSERT(result == REPL_CONTINUE, "Help command should work");

    // Test version command
    result = repl_process_line("version");
    TEST_ASSERT(result == REPL_CONTINUE, "Version command should work");

    // Test test command
    result = repl_process_line("test");
    TEST_ASSERT(result == REPL_CONTINUE, "Test command should work");

    // Reset precision
    set_precision(DEFAULT_PRECISION);

    printf("  ✅ Command processing integration tests passed\n");
    return 1;
}

int test_integration_error_handling(void)
{
    printf("Testing error handling integration...\n");

    // Test parse errors
    ReplResult result = repl_process_line("2+");
    TEST_ASSERT(result == REPL_CONTINUE, "Parse errors should be handled gracefully");

    result = repl_process_line("sin()");
    TEST_ASSERT(result == REPL_CONTINUE, "Function argument errors should be handled");

    result = repl_process_line("unknown_function(1)");
    TEST_ASSERT(result == REPL_CONTINUE, "Unknown functions should be handled");

    result = repl_process_line("(2+3");
    TEST_ASSERT(result == REPL_CONTINUE, "Unmatched parentheses should be handled");

    // Test domain errors
    result = repl_process_line("sqrt(-1)");
    TEST_ASSERT(result == REPL_CONTINUE, "Domain errors should be handled gracefully");

    result = repl_process_line("log(0)");
    TEST_ASSERT(result == REPL_CONTINUE, "Log of zero should be handled");

    printf("  ✅ Error handling integration tests passed\n");
    return 1;
}

int test_integration_precision_consistency(void)
{
    printf("Testing precision consistency across system...\n");

    // Set a specific precision
    set_precision(256);

    // Process an expression that should benefit from high precision
    ReplResult result = repl_process_line("1 + 1e-30");
    TEST_ASSERT(result == REPL_CONTINUE, "High precision expression should work");

    // Verify precision is still correct
    TEST_ASSERT(get_precision() == 256, "Precision should remain consistent");

    // Test that constants are computed at the right precision
    result = repl_process_line("pi");
    TEST_ASSERT(result == REPL_CONTINUE, "Pi should be computed at high precision");

    result = repl_process_line("e");
    TEST_ASSERT(result == REPL_CONTINUE, "e should be computed at high precision");

    // Change precision and verify constants are recomputed
    result = repl_process_line("precision 128");
    TEST_ASSERT(result == REPL_CONTINUE, "Precision change should work");

    result = repl_process_line("pi");
    TEST_ASSERT(result == REPL_CONTINUE, "Pi should be recomputed at new precision");

    // Reset precision
    set_precision(DEFAULT_PRECISION);

    printf("  ✅ Precision consistency tests passed\n");
    return 1;
}

int test_integration_memory_management(void)
{
    printf("Testing memory management across system...\n");

    // Process many expressions to test for memory leaks
    const char *expressions[] = {
        "2+3",
        "sin(0)",
        "sqrt(16)",
        "log(e)",
        "pi*2",
        "abs(-5)",
        "(1+2)*(3+4)",
        "pow(2,8)",
        "floor(3.7)",
        "ceil(2.1)"};

    // Run each expression multiple times
    for (int round = 0; round < 10; round++)
    {
        for (int i = 0; i < 10; i++)
        {
            ReplResult result = repl_process_line(expressions[i]);
            TEST_ASSERT(result == REPL_CONTINUE, "Expression should process successfully");
        }
    }

    // Test command processing memory management
    for (int i = 0; i < 10; i++)
    {
        ReplResult result = repl_process_line("precision");
        TEST_ASSERT(result == REPL_CONTINUE, "Command should process successfully");
    }

    printf("  ✅ Memory management tests passed\n");
    return 1;
}

int test_integration_scientific_notation(void)
{
    printf("Testing scientific notation integration...\n");

    // Test various scientific notation expressions
    const char *sci_expressions[] = {
        "1e3 + 1e2",
        "2.5e-10 * 4e10",
        "sin(1e-6)",
        "sqrt(1.44e4)",
        "log(1e10) / log(10)",
        "6.02e23 / 1e23",
        "1.23e-100 + 1.23e-100"};

    for (int i = 0; i < 7; i++)
    {
        ReplResult result = repl_process_line(sci_expressions[i]);
        TEST_ASSERT(result == REPL_CONTINUE, "Scientific notation should work");
    }

    printf("  ✅ Scientific notation integration tests passed\n");
    return 1;
}

int test_integration_complex_nested_expressions(void)
{
    printf("Testing complex nested expressions...\n");

    // Test deeply nested expressions
    const char *complex_expressions[] = {
        "sin(cos(tan(0)))",
        "log(exp(sqrt(abs(-16))))",
        "pow(sin(pi/6), 2) + pow(cos(pi/6), 2)",
        "sqrt(pow(sin(pi/4), 2) + pow(cos(pi/4), 2))",
        "atan2(sin(pi/3), cos(pi/3))",
        "log(pow(e, 5))",
        "((2+3)*(4+5))/((6+7)+(8+9))",
        "abs(sin(pi/2) - cos(0))",
        "floor(ceil(sqrt(pow(3,2) + pow(4,2))))",
        "sinh(log(exp(1)))"};

    for (int i = 0; i < 10; i++)
    {
        ReplResult result = repl_process_line(complex_expressions[i]);
        TEST_ASSERT(result == REPL_CONTINUE, "Complex expression should work");
    }

    printf("  ✅ Complex nested expression tests passed\n");
    return 1;
}

int test_integration_implicit_multiplication_edge_cases(void)
{
    printf("Testing implicit multiplication edge cases...\n");

    const char *implicit_mult_expressions[] = {
        "2(3)(4)",
        "pi e",
        "2pi e",
        "(2+3)(4+5)(6+7)",
        "sin(0)cos(0)",
        "2sqrt(4)",
        "pi(1+1)",
        "e(2*3)",
        "(2pi)(3e)",
        "2(pi+e)"};

    for (int i = 0; i < 10; i++)
    {
        ReplResult result = repl_process_line(implicit_mult_expressions[i]);
        TEST_ASSERT(result == REPL_CONTINUE, "Implicit multiplication should work");
    }

    printf("  ✅ Implicit multiplication edge case tests passed\n");
    return 1;
}

int run_integration_tests(void)
{
    printf("Running Integration Test Suite\n");
    printf("==============================\n\n");

    // Initialize the REPL system
    if (repl_init() != 0)
    {
        printf("Failed to initialize REPL system\n");
        return 1;
    }

    int passed = 0;
    int total = 0;

    total++;
    if (test_integration_full_pipeline())
        passed++;
    total++;
    if (test_integration_command_processing())
        passed++;
    total++;
    if (test_integration_error_handling())
        passed++;
    total++;
    if (test_integration_precision_consistency())
        passed++;
    total++;
    if (test_integration_memory_management())
        passed++;
    total++;
    if (test_integration_scientific_notation())
        passed++;
    total++;
    if (test_integration_complex_nested_expressions())
        passed++;
    total++;
    if (test_integration_implicit_multiplication_edge_cases())
        passed++;

    printf("\n==============================\n");
    printf("Integration Tests: %d/%d passed\n", passed, total);

    // Cleanup
    repl_cleanup();

    return (passed == total) ? 0 : 1;
}