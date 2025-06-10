#ifndef __PARSER__
#define __PARSER__

#include "lexer.h"

typedef enum
{
    NODE_NUMBER,
    NODE_BINOP,
    NODE_UNARY,
    NODE_FUNCTION,
    NODE_CONSTANT
} NodeType;

typedef struct ASTNode
{
    NodeType type;
    union
    {
        struct
        {
            double value;
            int is_int; // Track if originally an integer
        } number;
        struct
        {
            TokenType op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binop;
        struct
        {
            TokenType op;
            struct ASTNode *operand;
        } unary;
        struct
        {
            TokenType func_type;
            struct ASTNode **args;
            int arg_count;
        } function;
        struct
        {
            TokenType const_type;
        } constant;
    };
} ASTNode;

typedef struct
{
    Lexer *lexer;
    Token current_token;
    Token previous_token; // Track previous token for implicit multiplication
    int recursion_depth;  // Added for recursion depth tracking
    int max_depth;        // Added for maximum recursion depth
    int error_occurred;   // Added for error state tracking
} Parser;

// Parser functions
void init_parser(Parser *parser, Lexer *lexer);
void parser_advance(Parser *parser);
int should_insert_multiplication(Parser *parser);

// Recursive descent parsing functions
ASTNode *parse_expression(Parser *parser);
ASTNode *parse_comparison(Parser *parser);
ASTNode *parse_term(Parser *parser);
ASTNode *parse_factor(Parser *parser);
ASTNode *parse_power(Parser *parser);
ASTNode *parse_unary(Parser *parser);
ASTNode *parse_primary(Parser *parser);
ASTNode *parse_function_call(Parser *parser, TokenType func_type);

// AST creation helpers
ASTNode *create_number_node(double value, int is_int);
ASTNode *create_binop_node(TokenType op, ASTNode *left, ASTNode *right);
ASTNode *create_unary_node(TokenType op, ASTNode *operand);
ASTNode *create_function_node(TokenType func_type, ASTNode **args, int arg_count);
ASTNode *create_constant_node(TokenType const_type);

// AST cleanup
void free_ast(ASTNode *node);

// Evaluation (made const-correct)
double evaluate_ast(const ASTNode *node);

// Helper functions
int is_function_token(TokenType type);
int is_constant_token(TokenType type);

#endif