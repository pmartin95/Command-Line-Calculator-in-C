#include "parser.h"
#include "ast.h"
#include "function_table.h"
#include <stdio.h>
#include <stdlib.h>

// Maximum recursion depth to prevent stack overflow
#define MAX_RECURSION_DEPTH 100

// Forward declarations for static functions
static ASTNode *parse_expression_impl(Parser *parser);
static ASTNode *parse_comparison_impl(Parser *parser);
static ASTNode *parse_term_impl(Parser *parser);
static ASTNode *parse_factor_impl(Parser *parser);
static ASTNode *parse_power_impl(Parser *parser);
static ASTNode *parse_unary_impl(Parser *parser);
static ASTNode *parse_primary_impl(Parser *parser);

// Helper function for implicit multiplication detection
static int should_insert_multiplication(Parser *parser)
{
    if (!parser) {
        return 0;
    }

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
        ((prev == TOKEN_INT || prev == TOKEN_FLOAT) && token_is_function(curr)) ||
        // ')' followed by function
        (prev == TOKEN_RPAREN && token_is_function(curr)) ||
        // Number or ')' followed by constant
        ((prev == TOKEN_INT || prev == TOKEN_FLOAT || prev == TOKEN_RPAREN) &&
         token_is_constant(curr)) ||
        // Constant followed by number or '('
        (token_is_constant(prev) &&
         (curr == TOKEN_INT || curr == TOKEN_FLOAT || curr == TOKEN_LPAREN)));
}

void parser_init(Parser *parser, Lexer *lexer)
{
    if (!parser) return;

    parser->lexer = lexer;
    parser->previous_token = (Token){.type = TOKEN_INVALID};
    parser->recursion_depth = 0;
    parser->max_depth = MAX_RECURSION_DEPTH;
    parser->error_occurred = 0;

    if (lexer) {
        parser->current_token = lexer_get_next_token(lexer);
    } else {
        parser->current_token = (Token){.type = TOKEN_EOF};
    }
}

void parser_advance(Parser *parser)
{
    if (!parser || !parser->lexer) return;

    // Free the previous token if needed
    token_free(&parser->previous_token);

    parser->previous_token = parser->current_token;
    parser->current_token = lexer_get_next_token(parser->lexer);
}

// Wrapper functions with recursion depth checking
#define CHECK_RECURSION_DEPTH(parser, func_name) \
    do { \
        if (!parser) return NULL; \
        if (parser->recursion_depth >= parser->max_depth) { \
            fprintf(stderr, "Maximum recursion depth exceeded in %s\n", func_name); \
            parser->error_occurred = 1; \
            return NULL; \
        } \
        parser->recursion_depth++; \
    } while (0)

#define RETURN_WITH_DEPTH_DECREMENT(parser, result) \
    do { \
        if (parser) parser->recursion_depth--; \
        return result; \
    } while (0)

ASTNode *parser_parse_expression(Parser *parser)
{
    CHECK_RECURSION_DEPTH(parser, "parse_expression");
    ASTNode *result = parse_expression_impl(parser);
    RETURN_WITH_DEPTH_DECREMENT(parser, result);
}

static ASTNode *parse_expression_impl(Parser *parser)
{
    return parse_comparison_impl(parser);
}

ASTNode *parser_parse_comparison(Parser *parser)
{
    CHECK_RECURSION_DEPTH(parser, "parse_comparison");
    ASTNode *result = parse_comparison_impl(parser);
    RETURN_WITH_DEPTH_DECREMENT(parser, result);
}

static ASTNode *parse_comparison_impl(Parser *parser)
{
    ASTNode *left = parse_term_impl(parser);
    if (!left || parser->error_occurred) return left;

    while (token_is_comparison_op(parser->current_token.type)) {
        TokenType op = parser->current_token.type;
        parser_advance(parser);
        ASTNode *right = parse_term_impl(parser);
        if (!right || parser->error_occurred) {
            ast_free(left);
            ast_free(right);
            return NULL;
        }
        left = ast_create_binop(op, left, right);
        if (!left) return NULL;
    }

    return left;
}

ASTNode *parser_parse_term(Parser *parser)
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
           parser->current_token.type == TOKEN_MINUS) {
        TokenType op = parser->current_token.type;
        parser_advance(parser);
        ASTNode *right = parse_factor_impl(parser);
        if (!right || parser->error_occurred) {
            ast_free(left);
            ast_free(right);
            return NULL;
        }
        left = ast_create_binop(op, left, right);
        if (!left) return NULL;
    }

    return left;
}

ASTNode *parser_parse_factor(Parser *parser)
{
    CHECK_RECURSION_DEPTH(parser, "parse_factor");
    ASTNode *result = parse_factor_impl(parser);
    RETURN_WITH_DEPTH_DECREMENT(parser, result);
}

static ASTNode *parse_factor_impl(Parser *parser)
{
    ASTNode *left = parse_power_impl(parser);
    if (!left || parser->error_occurred) return left;

    // Handle explicit and implicit multiplication
    int implicit_mult_count = 0;
    const int max_implicit_mult = 1000;

    while ((parser->current_token.type == TOKEN_STAR ||
            parser->current_token.type == TOKEN_SLASH ||
            should_insert_multiplication(parser)) &&
           implicit_mult_count < max_implicit_mult) {
        
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
            ast_free(left);
            ast_free(right);
            return NULL;
        }
        left = ast_create_binop(op, left, right);
        if (!left) return NULL;
    }

    if (implicit_mult_count >= max_implicit_mult) {
        fprintf(stderr, "Too many implicit multiplications detected\n");
        parser->error_occurred = 1;
        ast_free(left);
        return NULL;
    }

    return left;
}

ASTNode *parser_parse_power(Parser *parser)
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
        ASTNode *right = parse_power_impl(parser); // Recursive for right-associativity
        if (!right || parser->error_occurred) {
            ast_free(left);
            ast_free(right);
            return NULL;
        }
        left = ast_create_binop(TOKEN_CARET, left, right);
    }

    return left;
}

ASTNode *parser_parse_unary(Parser *parser)
{
    CHECK_RECURSION_DEPTH(parser, "parse_unary");
    ASTNode *result = parse_unary_impl(parser);
    RETURN_WITH_DEPTH_DECREMENT(parser, result);
}

static ASTNode *parse_unary_impl(Parser *parser)
{
    if (token_is_unary_op(parser->current_token.type)) {
        TokenType op = parser->current_token.type;
        parser_advance(parser);
        ASTNode *operand = parse_unary_impl(parser); // Allow chaining: --5
        if (!operand || parser->error_occurred) {
            return NULL;
        }
        return ast_create_unary(op, operand);
    }

    return parse_primary_impl(parser);
}

ASTNode *parser_parse_primary(Parser *parser)
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
    case TOKEN_FLOAT:
    {
        parser_advance(parser);
        // Use the stored number string for MPFR parsing
        if (token.number_string) {
            return ast_create_number(token.number_string, token.type == TOKEN_INT);
        } else {
            // Fallback for backwards compatibility
            char temp_str[64];
            if (token.type == TOKEN_INT) {
                snprintf(temp_str, sizeof(temp_str), "%d", token.int_value);
            } else {
                snprintf(temp_str, sizeof(temp_str), "%.17g", token.float_value);
            }
            return ast_create_number(temp_str, token.type == TOKEN_INT);
        }
    }

    case TOKEN_LPAREN:
    {
        parser_advance(parser);
        ASTNode *expr = parse_expression_impl(parser);
        if (!expr || parser->error_occurred) {
            return NULL;
        }
        if (parser->current_token.type != TOKEN_RPAREN) {
            fprintf(stderr, "Expected ')'\n");
            parser->error_occurred = 1;
            ast_free(expr);
            return NULL;
        }
        parser_advance(parser);
        return expr;
    }

    case TOKEN_PI:
    case TOKEN_E:
        parser_advance(parser);
        return ast_create_constant(token.type);

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
        if (token_is_function(token.type)) {
            parser_advance(parser);
            return parser_parse_function_call(parser, token.type);
        }

        fprintf(stderr, "Unexpected token: %s\n", token_type_str(token.type));
        parser->error_occurred = 1;
        return NULL;
    }
}

ASTNode *parser_parse_function_call(Parser *parser, TokenType func_type)
{
    int expected_args = function_table_get_arg_count(func_type);

    // Expect opening parenthesis
    if (parser->current_token.type != TOKEN_LPAREN) {
        fprintf(stderr, "Expected '(' after function %s\n", function_table_get_name(func_type));
        parser->error_occurred = 1;
        return NULL;
    }
    parser_advance(parser); // consume '('

    // Parse arguments
    ASTNode **args = NULL;
    int arg_count = 0;

    if (expected_args > 0) {
        args = malloc(expected_args * sizeof(ASTNode *));
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
                    ast_free(args[i]);
                }
                free(args);
                return NULL;
            }
            arg_count++;
        }

        // Check if we have the right number of arguments
        if (arg_count != expected_args) {
            fprintf(stderr, "Function %s expects %d arguments, got %d\n",
                    function_table_get_name(func_type), expected_args, arg_count);
            parser->error_occurred = 1;
            for (int i = 0; i < arg_count; i++) {
                ast_free(args[i]);
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
                ast_free(args[i]);
            }
            free(args);
        }
        return NULL;
    }
    parser_advance(parser); // consume ')'

    return ast_create_function(func_type, args, arg_count);
}

// Additional parser utility functions from parser.h

int parser_has_error(Parser *parser)
{
    return parser ? parser->error_occurred : 1;
}

int parser_get_recursion_depth(Parser *parser)
{
    return parser ? parser->recursion_depth : 0;
}

void parser_set_max_recursion_depth(Parser *parser, int max_depth)
{
    if (parser) {
        parser->max_depth = max_depth;
    }
}

void parser_clear_error(Parser *parser)
{
    if (parser) {
        parser->error_occurred = 0;
    }
}

const char *parser_get_error_message(Parser *parser)
{
    if (parser && parser->error_occurred) {
        return "Parse error occurred";
    }
    return NULL;
}

int parser_at_end(Parser *parser)
{
    return parser ? (parser->current_token.type == TOKEN_EOF) : 1;
}

TokenType parser_current_token_type(Parser *parser)
{
    return parser ? parser->current_token.type : TOKEN_INVALID;
}

TokenType parser_previous_token_type(Parser *parser)
{
    return parser ? parser->previous_token.type : TOKEN_INVALID;
}

const Token *parser_peek_token(Parser *parser)
{
    return parser ? &parser->current_token : NULL;
}

int parser_match_token(Parser *parser, TokenType expected)
{
    return parser ? (parser->current_token.type == expected) : 0;
}

int parser_consume_token(Parser *parser, TokenType expected)
{
    if (parser && parser->current_token.type == expected) {
        parser_advance(parser);
        return 1;
    }
    return 0;
}

void parser_synchronize(Parser *parser)
{
    if (!parser) return;
    
    // Skip tokens until we find a synchronization point
    while (parser->current_token.type != TOKEN_EOF) {
        if (parser->current_token.type == TOKEN_RPAREN ||
            parser->current_token.type == TOKEN_COMMA) {
            break;
        }
        parser_advance(parser);
    }
}

void parser_panic(Parser *parser, const char *error_msg)
{
    if (parser) {
        parser->error_occurred = 1;
        if (error_msg) {
            fprintf(stderr, "Parser panic: %s\n", error_msg);
        }
        parser_synchronize(parser);
    }
}

int parser_is_panicking(Parser *parser)
{
    return parser ? parser->error_occurred : 1;
}