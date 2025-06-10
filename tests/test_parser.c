#include "parser.h"
#include "lexer.h"
#include "ast.h"
#include "precision.h"
#include "function_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("  ❌ FAIL: %s\n", message); \
            return 0; \
        } \
    } while(0)

// Helper function to parse an expression
ASTNode *parse_test_expression(const char *input)
{
    Lexer lexer;
    lexer_init(&lexer, input);
    
    Parser parser;
    parser_init(&parser, &lexer);
    
    ASTNode *ast = parser_parse_expression(&parser);
    
    if (!ast || parser_has_error(&parser) || parser.current_token.type != TOKEN_EOF) {
        ast_free(ast);
        return NULL;
    }
    
    return ast;
}

int test_parser_basic_arithmetic(void)
{
    printf("Testing basic arithmetic parsing...\n");
    
    struct {
        const char *expr;
        NodeType expected_type;
    } tests[] = {
        {"2+3", NODE_BINOP},
        {"5-2", NODE_BINOP},
        {"3*4", NODE_BINOP},
        {"8/2", NODE_BINOP},
        {"2^3", NODE_BINOP},
        {"123", NODE_NUMBER},
        {"pi", NODE_CONSTANT},
        {"-5", NODE_UNARY},
        {"+7", NODE_UNARY}
    };
    
    for (int i = 0; i < 9; i++) {
        ASTNode *ast = parse_test_expression(tests[i].expr);
        TEST_ASSERT(ast != NULL, tests[i].expr);
        TEST_ASSERT(ast->type == tests[i].expected_type, "Expected node type");
        ast_free(ast);
    }
    
    printf("  ✅ Basic arithmetic parsing tests passed\n");
    return 1;
}

int test_parser_precedence(void)
{
    printf("Testing operator precedence...\n");
    
    // Test: 2+3*4 should parse as 2+(3*4), not (2+3)*4
    ASTNode *ast = parse_test_expression("2+3*4");
    TEST_ASSERT(ast != NULL, "Should parse 2+3*4");
    TEST_ASSERT(ast->type == NODE_BINOP, "Root should be binary op");
    TEST_ASSERT(ast->binop.op == TOKEN_PLUS, "Root should be +");
    TEST_ASSERT(ast->binop.right->type == NODE_BINOP, "Right should be binary op");
    TEST_ASSERT(ast->binop.right->binop.op == TOKEN_STAR, "Right should be *");
    ast_free(ast);
    
    // Test: 2^3^2 should parse as 2^(3^2), not (2^3)^2 (right associative)
    ast = parse_test_expression("2^3^2");
    TEST_ASSERT(ast != NULL, "Should parse 2^3^2");
    TEST_ASSERT(ast->type == NODE_BINOP, "Root should be binary op");
    TEST_ASSERT(ast->binop.op == TOKEN_CARET, "Root should be ^");
    TEST_ASSERT(ast->binop.right->type == NODE_BINOP, "Right should be binary op");
    TEST_ASSERT(ast->binop.right->binop.op == TOKEN_CARET, "Right should be ^");
    ast_free(ast);
    
    printf("  ✅ Operator precedence tests passed\n");
    return 1;
}

int test_parser_parentheses(void)
{
    printf("Testing parentheses handling...\n");
    
    // Test: (2+3)*4 should parse as (2+3)*4
    ASTNode *ast = parse_test_expression("(2+3)*4");
    TEST_ASSERT(ast != NULL, "Should parse (2+3)*4");
    TEST_ASSERT(ast->type == NODE_BINOP, "Root should be binary op");
    TEST_ASSERT(ast->binop.op == TOKEN_STAR, "Root should be *");
    TEST_ASSERT(ast->binop.left->type == NODE_BINOP, "Left should be binary op");
    TEST_ASSERT(ast->binop.left->binop.op == TOKEN_PLUS, "Left should be +");
    ast_free(ast);
    
    // Test nested parentheses: ((2+3))
    ast = parse_test_expression("((2+3))");
    TEST_ASSERT(ast != NULL, "Should parse ((2+3))");
    TEST_ASSERT(ast->type == NODE_BINOP, "Should unwrap to binary op");
    TEST_ASSERT(ast->binop.op == TOKEN_PLUS, "Should be +");
    ast_free(ast);
    
    printf("  ✅ Parentheses handling tests passed\n");
    return 1;
}

int test_parser_functions(void)
{
    printf("Testing function parsing...\n");
    
    // Test single argument function
    ASTNode *ast = parse_test_expression("sin(0)");
    TEST_ASSERT(ast != NULL, "Should parse sin(0)");
    TEST_ASSERT(ast->type == NODE_FUNCTION, "Should be function node");
    TEST_ASSERT(ast->function.func_type == TOKEN_SIN, "Should be sin function");
    TEST_ASSERT(ast->function.arg_count == 1, "Should have 1 argument");
    ast_free(ast);
    
    // Test two argument function
    ast = parse_test_expression("atan2(1,1)");
    TEST_ASSERT(ast != NULL, "Should parse atan2(1,1)");
    TEST_ASSERT(ast->type == NODE_FUNCTION, "Should be function node");
    TEST_ASSERT(ast->function.func_type == TOKEN_ATAN2, "Should be atan2 function");
    TEST_ASSERT(ast->function.arg_count == 2, "Should have 2 arguments");
    ast_free(ast);
    
    // Test nested function
    ast = parse_test_expression("sin(cos(0))");
    TEST_ASSERT(ast != NULL, "Should parse sin(cos(0))");
    TEST_ASSERT(ast->type == NODE_FUNCTION, "Should be function node");
    TEST_ASSERT(ast->function.args[0]->type == NODE_FUNCTION, "Arg should be function");
    ast_free(ast);
    
    printf("  ✅ Function parsing tests passed\n");
    return 1;
}

int test_parser_implicit_multiplication(void)
{
    printf("Testing implicit multiplication...\n");
    
    struct {
        const char *expr;
        const char *desc;
    } tests[] = {
        {"2(3+4)", "number followed by parentheses"},
        {"(2+1)(3+1)", "parentheses followed by parentheses"},
        {"2pi", "number followed by constant"},
        {"3e", "number followed by constant"},
        {"2sin(0)", "number followed by function"},
        {"pi(2)", "constant followed by parentheses"}
    };
    
    for (int i = 0; i < 6; i++) {
        ASTNode *ast = parse_test_expression(tests[i].expr);
        TEST_ASSERT(ast != NULL, tests[i].desc);
        TEST_ASSERT(ast->type == NODE_BINOP, "Should create binary op");
        TEST_ASSERT(ast->binop.op == TOKEN_STAR, "Should be multiplication");
        ast_free(ast);
    }
    
    printf("  ✅ Implicit multiplication tests passed\n");
    return 1;
}

int test_parser_unary_operators(void)
{
    printf("Testing unary operators...\n");
    
    // Test simple unary minus
    ASTNode *ast = parse_test_expression("-5");
    TEST_ASSERT(ast != NULL, "Should parse -5");
    TEST_ASSERT(ast->type == NODE_UNARY, "Should be unary node");
    TEST_ASSERT(ast->unary.op == TOKEN_MINUS, "Should be minus");
    ast_free(ast);
    
    // Test double negative
    ast = parse_test_expression("--3");
    TEST_ASSERT(ast != NULL, "Should parse --3");
    TEST_ASSERT(ast->type == NODE_UNARY, "Should be unary node");
    TEST_ASSERT(ast->unary.operand->type == NODE_UNARY, "Should be nested unary");
    ast_free(ast);
    
    // Test unary plus
    ast = parse_test_expression("+7");
    TEST_ASSERT(ast != NULL, "Should parse +7");
    TEST_ASSERT(ast->type == NODE_UNARY, "Should be unary node");
    TEST_ASSERT(ast->unary.op == TOKEN_PLUS, "Should be plus");
    ast_free(ast);
    
    printf("  ✅ Unary operator tests passed\n");
    return 1;
}

int test_parser_comparison_operators(void)
{
    printf("Testing comparison operators...\n");
    
    struct {
        const char *expr;
        TokenType expected_op;
    } tests[] = {
        {"5 > 3", TOKEN_GT},
        {"2 < 1", TOKEN_LT},
        {"4 == 4", TOKEN_EQ},
        {"3 != 3", TOKEN_NEQ},
        {"5 >= 5", TOKEN_GTE},
        {"3 <= 2", TOKEN_LTE}
    };
    
    for (int i = 0; i < 6; i++) {
        ASTNode *ast = parse_test_expression(tests[i].expr);
        TEST_ASSERT(ast != NULL, tests[i].expr);
        TEST_ASSERT(ast->type == NODE_BINOP, "Should be binary op");
        TEST_ASSERT(ast->binop.op == tests[i].expected_op, "Should have correct operator");
        ast_free(ast);
    }
    
    printf("  ✅ Comparison operator tests passed\n");
    return 1;
}

int test_parser_error_cases(void)
{
    printf("Testing parser error cases...\n");
    
    const char *invalid_exprs[] = {
        "(2+3",         // Unmatched parenthesis
        "2+3)",         // Unmatched parenthesis
        "sin()",        // Missing argument
        "atan2(1)",     // Wrong number of arguments
        "",             // Empty expression
        "2+",           // Incomplete expression
        "*5",           // Starting with operator
        "2*/3",         // Invalid operator sequence
    };
    
    for (int i = 0; i < 8; i++) {
        ASTNode *ast = parse_test_expression(invalid_exprs[i]);
        TEST_ASSERT(ast == NULL, "Invalid expression should fail to parse");
    }
    
    printf("  ✅ Error handling tests passed\n");
    return 1;
}

int test_parser_complex_expressions(void)
{
    printf("Testing complex expressions...\n");
    
    const char *complex_exprs[] = {
        "sqrt(pow(3,2) + pow(4,2))",    // Pythagorean theorem
        "2*pi*sqrt(2)",                 // Using constants
        "log(exp(2))",                  // Nested functions
        "sin(pi/6) * 2",                // Trigonometry
        "(1+2)*(3+4)/(5+6)",           // Complex arithmetic
        "2^3^2 + 1",                    // Mixed operators
        "sin(cos(tan(0)))"              // Triple nesting
    };
    
    for (int i = 0; i < 7; i++) {
        ASTNode *ast = parse_test_expression(complex_exprs[i]);
        TEST_ASSERT(ast != NULL, complex_exprs[i]);
        ast_free(ast);
    }
    
    printf("  ✅ Complex expression tests passed\n");
    return 1;
}

int run_parser_tests(void)
{
    printf("Running Parser Test Suite\n");
    printf("=========================\n\n");
    
    // Initialize required systems
    precision_init();
    function_table_init();
    
    int passed = 0;
    int total = 0;
    
    total++; if (test_parser_basic_arithmetic()) passed++;
    total++; if (test_parser_precedence()) passed++;
    total++; if (test_parser_parentheses()) passed++;
    total++; if (test_parser_functions()) passed++;
    total++; if (test_parser_implicit_multiplication()) passed++;
    total++; if (test_parser_unary_operators()) passed++;
    total++; if (test_parser_comparison_operators()) passed++;
    total++; if (test_parser_error_cases()) passed++;
    total++; if (test_parser_complex_expressions()) passed++;
    
    printf("\n=========================\n");
    printf("Parser Tests: %d/%d passed\n", passed, total);
    
    precision_cleanup();
    
    return (passed == total) ? 0 : 1;
}