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
        if (node->number.is_int)
            printf("NUMBER: %d\n", (int)node->number.value);
        else
            printf("NUMBER: %g\n", node->number.value);
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
    printf("Mathematical Calculator with Function Support\n\n");

#ifdef HAVE_READLINE
    printf("Navigation:\n");
    printf("  Up/Down Arrow -> Browse command history\n");
    printf("  Left/Right    -> Move cursor in current line\n");
    printf("  Ctrl+A        -> Move to beginning of line\n");
    printf("  Ctrl+E        -> Move to end of line\n");
    printf("  Ctrl+L        -> Clear screen\n\n");
#endif

    printf("Basic operations:\n");
    printf("  2+3*4         -> 14\n");
    printf("  2(3+4)        -> 14 (implicit multiplication)\n");
    printf("  (3+4)(2+1)    -> 21\n");
    printf("  2^3^2         -> 512 (right-associative)\n");
    printf("  -5+3          -> -2\n\n");

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
    printf("  asin(1)       -> %g (pi/2)\n", 3.14159265 / 2);
    printf("  acos(0)       -> %g (pi/2)\n", 3.14159265 / 2);
    printf("  atan(1)       -> %g (pi/4)\n", 3.14159265 / 4);
    printf("  atan2(1,1)    -> %g (pi/4)\n\n", 3.14159265 / 4);

    printf("Hyperbolic functions:\n");
    printf("  sinh(1)       -> %g\n", 1.1752);
    printf("  cosh(0)       -> 1\n");
    printf("  tanh(0)       -> 0\n");
    printf("  asinh(1)      -> %g\n", 0.8814);
    printf("  acosh(2)      -> %g\n", 1.3170);
    printf("  atanh(0.5)    -> %g\n\n", 0.5493);

    printf("Other functions:\n");
    printf("  sqrt(16)      -> 4\n");
    printf("  log(e)        -> 1 (natural log)\n");
    printf("  log10(100)    -> 2\n");
    printf("  exp(1)        -> %g (e)\n", 2.7183);
    printf("  abs(-5)       -> 5\n");
    printf("  floor(3.7)    -> 3\n");
    printf("  ceil(3.2)     -> 4\n");
    printf("  pow(2,3)      -> 8\n\n");

    printf("Constants:\n");
    printf("  pi            -> %g\n", 3.14159265);
    printf("  e             -> %g\n\n", 2.71828183);

    printf("Comparison operators:\n");
    printf("  5>3           -> 1 (true)\n");
    printf("  2==2          -> 1 (true)\n");
    printf("  3!=4          -> 1 (true)\n\n");

    printf("Examples combining functions:\n");
    printf("  sin(pi/6)*2   -> 1\n");
    printf("  sqrt(pow(3,2)+pow(4,2)) -> 5\n");
    printf("  log(exp(2))   -> 2\n");
    printf("  2*pi*sqrt(2)  -> (using constants)\n");
    printf("  1.5e10/3e8    -> 50 (scientific notation)\n\n");
}

int main(int argc, char *argv[])
{
    char *input = NULL;

#ifdef HAVE_READLINE
    printf("Advanced Mathematical Calculator (with readline support)\n");
    printf("Type 'quit' to exit, 'help' for examples\n");
    printf("Use Up/Down arrows to browse history\n\n");
#else
    printf("Advanced Mathematical Calculator\n");
    printf("Type 'quit' to exit, 'help' for examples\n");
    printf("Note: Compile with -DHAVE_READLINE -lreadline for history support\n\n");
#endif

    while (1)
    {
        // Read input line (readline automatically handles previous input)
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

        // Evaluate and print result
        double result = evaluate_ast(ast);

        // Print as integer if the result is a whole number and reasonable
        if (result == (int)result && result <= INT_MAX && result >= INT_MIN)
        {
            printf("= %d\n", (int)result);
        }
        else
        {
            printf("= %g\n", result);
        }

        free_ast(ast);
        free(input);
    }

#ifdef HAVE_READLINE
    // Cleanup readline history
    clear_history();
#endif

    printf("Goodbye!\n");
    return 0;
}