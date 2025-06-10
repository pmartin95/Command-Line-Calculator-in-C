#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

// Maximum recursion depth to prevent stack overflow
#define MAX_RECURSION_DEPTH 100

// Mathematical constants
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

int is_function_token(TokenType type)
{
    return (type >= TOKEN_SIN && type <= TOKEN_POW);
}

int is_constant_token(TokenType type)
{
    return (type == TOKEN_PI || type == TOKEN_E);
}

void init_parser(Parser *parser, Lexer *lexer)
{
    if (!parser) return;
    
    parser->lexer = lexer;
    parser->previous_token = (Token){.type = TOKEN_INVALID};
    parser->recursion_depth = 0;
    parser->max_depth = MAX_RECURSION_DEPTH;
    parser->error_occurred = 0;
    
    if (lexer) {
        parser->current_token = get_next_token(lexer);
    } else {
        parser->current_token = (Token){.type = TOKEN_EOF};
    }
}

void parser_advance(Parser *parser)
{
    if (!parser || !parser->lexer) return;
    
    // Free the previous token if it's an identifier
    free_token(&parser->previous_token);
    
    parser->previous_token = parser->current_token;
    parser->current_token = get_next_token(parser->lexer);
}

int should_insert_multiplication(Parser *parser)
{
    if (!parser) return 0;
    
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
         (curr == TOKEN_INT || curr == TOKEN_FLOAT)) ||
        // Number followed by function
        ((prev == TOKEN_INT || prev == TOKEN_FLOAT) && is_function_token(curr)) ||
        // ')' followed by function
        (prev == TOKEN_RPAREN && is_function_token(curr)) ||
        // Number or ')' followed by constant
        ((prev == TOKEN_INT || prev == TOKEN_FLOAT || prev == TOKEN_RPAREN) && 
         is_constant_token(curr)) ||
        // Constant followed by number or '('
        (is_constant_token(prev) && 
         (curr == TOKEN_INT || curr == TOKEN_FLOAT || curr == TOKEN_LPAREN))
    );
}

ASTNode *create_number_node(double value, int is_int)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    node->type = NODE_NUMBER;
    node->number.value = value;
    node->number.is_int = is_int;
    return node;
}

ASTNode *create_binop_node(TokenType op, ASTNode *left, ASTNode *right)
{
    if (!left || !right) {
        free_ast(left);
        free_ast(right);
        return NULL;
    }
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        free_ast(left);
        free_ast(right);
        return NULL;
    }
    node->type = NODE_BINOP;
    node->binop.op = op;
    node->binop.left = left;
    node->binop.right = right;
    return node;
}

ASTNode *create_unary_node(TokenType op, ASTNode *operand)
{
    if (!operand) {
        return NULL;
    }
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        free_ast(operand);
        return NULL;
    }
    node->type = NODE_UNARY;
    node->unary.op = op;
    node->unary.operand = operand;
    return node;
}

ASTNode *create_function_node(TokenType func_type, ASTNode **args, int arg_count)
{
    if (!args && arg_count > 0) {
        return NULL;
    }
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        if (args) {
            for (int i = 0; i < arg_count; i++) {
                free_ast(args[i]);
            }
            free(args);
        }
        return NULL;
    }
    
    node->type = NODE_FUNCTION;
    node->function.func_type = func_type;
    node->function.args = args;
    node->function.arg_count = arg_count;
    return node;
}

ASTNode *create_constant_node(TokenType const_type)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    node->type = NODE_CONSTANT;
    node->constant.const_type = const_type;
    return node;
}

void free_ast(ASTNode *node)
{
    if (!node) return;
    
    switch (node->type) {
    case NODE_NUMBER:
    case NODE_CONSTANT:
        break;
    case NODE_BINOP:
        free_ast(node->binop.left);
        free_ast(node->binop.right);
        break;
    case NODE_UNARY:
        free_ast(node->unary.operand);
        break;
    case NODE_FUNCTION:
        if (node->function.args) {
            for (int i = 0; i < node->function.arg_count; i++) {
                free_ast(node->function.args[i]);
            }
            free(node->function.args);
        }
        break;
    }
    free(node);
}

// Forward declarations for recursion depth checking
static ASTNode *parse_expression_impl(Parser *parser);
static ASTNode *parse_comparison_impl(Parser *parser);
static ASTNode *parse_term_impl(Parser *parser);
static ASTNode *parse_factor_impl(Parser *parser);
static ASTNode *parse_power_impl(Parser *parser);
static ASTNode *parse_unary_impl(Parser *parser);
static ASTNode *parse_primary_impl(Parser *parser);

// Wrapper functions that check recursion depth
#define CHECK_RECURSION_DEPTH(parser, func_name) \
    do { \
        if (!parser) return NULL; \
        if (parser->recursion_depth >= parser->max_depth) { \
            fprintf(stderr, "Maximum recursion depth exceeded in %s\n", func_name); \
            parser->error_occurred = 1; \
            return NULL; \
        } \
        parser->recursion_depth++; \
    } while(0)

#define RETURN_WITH_DEPTH_DECREMENT(parser, result) \
    do { \
        if (parser) parser->recursion_depth--; \
        return result; \
    } while(0)

ASTNode *parse_expression(Parser *parser)
{
    CHECK_RECURSION_DEPTH(parser, "parse_expression");
    ASTNode *result = parse_expression_impl(parser);
    RETURN_WITH_DEPTH_DECREMENT(parser, result);
}

static ASTNode *parse_expression_impl(Parser *parser)
{
    return parse_comparison_impl(parser);
}

ASTNode *parse_comparison(Parser *parser)
{
    CHECK_RECURSION_DEPTH(parser, "parse_comparison");
    ASTNode *result = parse_comparison_impl(parser);
    RETURN_WITH_DEPTH_DECREMENT(parser, result);
}

static ASTNode *parse_comparison_impl(Parser *parser)
{
    ASTNode *left = parse_term_impl(parser);
    if (!left || parser->error_occurred) return left;
    
    while (parser->current_token.type == TOKEN_EQ ||
           parser->current_token.type == TOKEN_NEQ ||
           parser->current_token.type == TOKEN_LT ||
           parser->current_token.type == TOKEN_LTE ||
           parser->current_token.type == TOKEN_GT ||
           parser->current_token.type == TOKEN_GTE)
    {
        TokenType op = parser->current_token.type;
        parser_advance(parser);
        ASTNode *right = parse_term_impl(parser);
        if (!right || parser->error_occurred) {
            free_ast(left);
            free_ast(right);
            return NULL;
        }
        left = create_binop_node(op, left, right);
        if (!left) return NULL;
    }
    
    return left;
}

ASTNode *parse_term(Parser *parser)
{
    CHECK_RECURSION_DEPTH(parser, "parse_term");
    ASTNode *result = parse_term_impl(parser);
    RETURN_WITH_DEPTH_DECREMENT(parser, result);
}

static ASTNode *parse_term_impl(Parser *parser)
{
    ASTNode *left = parse_factor_impl(parser);
    if (!left || parser->error_occurred) return left;
    
    while (parser->current_token.type == TOKEN_PLUS ||
           parser->current_token.type == TOKEN_MINUS)
    {
        TokenType op = parser->current_token.type;
        parser_advance(parser);
        ASTNode *right = parse_factor_impl(parser);
        if (!right || parser->error_occurred) {
            free_ast(left);
            free_ast(right);
            return NULL;
        }
        left = create_binop_node(op, left, right);
        if (!left) return NULL;
    }
    
    return left;
}

ASTNode *parse_factor(Parser *parser)
{
    CHECK_RECURSION_DEPTH(parser, "parse_factor");
    ASTNode *result = parse_factor_impl(parser);
    RETURN_WITH_DEPTH_DECREMENT(parser, result);
}

static ASTNode *parse_factor_impl(Parser *parser)
{
    ASTNode *left = parse_power_impl(parser);
    if (!left || parser->error_occurred) return left;
    
    // Prevent infinite loops in implicit multiplication
    int implicit_mult_count = 0;
    const int max_implicit_mult = 1000;
    
    while ((parser->current_token.type == TOKEN_STAR ||
            parser->current_token.type == TOKEN_SLASH ||
            should_insert_multiplication(parser)) &&
           implicit_mult_count < max_implicit_mult)
    {
        TokenType op = parser->current_token.type;
        
        // Handle implicit multiplication
        if (should_insert_multiplication(parser)) {
            op = TOKEN_STAR;
            implicit_mult_count++;
            // Don't advance - we're inserting a virtual token
        } else {
            parser_advance(parser);
        }
        
        ASTNode *right = parse_power_impl(parser);
        if (!right || parser->error_occurred) {
            free_ast(left);
            free_ast(right);
            return NULL;
        }
        left = create_binop_node(op, left, right);
        if (!left) return NULL;
    }
    
    if (implicit_mult_count >= max_implicit_mult) {
        fprintf(stderr, "Too many implicit multiplications detected\n");
        parser->error_occurred = 1;
        free_ast(left);
        return NULL;
    }
    
    return left;
}

ASTNode *parse_power(Parser *parser)
{
    CHECK_RECURSION_DEPTH(parser, "parse_power");
    ASTNode *result = parse_power_impl(parser);
    RETURN_WITH_DEPTH_DECREMENT(parser, result);
}

static ASTNode *parse_power_impl(Parser *parser)
{
    ASTNode *left = parse_unary_impl(parser);
    if (!left || parser->error_occurred) return left;
    
    // Right-associative
    if (parser->current_token.type == TOKEN_CARET) {
        parser_advance(parser);
        ASTNode *right = parse_power_impl(parser);  // Recursive for right-associativity
        if (!right || parser->error_occurred) {
            free_ast(left);
            free_ast(right);
            return NULL;
        }
        left = create_binop_node(TOKEN_CARET, left, right);
    }
    
    return left;
}

ASTNode *parse_unary(Parser *parser)
{
    CHECK_RECURSION_DEPTH(parser, "parse_unary");
    ASTNode *result = parse_unary_impl(parser);
    RETURN_WITH_DEPTH_DECREMENT(parser, result);
}

static ASTNode *parse_unary_impl(Parser *parser)
{
    if (parser->current_token.type == TOKEN_PLUS ||
        parser->current_token.type == TOKEN_MINUS)
    {
        TokenType op = parser->current_token.type;
        parser_advance(parser);
        ASTNode *operand = parse_unary_impl(parser);  // Allow chaining: --5
        if (!operand || parser->error_occurred) {
            return NULL;
        }
        return create_unary_node(op, operand);
    }
    
    return parse_primary_impl(parser);
}

ASTNode *parse_function_call(Parser *parser, TokenType func_type)
{
    int expected_args = get_function_arg_count(func_type);
    
    // Expect opening parenthesis
    if (parser->current_token.type != TOKEN_LPAREN) {
        fprintf(stderr, "Expected '(' after function %s\n", get_function_name(func_type));
        parser->error_occurred = 1;
        return NULL;
    }
    parser_advance(parser); // consume '('
    
    // Parse arguments
    ASTNode **args = NULL;
    int arg_count = 0;
    
    if (expected_args > 0) {
        args = malloc(expected_args * sizeof(ASTNode*));
        if (!args) {
            fprintf(stderr, "Memory allocation failed\n");
            parser->error_occurred = 1;
            return NULL;
        }
        
        // Initialize to NULL for safe cleanup
        for (int i = 0; i < expected_args; i++) {
            args[i] = NULL;
        }
        
        // Parse first argument
        args[0] = parse_expression_impl(parser);
        if (!args[0] || parser->error_occurred) {
            free(args);
            return NULL;
        }
        arg_count = 1;
        
        // Parse remaining arguments
        while (arg_count < expected_args && parser->current_token.type == TOKEN_COMMA) {
            parser_advance(parser); // consume ','
            args[arg_count] = parse_expression_impl(parser);
            if (!args[arg_count] || parser->error_occurred) {
                for (int i = 0; i < arg_count; i++) {
                    free_ast(args[i]);
                }
                free(args);
                return NULL;
            }
            arg_count++;
        }
        
        // Check if we have the right number of arguments
        if (arg_count != expected_args) {
            fprintf(stderr, "Function %s expects %d arguments, got %d\n", 
                    get_function_name(func_type), expected_args, arg_count);
            parser->error_occurred = 1;
            for (int i = 0; i < arg_count; i++) {
                free_ast(args[i]);
            }
            free(args);
            return NULL;
        }
    }
    
    // Expect closing parenthesis
    if (parser->current_token.type != TOKEN_RPAREN) {
        fprintf(stderr, "Expected ')' after function arguments\n");
        parser->error_occurred = 1;
        if (args) {
            for (int i = 0; i < arg_count; i++) {
                free_ast(args[i]);
            }
            free(args);
        }
        return NULL;
    }
    parser_advance(parser); // consume ')'
    
    return create_function_node(func_type, args, arg_count);
}

ASTNode *parse_primary(Parser *parser)
{
    CHECK_RECURSION_DEPTH(parser, "parse_primary");
    ASTNode *result = parse_primary_impl(parser);
    RETURN_WITH_DEPTH_DECREMENT(parser, result);
}

static ASTNode *parse_primary_impl(Parser *parser)
{
    Token token = parser->current_token;
    
    switch (token.type) {
    case TOKEN_INT:
        parser_advance(parser);
        return create_number_node(token.int_value, 1);
        
    case TOKEN_FLOAT:
        parser_advance(parser);
        return create_number_node(token.float_value, 0);
        
    case TOKEN_LPAREN: {
        parser_advance(parser);
        ASTNode *expr = parse_expression_impl(parser);
        if (!expr || parser->error_occurred) {
            return NULL;
        }
        if (parser->current_token.type != TOKEN_RPAREN) {
            fprintf(stderr, "Expected ')'\n");
            parser->error_occurred = 1;
            free_ast(expr);
            return NULL;
        }
        parser_advance(parser);
        return expr;
    }
    
    case TOKEN_PI:
    case TOKEN_E:
        parser_advance(parser);
        return create_constant_node(token.type);
    
    case TOKEN_INVALID:
        fprintf(stderr, "Invalid token encountered\n");
        parser->error_occurred = 1;
        return NULL;
        
    case TOKEN_IDENTIFIER:
        fprintf(stderr, "Unknown function or variable: %s\n", token.string_value);
        parser->error_occurred = 1;
        return NULL;
        
    default:
        // Check if it's a function
        if (is_function_token(token.type)) {
            parser_advance(parser);
            return parse_function_call(parser, token.type);
        }
        
        fprintf(stderr, "Unexpected token: %s\n", token_type_str(token.type));
        parser->error_occurred = 1;
        return NULL;
    }
}

double evaluate_ast(const ASTNode *node)
{
    if (!node) return 0.0;
    
    switch (node->type) {
    case NODE_NUMBER:
        return node->number.value;
        
    case NODE_CONSTANT:
        switch (node->constant.const_type) {
        case TOKEN_PI:
            return M_PI;
        case TOKEN_E:
            return M_E;
        default:
            fprintf(stderr, "Unknown constant\n");
            return 0.0;
        }
        
    case NODE_BINOP: {
        double left = evaluate_ast(node->binop.left);
        double right = evaluate_ast(node->binop.right);
        
        switch (node->binop.op) {
        case TOKEN_PLUS:  
            return left + right;
        case TOKEN_MINUS: 
            return left - right;
        case TOKEN_STAR:  
            return left * right;
        case TOKEN_SLASH:
            if (right == 0.0) {
                fprintf(stderr, "Division by zero\n");
                return 0.0;
            }
            return left / right;
        case TOKEN_CARET: {
            // Handle domain errors for exponentiation
            if (left < 0 && right != floor(right)) {
                fprintf(stderr, "Complex result not supported (negative base with non-integer exponent)\n");
                return 0.0;
            }
            
            errno = 0;
            double result = pow(left, right);
            
            // Check for overflow/underflow
            if (errno == ERANGE || isinf(result)) {
                fprintf(stderr, "Result overflow in exponentiation\n");
                return 0.0;
            }
            if (isnan(result)) {
                fprintf(stderr, "Invalid mathematical operation in exponentiation\n");
                return 0.0;
            }
            
            return result;
        }
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
    
    case NODE_FUNCTION: {
        // Evaluate all arguments first
        double args[2]; // Maximum 2 arguments for any function we support
        for (int i = 0; i < node->function.arg_count; i++) {
            args[i] = evaluate_ast(node->function.args[i]);
        }
        
        switch (node->function.func_type) {
        // Trigonometric functions
        case TOKEN_SIN:
            return sin(args[0]);
        case TOKEN_COS:
            return cos(args[0]);
        case TOKEN_TAN:
            return tan(args[0]);
            
        // Inverse trigonometric functions
        case TOKEN_ASIN:
            if (args[0] < -1.0 || args[0] > 1.0) {
                fprintf(stderr, "Domain error: asin argument must be in [-1, 1]\n");
                return 0.0;
            }
            return asin(args[0]);
        case TOKEN_ACOS:
            if (args[0] < -1.0 || args[0] > 1.0) {
                fprintf(stderr, "Domain error: acos argument must be in [-1, 1]\n");
                return 0.0;
            }
            return acos(args[0]);
        case TOKEN_ATAN:
            return atan(args[0]);
        case TOKEN_ATAN2:
            return atan2(args[0], args[1]);
            
        // Hyperbolic functions
        case TOKEN_SINH:
            return sinh(args[0]);
        case TOKEN_COSH:
            return cosh(args[0]);
        case TOKEN_TANH:
            return tanh(args[0]);
            
        // Inverse hyperbolic functions
        case TOKEN_ASINH:
            return asinh(args[0]);
        case TOKEN_ACOSH:
            if (args[0] < 1.0) {
                fprintf(stderr, "Domain error: acosh argument must be >= 1\n");
                return 0.0;
            }
            return acosh(args[0]);
        case TOKEN_ATANH:
            if (args[0] <= -1.0 || args[0] >= 1.0) {
                fprintf(stderr, "Domain error: atanh argument must be in (-1, 1)\n");
                return 0.0;
            }
            return atanh(args[0]);
            
        // Other mathematical functions
        case TOKEN_SQRT:
            if (args[0] < 0.0) {
                fprintf(stderr, "Domain error: sqrt of negative number\n");
                return 0.0;
            }
            return sqrt(args[0]);
        case TOKEN_LOG:
            if (args[0] <= 0.0) {
                fprintf(stderr, "Domain error: log of non-positive number\n");
                return 0.0;
            }
            return log(args[0]);
        case TOKEN_LOG10:
            if (args[0] <= 0.0) {
                fprintf(stderr, "Domain error: log10 of non-positive number\n");
                return 0.0;
            }
            return log10(args[0]);
        case TOKEN_EXP: {
            errno = 0;
            double result = exp(args[0]);
            if (errno == ERANGE || isinf(result)) {
                fprintf(stderr, "Overflow in exp function\n");
                return 0.0;
            }
            return result;
        }
        case TOKEN_ABS:
            return fabs(args[0]);
        case TOKEN_FLOOR:
            return floor(args[0]);
        case TOKEN_CEIL:
            return ceil(args[0]);
        case TOKEN_POW: {
            // Handle domain errors for exponentiation
            if (args[0] < 0 && args[1] != floor(args[1])) {
                fprintf(stderr, "Complex result not supported (negative base with non-integer exponent)\n");
                return 0.0;
            }
            
            errno = 0;
            double result = pow(args[0], args[1]);
            
            // Check for overflow/underflow
            if (errno == ERANGE || isinf(result)) {
                fprintf(stderr, "Result overflow in pow function\n");
                return 0.0;
            }
            if (isnan(result)) {
                fprintf(stderr, "Invalid mathematical operation in pow function\n");
                return 0.0;
            }
            
            return result;
        }
        default:
            fprintf(stderr, "Unknown function\n");
            return 0.0;
        }
    }
    
    default:
        fprintf(stderr, "Unknown node type\n");
        return 0.0;
    }
}