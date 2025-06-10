#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif
#include "lexer.h"
#include "parser.h"

void print_ast(const ASTNode *node, int depth)
{
    if (!node)
        return;

    for (int i = 0; i < depth; i++)
        printf("  ");

    switch (node->type)
    {
    case NODE_NUMBER:
        // For MPFR numbers, we need to print differently
        if (node->number.is_int && mpfr_fits_slong_p(node->number.value, global_rounding))
        {
            long val = mpfr_get_si(node->number.value, global_rounding);
            printf("NUMBER: %ld\n", val);
        }
        else
        {
            mpfr_printf("NUMBER: %.6Rf\n", node->number.value);
        }
        break;

    case NODE_CONSTANT:
        printf("CONSTANT: %s\n", token_type_str(node->constant.const_type));
        break;

    case NODE_BINOP:
        printf("BINOP: %s\n", token_type_str(node->binop.op));
        print_ast(node->binop.left, depth + 1);
        print_ast(node->binop.right, depth + 1);
        break;

    case NODE_UNARY:
        printf("UNARY: %s\n", token_type_str(node->unary.op));
        print_ast(node->unary.operand, depth + 1);
        break;

    case NODE_FUNCTION:
        printf("FUNCTION: %s (%d args)\n",
               token_type_str(node->function.func_type),
               node->function.arg_count);
        for (int i = 0; i < node->function.arg_count; i++)
        {
            print_ast(node->function.args[i], depth + 1);
        }
        break;
    }
}

char *read_input_line(void)
{
#ifdef HAVE_READLINE
    // Use readline for history and line editing support
    char *line = readline("> ");

    // Add non-empty lines to history
    if (line && *line)
    {
        add_history(line);
    }

    return line;
#else
    // Fallback to basic getline
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    printf("> ");
    fflush(stdout);

    read = getline(&line, &len, stdin);
    if (read == -1)
    {
        free(line);
        return NULL;
    }

    // Remove newline if present
    if (read > 0 && line[read - 1] == '\n')
    {
        line[read - 1] = '\0';
    }

    return line;
#endif
}

void print_help(void)
{
    printf("High-Precision Mathematical Calculator with Function Support\n\n");

#ifdef HAVE_READLINE
    printf("Navigation:\n");
    printf("  Up/Down Arrow -> Browse command history\n");
    printf("  Left/Right    -> Move cursor in current line\n");
    printf("  Ctrl+A        -> Move to beginning of line\n");
    printf("  Ctrl+E        -> Move to end of line\n");
    printf("  Ctrl+L        -> Clear screen\n\n");
#endif

    printf("Precision Commands:\n");
    printf("  precision           -> Show current precision\n");
    printf("  precision <bits>    -> Set precision (53-8192 bits)\n");
    printf("  test                -> Test high precision arithmetic\n\n");

    printf("Basic operations:\n");
    printf("  2+3*4         -> 14\n");
    printf("  2(3+4)        -> 14 (implicit multiplication)\n");
    printf("  (3+4)(2+1)    -> 21\n");
    printf("  2^3^2         -> 512 (right-associative)\n");
    printf("  -5+3          -> -2\n\n");

    printf("High precision examples:\n");
    printf("  1+1e-30       -> Shows tiny differences\n");
    printf("  pi*e^100      -> Very large precise calculations\n");
    printf("  sqrt(2)       -> High precision square root\n\n");

    printf("Scientific notation:\n");
    printf("  1.5e10        -> 15000000000\n");
    printf("  2.3e-5        -> 2.3e-05\n");
    printf("  6.02e+23      -> 6.02e+23 (Avogadro's number)\n");
    printf("  1e3 + 1e2     -> 1100\n");
    printf("  sin(1e-6)     -> 1e-06\n\n");

    printf("Trigonometric functions (radians):\n");
    printf("  sin(pi/2)     -> 1\n");
    printf("  cos(0)        -> 1\n");
    printf("  tan(pi/4)     -> 1\n");
    printf("  asin(1)       -> pi/2\n");
    printf("  acos(0)       -> pi/2\n");
    printf("  atan(1)       -> pi/4\n");
    printf("  atan2(1,1)    -> pi/4\n\n");

    printf("Hyperbolic functions:\n");
    printf("  sinh(1)       -> Hyperbolic sine\n");
    printf("  cosh(0)       -> 1\n");
    printf("  tanh(0)       -> 0\n");
    printf("  asinh(1)      -> Inverse hyperbolic sine\n");
    printf("  acosh(2)      -> Inverse hyperbolic cosine\n");
    printf("  atanh(0.5)    -> Inverse hyperbolic tangent\n\n");

    printf("Other functions:\n");
    printf("  sqrt(16)      -> 4\n");
    printf("  log(e)        -> 1 (natural log)\n");
    printf("  log10(100)    -> 2\n");
    printf("  exp(1)        -> e\n");
    printf("  abs(-5)       -> 5\n");
    printf("  floor(3.7)    -> 3\n");
    printf("  ceil(3.2)     -> 4\n");
    printf("  pow(2,3)      -> 8\n\n");

    printf("Constants:\n");
    printf("  pi            -> High precision π\n");
    printf("  e             -> High precision e\n\n");

    printf("Comparison operators:\n");
    printf("  5>3           -> 1 (true)\n");
    printf("  2==2          -> 1 (true)\n");
    printf("  3!=4          -> 1 (true)\n\n");

    printf("Examples combining functions:\n");
    printf("  sin(pi/6)*2   -> 1\n");
    printf("  sqrt(pow(3,2)+pow(4,2)) -> 5\n");
    printf("  log(exp(2))   -> 2\n");
    printf("  2*pi*sqrt(2)  -> High precision result\n");
    printf("  1.5e10/3e8    -> 50 (scientific notation)\n\n");
}

int main(int argc, char *argv[])
{
    // Initialize MPFR with default precision
    mpfr_set_default_prec(DEFAULT_PRECISION);

    char *input = NULL;

#ifdef HAVE_READLINE
    printf("High-Precision Mathematical Calculator (with readline support)\n");
    printf("Type 'quit' to exit, 'help' for examples, 'precision <bits>' to set precision\n");
    printf("Use Up/Down arrows to browse history\n");
#else
    printf("High-Precision Mathematical Calculator\n");
    printf("Type 'quit' to exit, 'help' for examples, 'precision <bits>' to set precision\n");
    printf("Note: Compile with -DHAVE_READLINE -lreadline for history support\n");
#endif

    print_precision_info();
    printf("\n");

    while (1)
    {
        // Read input line
        input = read_input_line();
        if (!input)
        {
            break; // EOF or error
        }

        // Skip empty input
        if (strlen(input) == 0)
        {
            free(input);
            continue;
        }

        // Handle commands
        if (strcmp(input, "quit") == 0)
        {
            free(input);
            break;
        }

        if (strcmp(input, "help") == 0)
        {
            print_help();
            free(input);
            continue;
        }

        if (strcmp(input, "precision") == 0)
        {
            print_precision_info();
            free(input);
            continue;
        }

        // Handle precision setting
        if (strncmp(input, "precision ", 10) == 0)
        {
            long new_prec = strtol(input + 10, NULL, 10);
            if (new_prec > 0)
            {
                set_precision((mpfr_prec_t)new_prec);
                print_precision_info();
            }
            else
            {
                printf("Invalid precision value\n");
            }
            free(input);
            continue;
        }

        // Test command to verify high precision
        if (strcmp(input, "test") == 0)
        {
            printf("Testing high precision arithmetic:\n");
            
            mpfr_t one, small, result_test;
            mpfr_init2(one, global_precision);
            mpfr_init2(small, global_precision);
            mpfr_init2(result_test, global_precision);
            
            // Test 1: 1 + 1e-30
            mpfr_set_d(one, 1.0, global_rounding);
            mpfr_set_str(small, "1e-30", 10, global_rounding);
            mpfr_add(result_test, one, small, global_rounding);
            
            long decimal_digits = (long)(global_precision * 0.30103);
            if (decimal_digits < 35) decimal_digits = 35;
            
            printf("1 + 1e-30 = ");
            mpfr_printf("%.*Rf\n", (int)decimal_digits, result_test);
            
            // Test 2: High precision pi
            mpfr_const_pi(result_test, global_rounding);
            printf("π = ");
            mpfr_printf("%.*Rf\n", (int)decimal_digits, result_test);
            
            // Test 3: High precision e
            mpfr_set_d(result_test, 1.0, global_rounding);
            mpfr_exp(result_test, result_test, global_rounding);
            printf("e = ");
            mpfr_printf("%.*Rf\n", (int)decimal_digits, result_test);
            
            mpfr_clear(one);
            mpfr_clear(small);
            mpfr_clear(result_test);
            
            free(input);
            continue;
        }

        // Tokenize and parse
        Lexer lexer;
        init_lexer(&lexer, input);

        // Check if lexer initialization failed (e.g., input too long)
        if (lexer.input_length == 0 && strlen(input) > 0)
        {
            printf("Input too long or invalid\n");
            free(input);
            continue;
        }

        Parser parser;
        init_parser(&parser, &lexer);

        ASTNode *ast = parse_expression(&parser);

        // Check for parse errors
        if (!ast || parser.error_occurred)
        {
            printf("Parse error\n");
            free_ast(ast);
            free(input);
            continue;
        }

        // Check for unexpected tokens at end
        if (parser.current_token.type == TOKEN_INVALID)
        {
            printf("Invalid token encountered\n");
            free_ast(ast);
            free(input);
            continue;
        }

        if (parser.current_token.type != TOKEN_EOF)
        {
            printf("Unexpected token at end: %s\n",
                   token_type_str(parser.current_token.type));
            free_ast(ast);
            free(input);
            continue;
        }

// Uncomment to see AST structure for debugging
#ifdef DEBUG_AST
        printf("AST:\n");
        print_ast(ast, 0);
        printf("\n");
#endif

        // Evaluate with high precision
        mpfr_t result;
        mpfr_init2(result, global_precision);
        evaluate_ast(result, ast);

        // Print result
        print_mpfr_smart(result);

        mpfr_clear(result);
        free_ast(ast);
        free(input);
    }

#ifdef HAVE_READLINE
    // Cleanup readline history
    clear_history();
#endif

    // Cleanup MPFR
    mpfr_free_cache();
    printf("Goodbye!\n");
    return 0;
}