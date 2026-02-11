#include "lexer.h"
#include "tokens.h"
#include "function_table.h"
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

// Helper: get token from input, freeing any previous token's strings
static Token lex_single(const char *input)
{
    Lexer lexer;
    lexer_init(&lexer, input);
    return lexer_get_next_token(&lexer);
}

// Helper: collect all tokens into a dynamically allocated array
// Caller must free the array (and any token strings) when done
static Token *lex_all(const char *input, int *count)
{
    Lexer lexer;
    lexer_init(&lexer, input);

    int capacity = 16;
    Token *tokens = malloc(sizeof(Token) * capacity);
    *count = 0;

    while (1)
    {
        if (*count >= capacity)
        {
            capacity *= 2;
            tokens = realloc(tokens, sizeof(Token) * capacity);
        }
        tokens[*count] = lexer_get_next_token(&lexer);
        if (tokens[*count].type == TOKEN_EOF)
        {
            (*count)++; // Include EOF
            break;
        }
        (*count)++;
    }
    return tokens;
}

static void free_tokens(Token *tokens, int count)
{
    for (int i = 0; i < count; i++)
        token_free(&tokens[i]);
    free(tokens);
}

// ---- Test suites ----

static int test_lexer_integers(void)
{
    printf("Testing integer tokenization...\n");

    Token t = lex_single("42");
    TEST_ASSERT(t.type == TOKEN_INT, "42 is TOKEN_INT");
    TEST_ASSERT(t.int_value == 42, "42 has value 42");
    TEST_ASSERT(t.number_string != NULL, "42 has number_string");
    TEST_ASSERT(strcmp(t.number_string, "42") == 0, "42 number_string is '42'");
    token_free(&t);

    t = lex_single("0");
    TEST_ASSERT(t.type == TOKEN_INT, "0 is TOKEN_INT");
    TEST_ASSERT(t.int_value == 0, "0 has value 0");
    token_free(&t);

    t = lex_single("1234567");
    TEST_ASSERT(t.type == TOKEN_INT, "1234567 is TOKEN_INT");
    TEST_ASSERT(t.int_value == 1234567, "1234567 has correct value");
    token_free(&t);

    printf("  ✅ Integer tokenization tests passed\n");
    return 1;
}

static int test_lexer_floats(void)
{
    printf("Testing float tokenization...\n");

    Token t = lex_single("3.14");
    TEST_ASSERT(t.type == TOKEN_FLOAT, "3.14 is TOKEN_FLOAT");
    TEST_ASSERT(t.number_string != NULL, "3.14 has number_string");
    TEST_ASSERT(strcmp(t.number_string, "3.14") == 0, "3.14 number_string is '3.14'");
    token_free(&t);

    t = lex_single(".5");
    TEST_ASSERT(t.type == TOKEN_FLOAT, ".5 is TOKEN_FLOAT");
    TEST_ASSERT(t.number_string != NULL && strcmp(t.number_string, ".5") == 0, ".5 number_string is '.5'");
    token_free(&t);

    t = lex_single("0.0");
    TEST_ASSERT(t.type == TOKEN_FLOAT, "0.0 is TOKEN_FLOAT");
    token_free(&t);

    printf("  ✅ Float tokenization tests passed\n");
    return 1;
}

static int test_lexer_scientific_notation(void)
{
    printf("Testing scientific notation tokenization...\n");

    Token t = lex_single("1e5");
    TEST_ASSERT(t.type == TOKEN_FLOAT, "1e5 is TOKEN_FLOAT");
    TEST_ASSERT(t.number_string != NULL && strcmp(t.number_string, "1e5") == 0, "1e5 number_string");
    token_free(&t);

    t = lex_single("2.5e10");
    TEST_ASSERT(t.type == TOKEN_FLOAT, "2.5e10 is TOKEN_FLOAT");
    token_free(&t);

    t = lex_single("1E3");
    TEST_ASSERT(t.type == TOKEN_FLOAT, "1E3 is TOKEN_FLOAT (capital E)");
    token_free(&t);

    t = lex_single("3e+2");
    TEST_ASSERT(t.type == TOKEN_FLOAT, "3e+2 is TOKEN_FLOAT");
    token_free(&t);

    t = lex_single("5e-3");
    TEST_ASSERT(t.type == TOKEN_FLOAT, "5e-3 is TOKEN_FLOAT");
    token_free(&t);

    printf("  ✅ Scientific notation tests passed\n");
    return 1;
}

static int test_lexer_operators(void)
{
    printf("Testing operator tokenization...\n");

    TEST_ASSERT(lex_single("+").type == TOKEN_PLUS, "+ is TOKEN_PLUS");
    TEST_ASSERT(lex_single("-").type == TOKEN_MINUS, "- is TOKEN_MINUS");
    TEST_ASSERT(lex_single("*").type == TOKEN_STAR, "* is TOKEN_STAR");
    TEST_ASSERT(lex_single("/").type == TOKEN_SLASH, "/ is TOKEN_SLASH");
    TEST_ASSERT(lex_single("^").type == TOKEN_CARET, "^ is TOKEN_CARET");
    TEST_ASSERT(lex_single("(").type == TOKEN_LPAREN, "( is TOKEN_LPAREN");
    TEST_ASSERT(lex_single(")").type == TOKEN_RPAREN, ") is TOKEN_RPAREN");
    TEST_ASSERT(lex_single(",").type == TOKEN_COMMA, ", is TOKEN_COMMA");

    printf("  ✅ Operator tokenization tests passed\n");
    return 1;
}

static int test_lexer_comparison_operators(void)
{
    printf("Testing comparison operator tokenization...\n");

    TEST_ASSERT(lex_single("==").type == TOKEN_EQ, "== is TOKEN_EQ");
    TEST_ASSERT(lex_single("!=").type == TOKEN_NEQ, "!= is TOKEN_NEQ");
    TEST_ASSERT(lex_single("<").type == TOKEN_LT, "< is TOKEN_LT");
    TEST_ASSERT(lex_single("<=").type == TOKEN_LTE, "<= is TOKEN_LTE");
    TEST_ASSERT(lex_single(">").type == TOKEN_GT, "> is TOKEN_GT");
    TEST_ASSERT(lex_single(">=").type == TOKEN_GTE, ">= is TOKEN_GTE");

    // Single = is invalid
    TEST_ASSERT(lex_single("=").type == TOKEN_INVALID, "= alone is TOKEN_INVALID");

    // Single ! is invalid
    TEST_ASSERT(lex_single("!").type == TOKEN_INVALID, "! alone is TOKEN_INVALID");

    printf("  ✅ Comparison operator tests passed\n");
    return 1;
}

static int test_lexer_functions(void)
{
    printf("Testing function tokenization...\n");

    function_table_init();

    Token t;

    t = lex_single("sin");
    TEST_ASSERT(t.type == TOKEN_SIN, "sin is TOKEN_SIN");

    t = lex_single("cos");
    TEST_ASSERT(t.type == TOKEN_COS, "cos is TOKEN_COS");

    t = lex_single("tan");
    TEST_ASSERT(t.type == TOKEN_TAN, "tan is TOKEN_TAN");

    t = lex_single("sqrt");
    TEST_ASSERT(t.type == TOKEN_SQRT, "sqrt is TOKEN_SQRT");

    t = lex_single("log");
    TEST_ASSERT(t.type == TOKEN_LOG, "log is TOKEN_LOG");

    t = lex_single("log10");
    TEST_ASSERT(t.type == TOKEN_LOG10, "log10 is TOKEN_LOG10");

    t = lex_single("exp");
    TEST_ASSERT(t.type == TOKEN_EXP, "exp is TOKEN_EXP");

    t = lex_single("abs");
    TEST_ASSERT(t.type == TOKEN_ABS, "abs is TOKEN_ABS");

    t = lex_single("floor");
    TEST_ASSERT(t.type == TOKEN_FLOOR, "floor is TOKEN_FLOOR");

    t = lex_single("ceil");
    TEST_ASSERT(t.type == TOKEN_CEIL, "ceil is TOKEN_CEIL");

    t = lex_single("pow");
    TEST_ASSERT(t.type == TOKEN_POW, "pow is TOKEN_POW");

    t = lex_single("atan2");
    TEST_ASSERT(t.type == TOKEN_ATAN2, "atan2 is TOKEN_ATAN2");

    printf("  ✅ Function tokenization tests passed\n");
    return 1;
}

static int test_lexer_constants(void)
{
    printf("Testing constant tokenization...\n");

    function_table_init();

    Token t;

    t = lex_single("pi");
    TEST_ASSERT(t.type == TOKEN_CONSTANT, "pi is TOKEN_CONSTANT");
    TEST_ASSERT(t.string_value != NULL && strcmp(t.string_value, "pi") == 0, "pi string_value is 'pi'");
    token_free(&t);

    t = lex_single("e");
    TEST_ASSERT(t.type == TOKEN_CONSTANT, "e is TOKEN_CONSTANT");
    TEST_ASSERT(t.string_value != NULL && strcmp(t.string_value, "e") == 0, "e string_value is 'e'");
    token_free(&t);

    printf("  ✅ Constant tokenization tests passed\n");
    return 1;
}

static int test_lexer_unknown_identifiers(void)
{
    printf("Testing unknown identifier tokenization...\n");

    function_table_init();

    Token t = lex_single("foobar");
    TEST_ASSERT(t.type == TOKEN_IDENTIFIER, "foobar is TOKEN_IDENTIFIER");
    TEST_ASSERT(t.string_value != NULL && strcmp(t.string_value, "foobar") == 0, "foobar string_value");
    token_free(&t);

    t = lex_single("xyz_123");
    TEST_ASSERT(t.type == TOKEN_IDENTIFIER, "xyz_123 is TOKEN_IDENTIFIER");
    token_free(&t);

    printf("  ✅ Unknown identifier tests passed\n");
    return 1;
}

static int test_lexer_whitespace(void)
{
    printf("Testing whitespace handling...\n");

    int count;
    Token *tokens;

    // Whitespace between tokens
    tokens = lex_all("  2  +  3  ", &count);
    TEST_ASSERT(count == 4, "' 2 + 3 ' produces 4 tokens (including EOF)");
    TEST_ASSERT(tokens[0].type == TOKEN_INT, "first token is INT");
    TEST_ASSERT(tokens[1].type == TOKEN_PLUS, "second token is PLUS");
    TEST_ASSERT(tokens[2].type == TOKEN_INT, "third token is INT");
    TEST_ASSERT(tokens[3].type == TOKEN_EOF, "fourth token is EOF");
    free_tokens(tokens, count);

    // Tabs and newlines
    tokens = lex_all("\t5\n", &count);
    TEST_ASSERT(count == 2, "tab+5+newline produces 2 tokens");
    TEST_ASSERT(tokens[0].type == TOKEN_INT, "token is INT");
    free_tokens(tokens, count);

    printf("  ✅ Whitespace handling tests passed\n");
    return 1;
}

static int test_lexer_full_expressions(void)
{
    printf("Testing full expression tokenization...\n");

    function_table_init();

    int count;
    Token *tokens;

    // Simple arithmetic: 2+3*4
    tokens = lex_all("2+3*4", &count);
    TEST_ASSERT(count == 6, "2+3*4 produces 6 tokens");
    TEST_ASSERT(tokens[0].type == TOKEN_INT && tokens[0].int_value == 2, "first is 2");
    TEST_ASSERT(tokens[1].type == TOKEN_PLUS, "second is +");
    TEST_ASSERT(tokens[2].type == TOKEN_INT && tokens[2].int_value == 3, "third is 3");
    TEST_ASSERT(tokens[3].type == TOKEN_STAR, "fourth is *");
    TEST_ASSERT(tokens[4].type == TOKEN_INT && tokens[4].int_value == 4, "fifth is 4");
    TEST_ASSERT(tokens[5].type == TOKEN_EOF, "sixth is EOF");
    free_tokens(tokens, count);

    // Function call: sin(pi/2)
    tokens = lex_all("sin(pi/2)", &count);
    TEST_ASSERT(count == 7, "sin(pi/2) produces 7 tokens");
    TEST_ASSERT(tokens[0].type == TOKEN_SIN, "first is SIN");
    TEST_ASSERT(tokens[1].type == TOKEN_LPAREN, "second is LPAREN");
    TEST_ASSERT(tokens[2].type == TOKEN_CONSTANT, "third is CONSTANT (pi)");
    TEST_ASSERT(tokens[3].type == TOKEN_SLASH, "fourth is SLASH");
    TEST_ASSERT(tokens[4].type == TOKEN_INT && tokens[4].int_value == 2, "fifth is 2");
    TEST_ASSERT(tokens[5].type == TOKEN_RPAREN, "sixth is RPAREN");
    TEST_ASSERT(tokens[6].type == TOKEN_EOF, "seventh is EOF");
    free_tokens(tokens, count);

    // Two-arg function: atan2(1,2)
    tokens = lex_all("atan2(1,2)", &count);
    TEST_ASSERT(count == 7, "atan2(1,2) produces 7 tokens");
    TEST_ASSERT(tokens[0].type == TOKEN_ATAN2, "first is ATAN2");
    TEST_ASSERT(tokens[3].type == TOKEN_COMMA, "comma present");
    free_tokens(tokens, count);

    // Comparison: 5>=3
    tokens = lex_all("5>=3", &count);
    TEST_ASSERT(count == 4, "5>=3 produces 4 tokens");
    TEST_ASSERT(tokens[1].type == TOKEN_GTE, "operator is GTE");
    free_tokens(tokens, count);

    printf("  ✅ Full expression tokenization tests passed\n");
    return 1;
}

static int test_lexer_edge_cases(void)
{
    printf("Testing edge cases...\n");

    // Empty input
    Token t = lex_single("");
    TEST_ASSERT(t.type == TOKEN_EOF, "empty string produces EOF");

    // Single dot is invalid
    t = lex_single(".");
    TEST_ASSERT(t.type == TOKEN_INVALID, "standalone dot is INVALID");

    // Unknown character
    t = lex_single("@");
    TEST_ASSERT(t.type == TOKEN_INVALID, "@ is INVALID");

    t = lex_single("#");
    TEST_ASSERT(t.type == TOKEN_INVALID, "# is INVALID");

    // NULL input
    Lexer lexer;
    lexer_init(&lexer, NULL);
    t = lexer_get_next_token(&lexer);
    TEST_ASSERT(t.type == TOKEN_EOF, "NULL input produces EOF");

    // Lexer utility functions
    lexer_init(&lexer, "abc");
    TEST_ASSERT(!lexer_at_end(&lexer), "not at end initially");
    TEST_ASSERT(lexer_get_position(&lexer) == 0, "position starts at 0");
    TEST_ASSERT(lexer_remaining_length(&lexer) == 3, "remaining length is 3");

    printf("  ✅ Edge case tests passed\n");
    return 1;
}

// ---- Main entry point ----

int run_lexer_tests(void)
{
    printf("Running Lexer Test Suite\n");
    printf("========================\n\n");

    function_table_init();

    int passed = 0;
    int total = 0;

    total++;
    if (test_lexer_integers())
        passed++;
    total++;
    if (test_lexer_floats())
        passed++;
    total++;
    if (test_lexer_scientific_notation())
        passed++;
    total++;
    if (test_lexer_operators())
        passed++;
    total++;
    if (test_lexer_comparison_operators())
        passed++;
    total++;
    if (test_lexer_functions())
        passed++;
    total++;
    if (test_lexer_constants())
        passed++;
    total++;
    if (test_lexer_unknown_identifiers())
        passed++;
    total++;
    if (test_lexer_whitespace())
        passed++;
    total++;
    if (test_lexer_full_expressions())
        passed++;
    total++;
    if (test_lexer_edge_cases())
        passed++;

    printf("\n========================\n");
    printf("Lexer Tests: %d/%d passed\n", passed, total);

    return (passed == total) ? 0 : 1;
}
