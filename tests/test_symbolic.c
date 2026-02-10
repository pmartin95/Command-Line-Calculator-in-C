#include "symbolic.h"
#include "parser.h"
#include "lexer.h"
#include "precision.h"
#include "constants.h"
#include "functions.h"
#include "function_table.h"
#include "printer.h"
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

// Helper: parse expression string into AST
static ASTNode *parse_expr(const char *input)
{
    Lexer lexer;
    lexer_init(&lexer, input);

    Parser parser;
    parser_init(&parser, &lexer);

    ASTNode *ast = parser_parse_expression(&parser);
    if (!ast || parser_has_error(&parser))
    {
        ast_free(ast);
        return NULL;
    }
    return ast;
}

// Helper: symbolically evaluate expression and return infix string (caller must free)
static char *symbolic_eval_str(const char *input)
{
    ASTNode *ast = parse_expr(input);
    if (!ast)
        return NULL;

    ASTNode *simplified = symbolic_eval(ast);
    ast_free(ast);

    if (!simplified)
        return NULL;

    char *result = printer_ast_infix_to_string(simplified);
    ast_free(simplified);
    return result;
}

// Helper: assert symbolic evaluation produces expected output
static int assert_symbolic(const char *input, const char *expected, const char *test_name)
{
    char *result = symbolic_eval_str(input);
    if (!result)
    {
        printf("  ❌ FAIL: %s — symbolic_eval returned NULL for '%s'\n", test_name, input);
        return 0;
    }

    if (strcmp(result, expected) != 0)
    {
        printf("  ❌ FAIL: %s\n", test_name);
        printf("    Input:    %s\n", input);
        printf("    Expected: %s\n", expected);
        printf("    Got:      %s\n", result);
        free(result);
        return 0;
    }

    free(result);
    return 1;
}

// ---- Test suites ----

static int test_symbolic_clone_and_equals(void)
{
    printf("Testing symbolic clone and equals...\n");

    // Clone a number
    ASTNode *num = parse_expr("42");
    TEST_ASSERT(num != NULL, "parse '42'");
    ASTNode *clone = symbolic_clone(num);
    TEST_ASSERT(clone != NULL, "clone '42'");
    TEST_ASSERT(symbolic_equals(num, clone), "42 equals its clone");
    ast_free(num);
    ast_free(clone);

    // Clone a constant
    ASTNode *pi = parse_expr("pi");
    TEST_ASSERT(pi != NULL, "parse 'pi'");
    clone = symbolic_clone(pi);
    TEST_ASSERT(clone != NULL, "clone 'pi'");
    TEST_ASSERT(symbolic_equals(pi, clone), "pi equals its clone");
    ast_free(pi);
    ast_free(clone);

    // Clone a binop
    ASTNode *binop = parse_expr("2+3");
    TEST_ASSERT(binop != NULL, "parse '2+3'");
    clone = symbolic_clone(binop);
    TEST_ASSERT(clone != NULL, "clone '2+3'");
    TEST_ASSERT(symbolic_equals(binop, clone), "2+3 equals its clone");
    ast_free(binop);
    ast_free(clone);

    // Clone a function call
    ASTNode *func = parse_expr("sin(pi)");
    TEST_ASSERT(func != NULL, "parse 'sin(pi)'");
    clone = symbolic_clone(func);
    TEST_ASSERT(clone != NULL, "clone 'sin(pi)'");
    TEST_ASSERT(symbolic_equals(func, clone), "sin(pi) equals its clone");
    ast_free(func);
    ast_free(clone);

    // Non-equal nodes
    ASTNode *a = parse_expr("2+3");
    ASTNode *b = parse_expr("2*3");
    TEST_ASSERT(!symbolic_equals(a, b), "2+3 != 2*3");
    ast_free(a);
    ast_free(b);

    // NULL handling
    TEST_ASSERT(symbolic_equals(NULL, NULL), "NULL equals NULL");
    ASTNode *n = parse_expr("1");
    TEST_ASSERT(!symbolic_equals(n, NULL), "1 != NULL");
    TEST_ASSERT(!symbolic_equals(NULL, n), "NULL != 1");
    ast_free(n);

    // Clone NULL
    TEST_ASSERT(symbolic_clone(NULL) == NULL, "clone NULL is NULL");

    printf("  ✅ Clone and equals tests passed\n");
    return 1;
}

static int test_symbolic_predicates(void)
{
    printf("Testing symbolic predicates (is_zero, is_one, is_integer)...\n");

    ASTNode *zero = parse_expr("0");
    ASTNode *one = parse_expr("1");
    ASTNode *two = parse_expr("2");
    ASTNode *pi = parse_expr("pi");

    TEST_ASSERT(symbolic_is_zero(zero), "0 is zero");
    TEST_ASSERT(!symbolic_is_zero(one), "1 is not zero");
    TEST_ASSERT(!symbolic_is_zero(pi), "pi is not zero");
    TEST_ASSERT(!symbolic_is_zero(NULL), "NULL is not zero");

    TEST_ASSERT(symbolic_is_one(one), "1 is one");
    TEST_ASSERT(!symbolic_is_one(zero), "0 is not one");
    TEST_ASSERT(!symbolic_is_one(pi), "pi is not one");
    TEST_ASSERT(!symbolic_is_one(NULL), "NULL is not one");

    TEST_ASSERT(symbolic_is_integer(zero), "0 is integer");
    TEST_ASSERT(symbolic_is_integer(one), "1 is integer");
    TEST_ASSERT(symbolic_is_integer(two), "2 is integer");
    TEST_ASSERT(!symbolic_is_integer(pi), "pi is not integer");
    TEST_ASSERT(!symbolic_is_integer(NULL), "NULL is not integer");

    ast_free(zero);
    ast_free(one);
    ast_free(two);
    ast_free(pi);

    printf("  ✅ Predicate tests passed\n");
    return 1;
}

static int test_symbolic_addition(void)
{
    printf("Testing symbolic addition simplification...\n");

    // x + 0 → x
    TEST_ASSERT(assert_symbolic("pi+0", "π", "pi + 0 → pi"), "pi+0");

    // 0 + x → x
    TEST_ASSERT(assert_symbolic("0+pi", "π", "0 + pi → pi"), "0+pi");

    // Integer addition: 2 + 3 → 5
    TEST_ASSERT(assert_symbolic("2+3", "5", "2 + 3 → 5"), "2+3");

    // x + x → 2*x
    TEST_ASSERT(assert_symbolic("pi+pi", "2 × π", "pi + pi → 2*pi"), "pi+pi");

    printf("  ✅ Addition simplification tests passed\n");
    return 1;
}

static int test_symbolic_subtraction(void)
{
    printf("Testing symbolic subtraction simplification...\n");

    // x - 0 → x
    TEST_ASSERT(assert_symbolic("pi-0", "π", "pi - 0 → pi"), "pi-0");

    // x - x → 0
    TEST_ASSERT(assert_symbolic("pi-pi", "0", "pi - pi → 0"), "pi-pi");

    printf("  ✅ Subtraction simplification tests passed\n");
    return 1;
}

static int test_symbolic_multiplication(void)
{
    printf("Testing symbolic multiplication simplification...\n");

    // x * 0 → 0
    TEST_ASSERT(assert_symbolic("pi*0", "0", "pi * 0 → 0"), "pi*0");

    // 0 * x → 0
    TEST_ASSERT(assert_symbolic("0*pi", "0", "0 * pi → 0"), "0*pi");

    // x * 1 → x
    TEST_ASSERT(assert_symbolic("pi*1", "π", "pi * 1 → pi"), "pi*1");

    // 1 * x → x
    TEST_ASSERT(assert_symbolic("1*pi", "π", "1 * pi → pi"), "1*pi");

    // Integer multiplication: 3 * 4 → 12
    TEST_ASSERT(assert_symbolic("3*4", "12", "3 * 4 → 12"), "3*4");

    printf("  ✅ Multiplication simplification tests passed\n");
    return 1;
}

static int test_symbolic_division(void)
{
    printf("Testing symbolic division simplification...\n");

    // x / 1 → x
    TEST_ASSERT(assert_symbolic("pi/1", "π", "pi / 1 → pi"), "pi/1");

    // x / x → 1
    TEST_ASSERT(assert_symbolic("pi/pi", "1", "pi / pi → 1"), "pi/pi");

    // 0 / x → 0
    TEST_ASSERT(assert_symbolic("0/pi", "0", "0 / pi → 0"), "0/pi");

    // Exact integer division: 12 / 4 → 3
    TEST_ASSERT(assert_symbolic("12/4", "3", "12 / 4 → 3"), "12/4");

    // Non-exact integer division stays as fraction: 7 / 3 → 7÷3
    TEST_ASSERT(assert_symbolic("7/3", "7 ÷ 3", "7 / 3 stays as fraction"), "7/3");

    printf("  ✅ Division simplification tests passed\n");
    return 1;
}

static int test_symbolic_power(void)
{
    printf("Testing symbolic power simplification...\n");

    // x^0 → 1
    TEST_ASSERT(assert_symbolic("pi^0", "1", "pi^0 → 1"), "pi^0");

    // x^1 → x
    TEST_ASSERT(assert_symbolic("pi^1", "π", "pi^1 → pi"), "pi^1");

    // 0^x → 0
    TEST_ASSERT(assert_symbolic("0^5", "0", "0^5 → 0"), "0^5");

    // 1^x → 1
    TEST_ASSERT(assert_symbolic("1^5", "1", "1^5 → 1"), "1^5");

    // Small integer power: 2^3 → 8
    TEST_ASSERT(assert_symbolic("2^3", "8", "2^3 → 8"), "2^3");

    printf("  ✅ Power simplification tests passed\n");
    return 1;
}

static int test_symbolic_trig(void)
{
    printf("Testing symbolic trig simplification...\n");

    // sin(0) → 0
    TEST_ASSERT(assert_symbolic("sin(0)", "0", "sin(0) → 0"), "sin(0)");

    // sin(pi) → 0
    TEST_ASSERT(assert_symbolic("sin(pi)", "0", "sin(pi) → 0"), "sin(pi)");

    // cos(0) → 1
    TEST_ASSERT(assert_symbolic("cos(0)", "1", "cos(0) → 1"), "cos(0)");

    // cos(pi) → -1
    TEST_ASSERT(assert_symbolic("cos(pi)", "-1", "cos(pi) → -1"), "cos(pi)");

    // tan(0) → 0
    TEST_ASSERT(assert_symbolic("tan(0)", "0", "tan(0) → 0"), "tan(0)");

    // tan(pi) → 0
    TEST_ASSERT(assert_symbolic("tan(pi)", "0", "tan(pi) → 0"), "tan(pi)");

    printf("  ✅ Trig simplification tests passed\n");
    return 1;
}

static int test_symbolic_log_exp(void)
{
    printf("Testing symbolic log/exp simplification...\n");

    // log(1) → 0
    TEST_ASSERT(assert_symbolic("log(1)", "0", "log(1) → 0"), "log(1)");

    // log(e) → 1
    TEST_ASSERT(assert_symbolic("log(e)", "1", "log(e) → 1"), "log(e)");

    // log10(1) → 0
    TEST_ASSERT(assert_symbolic("log10(1)", "0", "log10(1) → 0"), "log10(1)");

    // log10(10) → 1
    TEST_ASSERT(assert_symbolic("log10(10)", "1", "log10(10) → 1"), "log10(10)");

    // exp(0) → 1
    TEST_ASSERT(assert_symbolic("exp(0)", "1", "exp(0) → 1"), "exp(0)");

    // exp(1) → e
    TEST_ASSERT(assert_symbolic("exp(1)", "e", "exp(1) → e"), "exp(1)");

    printf("  ✅ Log/exp simplification tests passed\n");
    return 1;
}

static int test_symbolic_sqrt(void)
{
    printf("Testing symbolic sqrt simplification...\n");

    // sqrt(0) → 0
    TEST_ASSERT(assert_symbolic("sqrt(0)", "0", "sqrt(0) → 0"), "sqrt(0)");

    // sqrt(1) → 1
    TEST_ASSERT(assert_symbolic("sqrt(1)", "1", "sqrt(1) → 1"), "sqrt(1)");

    // sqrt(4) → 2
    TEST_ASSERT(assert_symbolic("sqrt(4)", "2", "sqrt(4) → 2"), "sqrt(4)");

    // sqrt(9) → 3
    TEST_ASSERT(assert_symbolic("sqrt(9)", "3", "sqrt(9) → 3"), "sqrt(9)");

    printf("  ✅ Sqrt simplification tests passed\n");
    return 1;
}

static int test_symbolic_abs(void)
{
    printf("Testing symbolic abs simplification...\n");

    // abs(0) → 0
    TEST_ASSERT(assert_symbolic("abs(0)", "0", "abs(0) → 0"), "abs(0)");

    // abs(5) → 5 (positive stays positive)
    TEST_ASSERT(assert_symbolic("abs(5)", "5", "abs(5) → 5"), "abs(5)");

    printf("  ✅ Abs simplification tests passed\n");
    return 1;
}

static int test_symbolic_combined(void)
{
    printf("Testing symbolic combined/nested simplification...\n");

    // 0 + 0 → 0
    TEST_ASSERT(assert_symbolic("0+0", "0", "0 + 0 → 0"), "0+0");

    // 1 * 1 → 1
    TEST_ASSERT(assert_symbolic("1*1", "1", "1 * 1 → 1"), "1*1");

    // (2+3)*(4+1) → 25  (integers fold)
    TEST_ASSERT(assert_symbolic("(2+3)*(4+1)", "25", "(2+3)*(4+1) → 25"), "(2+3)*(4+1)");

    // 2^3 + 2 → 10
    TEST_ASSERT(assert_symbolic("2^3+2", "10", "2^3 + 2 → 10"), "2^3+2");

    // sin(0) + cos(0) → 0 + 1 → 1
    TEST_ASSERT(assert_symbolic("sin(0)+cos(0)", "1", "sin(0)+cos(0) → 1"), "sin(0)+cos(0)");

    // exp(0) * pi → 1 * pi → pi
    TEST_ASSERT(assert_symbolic("exp(0)*pi", "π", "exp(0)*pi → pi"), "exp(0)*pi");

    printf("  ✅ Combined simplification tests passed\n");
    return 1;
}

// ---- Main entry point ----

int run_symbolic_tests(void)
{
    printf("Running Symbolic Test Suite\n");
    printf("===========================\n\n");

    int passed = 0;
    int total = 0;

    total++;
    if (test_symbolic_clone_and_equals())
        passed++;
    total++;
    if (test_symbolic_predicates())
        passed++;
    total++;
    if (test_symbolic_addition())
        passed++;
    total++;
    if (test_symbolic_subtraction())
        passed++;
    total++;
    if (test_symbolic_multiplication())
        passed++;
    total++;
    if (test_symbolic_division())
        passed++;
    total++;
    if (test_symbolic_power())
        passed++;
    total++;
    if (test_symbolic_trig())
        passed++;
    total++;
    if (test_symbolic_log_exp())
        passed++;
    total++;
    if (test_symbolic_sqrt())
        passed++;
    total++;
    if (test_symbolic_abs())
        passed++;
    total++;
    if (test_symbolic_combined())
        passed++;

    printf("\n===========================\n");
    printf("Symbolic Tests: %d/%d passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
