#include "evaluator.h"
#include "parser.h"
#include "lexer.h"
#include "precision.h"
#include "constants.h"
#include "functions.h"
#include "function_table.h"  // Added for function_table_init()
#include <stdio.h>
#include <math.h>

#define EPSILON 1e-9
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("  ❌ FAIL: %s\n", message); \
            return 0; \
        } \
    } while(0)

#define TEST_ASSERT_DOUBLE_EQ(actual, expected, message) \
    do { \
        if (fabs((actual) - (expected)) >= EPSILON) { \
            printf("  ❌ FAIL: %s (expected %g, got %g)\n", message, expected, actual); \
            return 0; \
        } \
    } while(0)

// Helper function to evaluate an expression
double eval_test_expression(const char *input, int *success)
{
    *success = 0;
    
    Lexer lexer;
    lexer_init(&lexer, input);
    
    Parser parser;
    parser_init(&parser, &lexer);
    
    ASTNode *ast = parser_parse_expression(&parser);
    
    if (!ast || parser_has_error(&parser) || parser.current_token.type != TOKEN_EOF) {
        ast_free(ast);
        return 0.0;
    }
    
    mpfr_t result;
    mpfr_init2(result, global_precision);
    evaluator_eval(result, ast);
    
    double double_result = mpfr_get_d(result, global_rounding);
    mpfr_clear(result);
    ast_free(ast);
    
    *success = 1;
    return double_result;
}

int test_evaluator_basic_arithmetic(void)
{
    printf("Testing basic arithmetic evaluation...\n");
    
    struct {
        const char *expr;
        double expected;
    } tests[] = {
        {"2+3", 5.0},
        {"5-2", 3.0},
        {"3*4", 12.0},
        {"8/2", 4.0},
        {"2^3", 8.0},
        {"2+3*4", 14.0},        // Precedence
        {"(2+3)*4", 20.0},      // Parentheses
        {"2^3^2", 512.0},       // Right associative
        {"-5", -5.0},           // Unary minus
        {"+7", 7.0},            // Unary plus
        {"--3", 3.0},           // Double negative
    };
    
    for (int i = 0; i < 11; i++) {
        int success;
        double result = eval_test_expression(tests[i].expr, &success);
        TEST_ASSERT(success, "Expression should evaluate successfully");
        TEST_ASSERT_DOUBLE_EQ(result, tests[i].expected, tests[i].expr);
    }
    
    printf("  ✅ Basic arithmetic evaluation tests passed\n");
    return 1;
}

int test_evaluator_trigonometric_functions(void)
{
    printf("Testing trigonometric function evaluation...\n");
    
    struct {
        const char *expr;
        double expected;
    } tests[] = {
        {"sin(0)", 0.0},
        {"cos(0)", 1.0},
        {"tan(0)", 0.0},
        {"sin(pi/2)", 1.0},
        {"cos(pi)", -1.0},
        {"asin(1)", M_PI/2},
        {"acos(1)", 0.0},
        {"atan(1)", M_PI/4},
        {"atan2(1,1)", M_PI/4},
    };
    
    for (int i = 0; i < 9; i++) {
        int success;
        double result = eval_test_expression(tests[i].expr, &success);
        TEST_ASSERT(success, "Expression should evaluate successfully");
        TEST_ASSERT_DOUBLE_EQ(result, tests[i].expected, tests[i].expr);
    }
    
    printf("  ✅ Trigonometric function tests passed\n");
    return 1;
}

int test_evaluator_logarithmic_functions(void)
{
    printf("Testing logarithmic and exponential functions...\n");
    
    struct {
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
    
    for (int i = 0; i < 8; i++) {
        int success;
        double result = eval_test_expression(tests[i].expr, &success);
        TEST_ASSERT(success, "Expression should evaluate successfully");
        TEST_ASSERT_DOUBLE_EQ(result, tests[i].expected, tests[i].expr);
    }
    
    printf("  ✅ Logarithmic function tests passed\n");
    return 1;
}

int test_evaluator_other_functions(void)
{
    printf("Testing other mathematical functions...\n");
    
    struct {
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
    
    for (int i = 0; i < 9; i++) {
        int success;
        double result = eval_test_expression(tests[i].expr, &success);
        TEST_ASSERT(success, "Expression should evaluate successfully");
        TEST_ASSERT_DOUBLE_EQ(result, tests[i].expected, tests[i].expr);
    }
    
    printf("  ✅ Other function tests passed\n");
    return 1;
}

int test_evaluator_comparison_operators(void)
{
    printf("Testing comparison operators...\n");
    
    struct {
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
    
    for (int i = 0; i < 8; i++) {
        int success;
        double result = eval_test_expression(tests[i].expr, &success);
        TEST_ASSERT(success, "Expression should evaluate successfully");
        TEST_ASSERT_DOUBLE_EQ(result, tests[i].expected, tests[i].expr);
    }
    
    printf("  ✅ Comparison operator tests passed\n");
    return 1;
}

int test_evaluator_constants(void)
{
    printf("Testing mathematical constants...\n");
    
    // Test pi
    int success;
    double result = eval_test_expression("pi", &success);
    TEST_ASSERT(success, "Should evaluate pi");
    TEST_ASSERT_DOUBLE_EQ(result, M_PI, "pi value");
    
    // Test e
    result = eval_test_expression("e", &success);
    TEST_ASSERT(success, "Should evaluate e");
    TEST_ASSERT_DOUBLE_EQ(result, M_E, "e value");
    
    // Test constants in expressions
    result = eval_test_expression("2*pi", &success);
    TEST_ASSERT(success, "Should evaluate 2*pi");
    TEST_ASSERT_DOUBLE_EQ(result, 2 * M_PI, "2*pi value");
    
    printf("  ✅ Constant evaluation tests passed\n");
    return 1;
}

int test_evaluator_complex_expressions(void)
{
    printf("Testing complex expression evaluation...\n");
    
    struct {
        const char *expr;
        double expected;
    } tests[] = {
        {"sqrt(pow(3,2) + pow(4,2))", 5.0},    // Pythagorean theorem
        {"2*pi*sqrt(2)", 2 * M_PI * sqrt(2)}, // Using constants
        {"log(exp(2))", 2.0},                 // log(exp(x)) = x
        {"sin(pi/6) * 2", 1.0},               // sin(30°) * 2
        {"abs(-3) + sqrt(16)", 7.0},          // Multiple functions
        {"(1+2)*(3+4)/(5+6)", 21.0/11.0},    // Complex arithmetic
    };
    
    for (int i = 0; i < 6; i++) {
        int success;
        double result = eval_test_expression(tests[i].expr, &success);
        TEST_ASSERT(success, "Expression should evaluate successfully");
        TEST_ASSERT_DOUBLE_EQ(result, tests[i].expected, tests[i].expr);
    }
    
    printf("  ✅ Complex expression tests passed\n");
    return 1;
}

int test_evaluator_domain_errors(void)
{
    printf("Testing domain error handling...\n");
    
    // These should parse but may produce domain errors
    const char *domain_error_exprs[] = {
        "sqrt(-1)",     // Square root of negative
        "log(0)",       // Log of zero
        "log(-1)",      // Log of negative
        "asin(2)",      // asin out of domain
        "acos(-2)",     // acos out of domain
        "acosh(0)",     // acosh out of domain
        "atanh(1)",     // atanh out of domain
        "1/0",          // Division by zero
    };
    
    for (int i = 0; i < 8; i++) {
        int success;
        eval_test_expression(domain_error_exprs[i], &success);
        TEST_ASSERT(success, "Should parse successfully even with domain errors");
        // Note: We don't check the result value since error handling varies
    }
    
    printf("  ✅ Domain error handling tests passed\n");
    return 1;
}

int test_evaluator_implicit_multiplication(void)
{
    printf("Testing implicit multiplication evaluation...\n");
    
    struct {
        const char *expr;
        double expected;
    } tests[] = {
        {"2(3+4)", 14.0},
        {"(2+1)(3+1)", 12.0},
        {"2pi", 2 * M_PI},
        {"3e", 3 * M_E},
        {"pi(2)", 2 * M_PI},
        {"(2)(3)(4)", 24.0},
        {"2sin(0)", 0.0},       // 2 * sin(0)
    };
    
    for (int i = 0; i < 7; i++) {
        int success;
        double result = eval_test_expression(tests[i].expr, &success);
        TEST_ASSERT(success, "Expression should evaluate successfully");
        TEST_ASSERT_DOUBLE_EQ(result, tests[i].expected, tests[i].expr);
    }
    
    printf("  ✅ Implicit multiplication tests passed\n");
    return 1;
}

int run_evaluator_tests(void)
{
    printf("Running Evaluator Test Suite\n");
    printf("============================\n\n");
    
    // Initialize required systems
    precision_init();
    constants_init();
    functions_init();
    function_table_init();
    
    int passed = 0;
    int total = 0;
    
    total++; if (test_evaluator_basic_arithmetic()) passed++;
    total++; if (test_evaluator_trigonometric_functions()) passed++;
    total++; if (test_evaluator_logarithmic_functions()) passed++;
    total++; if (test_evaluator_other_functions()) passed++;
    total++; if (test_evaluator_comparison_operators()) passed++;
    total++; if (test_evaluator_constants()) passed++;
    total++; if (test_evaluator_complex_expressions()) passed++;
    total++; if (test_evaluator_domain_errors()) passed++;
    total++; if (test_evaluator_implicit_multiplication()) passed++;
    
    printf("\n============================\n");
    printf("Evaluator Tests: %d/%d passed\n", passed, total);
    
    // Cleanup
    constants_cleanup();
    functions_cleanup();
    precision_cleanup();
    
    return (passed == total) ? 0 : 1;
}