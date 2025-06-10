#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

void print_ast(ASTNode *node, int depth)
{
    if (!node) return;
    
    for (int i = 0; i < depth; i++) printf("  ");
    
    switch (node->type) {
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

int main(int argc, char *argv[])
{
    char input[256];
    
    printf("Command Line Calculator with Implicit Multiplication\n");
    printf("Type 'quit' to exit, 'help' for examples\n\n");
    
    while (1) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) break;
        
        // Remove newline
        input[strcspn(input, "\n")] = '\0';
        
        if (strcmp(input, "quit") == 0) break;
        
        if (strcmp(input, "help") == 0) {
            printf("Examples:\n");
            printf("  2+3*4         -> 14\n");
            printf("  2(3+4)        -> 14 (implicit multiplication)\n");
            printf("  (3+4)(2+1)    -> 21 (implicit multiplication)\n");
            printf("  2^3^2         -> 512 (right-associative)\n");
            printf("  -5+3          -> -2\n");
            printf("  3.14*2        -> 6.28\n");
            printf("  5>3           -> 1 (true)\n");
            printf("  2==2          -> 1 (true)\n");
            continue;
        }
        
        // Tokenize and parse
        Lexer lexer;
        init_lexer(&lexer, input);
        
        Parser parser;
        init_parser(&parser, &lexer);
        
        ASTNode *ast = parse_expression(&parser);
        
        if (!ast) {
            printf("Parse error\n");
            continue;
        }
        
        // Check for unexpected tokens
        if (parser.current_token.type != TOKEN_EOF) {
            printf("Unexpected token at end: %s\n", 
                   token_type_str(parser.current_token.type));
            free_ast(ast);
            continue;
        }
        
        // Uncomment to see AST structure
        // printf("AST:\n");
        // print_ast(ast, 0);
        
        // Evaluate and print result
        double result = evaluate_ast(ast);
        
        // Print as integer if the result is a whole number
        if (result == (int)result) {
            printf("= %d\n", (int)result);
        } else {
            printf("= %g\n", result);
        }
        
        free_ast(ast);
    }
    
    printf("Goodbye!\n");
    return 0;
}