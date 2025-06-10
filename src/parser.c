#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void init_parser(Parser *parser, Lexer *lexer)
{
    parser->lexer = lexer;
    parser->previous_token = (Token){.type = TOKEN_INVALID};
    parser->current_token = get_next_token(lexer);
}

void parser_advance(Parser *parser)
{
    parser->previous_token = parser->current_token;
    parser->current_token = get_next_token(parser->lexer);
}

int should_insert_multiplication(Parser *parser)
{
    TokenType prev = parser->previous_token.type;
    TokenType curr = parser->current_token.type;
    
    // Check for implicit multiplication patterns
    return (
        // Number followed by '('
        ((prev == TOKEN_INT || prev == TOKEN_FLOAT) && curr == TOKEN_LPAREN) ||
        // ')' followed by '('
        (prev == TOKEN_RPAREN && curr == TOKEN_LPAREN) ||
        // ')' followed by number
        (prev == TOKEN_RPAREN && (curr == TOKEN_INT || curr == TOKEN_FLOAT)) ||
        // Number followed by number (rare but handle it)
        ((prev == TOKEN_INT || prev == TOKEN_FLOAT) && 
         (curr == TOKEN_INT || curr == TOKEN_FLOAT))
    );
}

ASTNode *create_number_node(double value, int is_int)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    node->type = NODE_NUMBER;
    node->number.value = value;
    node->number.is_int = is_int;
    return node;
}

ASTNode *create_binop_node(TokenType op, ASTNode *left, ASTNode *right)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    node->type = NODE_BINOP;
    node->binop.op = op;
    node->binop.left = left;
    node->binop.right = right;
    return node;
}

ASTNode *create_unary_node(TokenType op, ASTNode *operand)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    node->type = NODE_UNARY;
    node->unary.op = op;
    node->unary.operand = operand;
    return node;
}

void free_ast(ASTNode *node)
{
    if (!node) return;
    
    switch (node->type) {
    case NODE_NUMBER:
        break;
    case NODE_BINOP:
        free_ast(node->binop.left);
        free_ast(node->binop.right);
        break;
    case NODE_UNARY:
        free_ast(node->unary.operand);
        break;
    }
    free(node);
}

// Parser implementation with operator precedence:
// Lowest to highest:
// 1. Comparison operators (==, !=, <, <=, >, >=)
// 2. Addition and subtraction (+, -)
// 3. Multiplication and division (*, /)
// 4. Exponentiation (^)
// 5. Unary operators (+, -)
// 6. Parentheses and primary expressions

ASTNode *parse_expression(Parser *parser)
{
    return parse_comparison(parser);
}

ASTNode *parse_comparison(Parser *parser)
{
    ASTNode *left = parse_term(parser);
    
    while (parser->current_token.type == TOKEN_EQ ||
           parser->current_token.type == TOKEN_NEQ ||
           parser->current_token.type == TOKEN_LT ||
           parser->current_token.type == TOKEN_LTE ||
           parser->current_token.type == TOKEN_GT ||
           parser->current_token.type == TOKEN_GTE)
    {
        TokenType op = parser->current_token.type;
        parser_advance(parser);
        ASTNode *right = parse_term(parser);
        left = create_binop_node(op, left, right);
    }
    
    return left;
}

ASTNode *parse_term(Parser *parser)
{
    ASTNode *left = parse_factor(parser);
    
    while (parser->current_token.type == TOKEN_PLUS ||
           parser->current_token.type == TOKEN_MINUS)
    {
        TokenType op = parser->current_token.type;
        parser_advance(parser);
        ASTNode *right = parse_factor(parser);
        left = create_binop_node(op, left, right);
    }
    
    return left;
}

ASTNode *parse_factor(Parser *parser)
{
    ASTNode *left = parse_power(parser);
    
    while (parser->current_token.type == TOKEN_STAR ||
           parser->current_token.type == TOKEN_SLASH ||
           should_insert_multiplication(parser))
    {
        TokenType op = parser->current_token.type;
        
        // Handle implicit multiplication
        if (should_insert_multiplication(parser)) {
            op = TOKEN_STAR;
            // Don't advance - we're inserting a virtual token
        } else {
            parser_advance(parser);
        }
        
        ASTNode *right = parse_power(parser);
        left = create_binop_node(op, left, right);
    }
    
    return left;
}

ASTNode *parse_power(Parser *parser)
{
    ASTNode *left = parse_unary(parser);
    
    // Right-associative
    if (parser->current_token.type == TOKEN_CARET) {
        parser_advance(parser);
        ASTNode *right = parse_power(parser);  // Recursive for right-associativity
        left = create_binop_node(TOKEN_CARET, left, right);
    }
    
    return left;
}

ASTNode *parse_unary(Parser *parser)
{
    if (parser->current_token.type == TOKEN_PLUS ||
        parser->current_token.type == TOKEN_MINUS)
    {
        TokenType op = parser->current_token.type;
        parser_advance(parser);
        ASTNode *operand = parse_unary(parser);  // Allow chaining: --5
        return create_unary_node(op, operand);
    }
    
    return parse_primary(parser);
}

ASTNode *parse_primary(Parser *parser)
{
    Token token = parser->current_token;
    
    switch (token.type) {
    case TOKEN_INT:
        parser_advance(parser);
        return create_number_node(token.int_value, 1);
        
    case TOKEN_FLOAT:
        parser_advance(parser);
        return create_number_node(token.float_value, 0);
        
    case TOKEN_LPAREN:
        parser_advance(parser);
        ASTNode *expr = parse_expression(parser);
        if (parser->current_token.type != TOKEN_RPAREN) {
            fprintf(stderr, "Expected ')'\n");
            free_ast(expr);
            return NULL;
        }
        parser_advance(parser);
        return expr;
        
    default:
        fprintf(stderr, "Unexpected token: %s\n", token_type_str(token.type));
        return NULL;
    }
}

double evaluate_ast(ASTNode *node)
{
    if (!node) return 0.0;
    
    switch (node->type) {
    case NODE_NUMBER:
        return node->number.value;
        
    case NODE_BINOP: {
        double left = evaluate_ast(node->binop.left);
        double right = evaluate_ast(node->binop.right);
        
        switch (node->binop.op) {
        case TOKEN_PLUS:  return left + right;
        case TOKEN_MINUS: return left - right;
        case TOKEN_STAR:  return left * right;
        case TOKEN_SLASH:
            if (right == 0.0) {
                fprintf(stderr, "Division by zero\n");
                return 0.0;
            }
            return left / right;
        case TOKEN_CARET: return pow(left, right);
        case TOKEN_EQ:    return (left == right) ? 1.0 : 0.0;
        case TOKEN_NEQ:   return (left != right) ? 1.0 : 0.0;
        case TOKEN_LT:    return (left < right) ? 1.0 : 0.0;
        case TOKEN_LTE:   return (left <= right) ? 1.0 : 0.0;
        case TOKEN_GT:    return (left > right) ? 1.0 : 0.0;
        case TOKEN_GTE:   return (left >= right) ? 1.0 : 0.0;
        default:
            fprintf(stderr, "Unknown binary operator\n");
            return 0.0;
        }
    }
    
    case NODE_UNARY: {
        double operand = evaluate_ast(node->unary.operand);
        switch (node->unary.op) {
        case TOKEN_PLUS:  return operand;
        case TOKEN_MINUS: return -operand;
        default:
            fprintf(stderr, "Unknown unary operator\n");
            return 0.0;
        }
    }
    
    default:
        fprintf(stderr, "Unknown node type\n");
        return 0.0;
    }
}