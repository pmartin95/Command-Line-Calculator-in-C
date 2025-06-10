
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
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

    case NODE_BINOP:
        printf("BINOP: %s\n", token_type_str(node->binop.op));
        print_ast(node->binop.left, depth + 1);
        print_ast(node->binop.right, depth + 1);
        break;

    case NODE_UNARY:
        printf("UNARY: %s\n", token_type_str(node->unary.op));
        print_ast(node->unary.operand, depth + 1);
        break;
    }
}

char *read_input_line(void)
{
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
}

int main(int argc, char *argv[])
{
    char *input = NULL;

    printf("Command Line Calculator with Implicit Multiplication\n");
    printf("Type 'quit' to exit, 'help' for examples\n\n");

    while (1)
    {
        // Free previous input
        free(input);
        input = NULL;

        // Read input line
        input = read_input_line();
        if (!input)
        {
            break; // EOF or error
        }

        // Skip empty input
        if (strlen(input) == 0)
        {
            continue;
        }

        // Handle commands
        if (strcmp(input, "quit") == 0)
        {
            break;
        }

        if (strcmp(input, "help") == 0)
        {
            printf("Examples:\n");
            printf("  2+3*4         -> 14\n");
            printf("  2(3+4)        -> 14 (implicit multiplication)\n");
            printf("  (3+4)(2+1)    -> 21 (implicit multiplication)\n");
            printf("  2^3^2         -> 512 (right-associative)\n");
            printf("  -5+3          -> -2\n");
            printf("  3.14*2        -> 6.28\n");
            printf("  5>3           -> 1 (true)\n");
            printf("  2==2          -> 1 (true)\n");
            printf("  .5 + 3.       -> 3.5 (decimal numbers)\n");
            continue;
        }

        // Tokenize and parse
        Lexer lexer;
        init_lexer(&lexer, input);

        // Check if lexer initialization failed (e.g., input too long)
        if (lexer.input_length == 0 && strlen(input) > 0)
        {
            printf("Input too long or invalid\n");
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
            continue;
        }

        // Check for unexpected tokens at end
        if (parser.current_token.type == TOKEN_INVALID)
        {
            printf("Invalid token encountered\n");
            free_ast(ast);
            continue;
        }

        if (parser.current_token.type != TOKEN_EOF)
        {
            printf("Unexpected token at end: %s\n",
                   token_type_str(parser.current_token.type));
            free_ast(ast);
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
    }

    // Cleanup
    free(input);
    printf("Goodbye!\n");
    return 0;
}