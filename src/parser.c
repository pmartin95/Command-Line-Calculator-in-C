#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

// Global precision settings
mpfr_prec_t global_precision = DEFAULT_PRECISION;
mpfr_rnd_t global_rounding = MPFR_RNDN; // Round to nearest

// Maximum recursion depth to prevent stack overflow
#define MAX_RECURSION_DEPTH 100

// Mathematical constants
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

void set_precision(mpfr_prec_t prec)
{
    if (prec < MIN_PRECISION)
        prec = MIN_PRECISION;
    if (prec > MAX_PRECISION)
        prec = MAX_PRECISION;
    global_precision = prec;
    mpfr_set_default_prec(prec);
}

mpfr_prec_t get_precision(void)
{
    return global_precision;
}

void print_precision_info(void)
{
    printf("Current precision: %ld bits (approximately %ld decimal digits)\n",
           (long)global_precision,
           (long)(global_precision * 0.30103)); // log10(2) ≈ 0.30103
}

void print_mpfr_result(const mpfr_t value, int original_is_int)
{
    // Check if the number is an integer and not too large
    if (original_is_int && mpfr_integer_p(value)) {
        // Try to print as integer if it fits in long long
        if (mpfr_fits_slong_p(value, global_rounding)) {
            long int_val = mpfr_get_si(value, global_rounding);
            printf("= %ld\n", int_val);
            return;
        }
    }
    
    // Calculate number of decimal digits to show based on precision
    long decimal_digits = (long)(global_precision * 0.30103); // log10(2) ≈ 0.30103
    if (decimal_digits < 20) decimal_digits = 20;
    if (decimal_digits > 150) decimal_digits = 150; // Cap for reasonable output
    
    // For very small numbers, use scientific notation
    mpfr_t abs_val;
    mpfr_init2(abs_val, global_precision);
    mpfr_abs(abs_val, value, global_rounding);
    
    // Get the string representation
    char *str = NULL;
    mpfr_exp_t exp;
    
    if (!mpfr_zero_p(value) && mpfr_cmp_d(abs_val, 1e-6) < 0) {
        // Scientific notation for small numbers
        str = mpfr_get_str(NULL, &exp, 10, decimal_digits, value, global_rounding);
        if (str) {
            printf("= %c.%se%+ld\n", str[0], str + 1, (long)(exp - 1));
            mpfr_free_str(str);
        }
    } else if (mpfr_cmp_d(abs_val, 1e15) > 0) {
        // Scientific notation for large numbers
        str = mpfr_get_str(NULL, &exp, 10, decimal_digits, value, global_rounding);
        if (str) {
            printf("= %c.%se%+ld\n", str[0], str + 1, (long)(exp - 1));
            mpfr_free_str(str);
        }
    } else {
        // Regular notation for normal-sized numbers - this is where we do smart trimming
        str = mpfr_get_str(NULL, &exp, 10, decimal_digits, value, global_rounding);
        if (str) {
            // Handle the formatting manually to trim trailing zeros intelligently
            size_t len = strlen(str);
            int is_negative = (str[0] == '-');
            size_t start_idx = is_negative ? 1 : 0;  // Changed to size_t
            
            // Find the position where we should place the decimal point
            long int_digits = exp;
            
            if (int_digits <= 0) {
                // Number is less than 1, like 0.00123
                printf("= %s0.", is_negative ? "-" : "");
                // Print leading zeros
                for (long i = 0; i < -int_digits; i++) {
                    printf("0");
                }
                // Print the significant digits, trimming trailing zeros
                char *trimmed = str + start_idx;
                size_t trimmed_len = len - start_idx;
                
                // Trim trailing zeros
                while (trimmed_len > 1 && trimmed[trimmed_len - 1] == '0') {
                    trimmed_len--;
                }
                
                printf("%.*s\n", (int)trimmed_len, trimmed);
            } else if (int_digits >= (long)(len - start_idx)) {
                // Integer part uses all digits, like 12300
                printf("= %.*s", (int)(len - start_idx), str + start_idx);
                // Add trailing zeros for the integer part if needed
                for (long i = (long)(len - start_idx); i < int_digits; i++) {
                    printf("0");
                }
                printf("\n");
            } else {
                // Normal case: some digits before decimal, some after
                printf("= %s", is_negative ? "-" : "");
                
                // Print integer part
                printf("%.*s", (int)int_digits, str + start_idx);
                
                // Print decimal point and fractional part
                char *frac_part = str + start_idx + int_digits;
                size_t frac_len = len - start_idx - int_digits;
                
                if (frac_len > 0) {
                    // Trim trailing zeros from fractional part
                    while (frac_len > 0 && frac_part[frac_len - 1] == '0') {
                        frac_len--;
                    }
                    
                    if (frac_len > 0) {
                        printf(".%.*s", (int)frac_len, frac_part);
                    }
                }
                printf("\n");
            }
            
            mpfr_free_str(str);
        }
    }
    
    mpfr_clear(abs_val);
}

// Fixed version of the smart printing function:

void print_mpfr_smart(const mpfr_t value)
{
    // This is an alternative implementation that's even smarter about significant digits
    if (mpfr_zero_p(value)) {
        printf("= 0\n");
        return;
    }
    
    // Get string with maximum precision
    char *str = NULL;
    mpfr_exp_t exp;
    long max_digits = (long)(global_precision * 0.30103);
    if (max_digits > 200) max_digits = 200;
    
    str = mpfr_get_str(NULL, &exp, 10, max_digits, value, global_rounding);
    if (!str) {
        printf("= [error formatting number]\n");
        return;
    }
    
    size_t len = strlen(str);
    int is_negative = (str[0] == '-');
    size_t start_idx = is_negative ? 1 : 0;  // Changed to size_t
    
    // Find the last significant digit (not zero)
    size_t last_significant = len - 1;
    while (last_significant > start_idx && str[last_significant] == '0') {
        last_significant--;
    }
    
    // Ensure we show at least one digit after decimal for very precise numbers
    long int_part_len = exp;
    if (int_part_len <= 0) {
        // For numbers like 0.00001, show a reasonable number of digits
        size_t min_frac_digits = (size_t)(-int_part_len) + 1;  // Cast to size_t
        if (last_significant < start_idx + min_frac_digits - 1) {
            last_significant = start_idx + min_frac_digits - 1;
        }
    } else if (int_part_len < (long)(len - start_idx)) {
        // For numbers like 1.00001, ensure we show the significant fractional part
        size_t min_total_digits = (size_t)int_part_len + 1; // Cast to size_t
        if (last_significant < start_idx + min_total_digits - 1) {
            last_significant = start_idx + min_total_digits - 1;
        }
    }
    
    printf("= %s", is_negative ? "-" : "");
    
    if (int_part_len <= 0) {
        // Number less than 1
        printf("0.");
        for (long i = 0; i < -int_part_len; i++) {
            printf("0");
        }
        printf("%.*s\n", (int)(last_significant - start_idx + 1), str + start_idx);
    } else if (int_part_len >= (long)(last_significant - start_idx + 1)) {
        // Pure integer (all significant digits are in integer part)
        printf("%.*s", (int)(last_significant - start_idx + 1), str + start_idx);
        for (long i = (long)(last_significant - start_idx + 1); i < int_part_len; i++) {
            printf("0");
        }
        printf("\n");
    } else {
        // Mixed: integer and fractional parts
        printf("%.*s", (int)int_part_len, str + start_idx);
        size_t decimal_start = start_idx + (size_t)int_part_len;  // Cast to size_t
        if (last_significant >= decimal_start) {
            printf(".%.*s", (int)(last_significant - decimal_start + 1), 
                   str + decimal_start);
        }
        printf("\n");
    }
    
    mpfr_free_str(str);
}

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
    if (!parser)
        return;

    parser->lexer = lexer;
    parser->previous_token = (Token){.type = TOKEN_INVALID};
    parser->recursion_depth = 0;
    parser->max_depth = 100;
    parser->error_occurred = 0;
    parser->precision = global_precision;

    if (lexer)
    {
        parser->current_token = get_next_token(lexer);
    }
    else
    {
        parser->current_token = (Token){.type = TOKEN_EOF};
    }
}

void parser_advance(Parser *parser)
{
    if (!parser || !parser->lexer)
        return;

    // Free the previous token if it's an identifier
    free_token(&parser->previous_token);

    parser->previous_token = parser->current_token;
    parser->current_token = get_next_token(parser->lexer);
}

int should_insert_multiplication(Parser *parser)
{
    if (!parser)
        return 0;

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
         (curr == TOKEN_INT || curr == TOKEN_FLOAT || curr == TOKEN_LPAREN)));
}

ASTNode *create_number_node_mpfr(const char *str, int is_int)
{
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    node->type = NODE_NUMBER;
    node->number.is_int = is_int;

    // Initialize MPFR number with current precision
    mpfr_init2(node->number.value, global_precision);

    // Parse the string with MPFR
    int ret = mpfr_set_str(node->number.value, str, 10, global_rounding);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to parse number: %s\n", str);
        mpfr_clear(node->number.value);
        free(node);
        return NULL;
    }

    return node;
}

ASTNode *create_binop_node(TokenType op, ASTNode *left, ASTNode *right)
{
    if (!left || !right)
    {
        free_ast(left);
        free_ast(right);
        return NULL;
    }

    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node)
    {
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
    if (!operand)
    {
        return NULL;
    }

    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node)
    {
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
    if (!args && arg_count > 0)
    {
        return NULL;
    }

    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node)
    {
        fprintf(stderr, "Memory allocation failed\n");
        if (args)
        {
            for (int i = 0; i < arg_count; i++)
            {
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
    if (!node)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    node->type = NODE_CONSTANT;
    node->constant.const_type = const_type;
    return node;
}

void free_ast(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_NUMBER:
        mpfr_clear(node->number.value);
        break;
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
        if (node->function.args)
        {
            for (int i = 0; i < node->function.arg_count; i++)
            {
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
#define CHECK_RECURSION_DEPTH(parser, func_name)                                    \
    do                                                                              \
    {                                                                               \
        if (!parser)                                                                \
            return NULL;                                                            \
        if (parser->recursion_depth >= parser->max_depth)                           \
        {                                                                           \
            fprintf(stderr, "Maximum recursion depth exceeded in %s\n", func_name); \
            parser->error_occurred = 1;                                             \
            return NULL;                                                            \
        }                                                                           \
        parser->recursion_depth++;                                                  \
    } while (0)

#define RETURN_WITH_DEPTH_DECREMENT(parser, result) \
    do                                              \
    {                                               \
        if (parser)                                 \
            parser->recursion_depth--;              \
        return result;                              \
    } while (0)

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
    if (!left || parser->error_occurred)
        return left;

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
        if (!right || parser->error_occurred)
        {
            free_ast(left);
            free_ast(right);
            return NULL;
        }
        left = create_binop_node(op, left, right);
        if (!left)
            return NULL;
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
    if (!left || parser->error_occurred)
        return left;

    while (parser->current_token.type == TOKEN_PLUS ||
           parser->current_token.type == TOKEN_MINUS)
    {
        TokenType op = parser->current_token.type;
        parser_advance(parser);
        ASTNode *right = parse_factor_impl(parser);
        if (!right || parser->error_occurred)
        {
            free_ast(left);
            free_ast(right);
            return NULL;
        }
        left = create_binop_node(op, left, right);
        if (!left)
            return NULL;
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
    if (!left || parser->error_occurred)
        return left;

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
        if (should_insert_multiplication(parser))
        {
            op = TOKEN_STAR;
            implicit_mult_count++;
            // Don't advance - we're inserting a virtual token
        }
        else
        {
            parser_advance(parser);
        }

        ASTNode *right = parse_power_impl(parser);
        if (!right || parser->error_occurred)
        {
            free_ast(left);
            free_ast(right);
            return NULL;
        }
        left = create_binop_node(op, left, right);
        if (!left)
            return NULL;
    }

    if (implicit_mult_count >= max_implicit_mult)
    {
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
    if (!left || parser->error_occurred)
        return left;

    // Right-associative
    if (parser->current_token.type == TOKEN_CARET)
    {
        parser_advance(parser);
        ASTNode *right = parse_power_impl(parser); // Recursive for right-associativity
        if (!right || parser->error_occurred)
        {
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
        ASTNode *operand = parse_unary_impl(parser); // Allow chaining: --5
        if (!operand || parser->error_occurred)
        {
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
    if (parser->current_token.type != TOKEN_LPAREN)
    {
        fprintf(stderr, "Expected '(' after function %s\n", get_function_name(func_type));
        parser->error_occurred = 1;
        return NULL;
    }
    parser_advance(parser); // consume '('

    // Parse arguments
    ASTNode **args = NULL;
    int arg_count = 0;

    if (expected_args > 0)
    {
        args = malloc(expected_args * sizeof(ASTNode *));
        if (!args)
        {
            fprintf(stderr, "Memory allocation failed\n");
            parser->error_occurred = 1;
            return NULL;
        }

        // Initialize to NULL for safe cleanup
        for (int i = 0; i < expected_args; i++)
        {
            args[i] = NULL;
        }

        // Parse first argument
        args[0] = parse_expression_impl(parser);
        if (!args[0] || parser->error_occurred)
        {
            free(args);
            return NULL;
        }
        arg_count = 1;

        // Parse remaining arguments
        while (arg_count < expected_args && parser->current_token.type == TOKEN_COMMA)
        {
            parser_advance(parser); // consume ','
            args[arg_count] = parse_expression_impl(parser);
            if (!args[arg_count] || parser->error_occurred)
            {
                for (int i = 0; i < arg_count; i++)
                {
                    free_ast(args[i]);
                }
                free(args);
                return NULL;
            }
            arg_count++;
        }

        // Check if we have the right number of arguments
        if (arg_count != expected_args)
        {
            fprintf(stderr, "Function %s expects %d arguments, got %d\n",
                    get_function_name(func_type), expected_args, arg_count);
            parser->error_occurred = 1;
            for (int i = 0; i < arg_count; i++)
            {
                free_ast(args[i]);
            }
            free(args);
            return NULL;
        }
    }

    // Expect closing parenthesis
    if (parser->current_token.type != TOKEN_RPAREN)
    {
        fprintf(stderr, "Expected ')' after function arguments\n");
        parser->error_occurred = 1;
        if (args)
        {
            for (int i = 0; i < arg_count; i++)
            {
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

    switch (token.type)
    {
    case TOKEN_INT:
    case TOKEN_FLOAT:
    {
        parser_advance(parser);
        // Use the stored number string for MPFR parsing
        if (token.number_string)
        {
            return create_number_node_mpfr(token.number_string, token.type == TOKEN_INT);
        }
        else
        {
            // Fallback for backwards compatibility
            char temp_str[64];
            if (token.type == TOKEN_INT)
            {
                snprintf(temp_str, sizeof(temp_str), "%d", token.int_value);
            }
            else
            {
                snprintf(temp_str, sizeof(temp_str), "%.17g", token.float_value);
            }
            return create_number_node_mpfr(temp_str, token.type == TOKEN_INT);
        }
    }

    case TOKEN_LPAREN:
    {
        parser_advance(parser);
        ASTNode *expr = parse_expression_impl(parser);
        if (!expr || parser->error_occurred)
        {
            return NULL;
        }
        if (parser->current_token.type != TOKEN_RPAREN)
        {
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
        if (is_function_token(token.type))
        {
            parser_advance(parser);
            return parse_function_call(parser, token.type);
        }

        fprintf(stderr, "Unexpected token: %s\n", token_type_str(token.type));
        parser->error_occurred = 1;
        return NULL;
    }
}

// In parser.c - Replace the evaluate_ast function with this (Part 1):

void evaluate_ast(mpfr_t result, const ASTNode *node)
{
    if (!node)
    {
        mpfr_set_d(result, 0.0, global_rounding);
        return;
    }

    switch (node->type)
    {
    case NODE_NUMBER:
        mpfr_set(result, node->number.value, global_rounding);
        break;

    case NODE_CONSTANT:
        switch (node->constant.const_type)
        {
        case TOKEN_PI:
            mpfr_const_pi(result, global_rounding);
            break;
        case TOKEN_E:
            mpfr_set_d(result, 1.0, global_rounding);
            mpfr_exp(result, result, global_rounding); // e = exp(1)
            break;
        default:
            fprintf(stderr, "Unknown constant\n");
            mpfr_set_d(result, 0.0, global_rounding);
        }
        break;

    case NODE_BINOP:
    {
        mpfr_t left, right;
        mpfr_init2(left, global_precision);
        mpfr_init2(right, global_precision);

        evaluate_ast(left, node->binop.left);
        evaluate_ast(right, node->binop.right);

        switch (node->binop.op)
        {
        case TOKEN_PLUS:
            mpfr_add(result, left, right, global_rounding);
            break;
        case TOKEN_MINUS:
            mpfr_sub(result, left, right, global_rounding);
            break;
        case TOKEN_STAR:
            mpfr_mul(result, left, right, global_rounding);
            break;
        case TOKEN_SLASH:
            if (mpfr_zero_p(right))
            {
                fprintf(stderr, "Division by zero\n");
                mpfr_set_d(result, 0.0, global_rounding);
            }
            else
            {
                mpfr_div(result, left, right, global_rounding);
            }
            break;
        case TOKEN_CARET:
            mpfr_pow(result, left, right, global_rounding);
            break;
        case TOKEN_EQ:
            mpfr_set_d(result, mpfr_equal_p(left, right) ? 1.0 : 0.0, global_rounding);
            break;
        case TOKEN_NEQ:
            mpfr_set_d(result, !mpfr_equal_p(left, right) ? 1.0 : 0.0, global_rounding);
            break;
        case TOKEN_LT:
            mpfr_set_d(result, mpfr_less_p(left, right) ? 1.0 : 0.0, global_rounding);
            break;
        case TOKEN_LTE:
            mpfr_set_d(result, mpfr_lessequal_p(left, right) ? 1.0 : 0.0, global_rounding);
            break;
        case TOKEN_GT:
            mpfr_set_d(result, mpfr_greater_p(left, right) ? 1.0 : 0.0, global_rounding);
            break;
        case TOKEN_GTE:
            mpfr_set_d(result, mpfr_greaterequal_p(left, right) ? 1.0 : 0.0, global_rounding);
            break;
        default:
            fprintf(stderr, "Unknown binary operator\n");
            mpfr_set_d(result, 0.0, global_rounding);
        }

        mpfr_clear(left);
        mpfr_clear(right);
        break;
    }
        // In parser.c - Continue the evaluate_ast function (Part 2):

    case NODE_UNARY:
    {
        mpfr_t operand;
        mpfr_init2(operand, global_precision);
        evaluate_ast(operand, node->unary.operand);

        switch (node->unary.op)
        {
        case TOKEN_PLUS:
            mpfr_set(result, operand, global_rounding);
            break;
        case TOKEN_MINUS:
            mpfr_neg(result, operand, global_rounding);
            break;
        default:
            fprintf(stderr, "Unknown unary operator\n");
            mpfr_set_d(result, 0.0, global_rounding);
        }

        mpfr_clear(operand);
        break;
    }

    case NODE_FUNCTION:
    {
        mpfr_t args[2];
        for (int i = 0; i < node->function.arg_count; i++)
        {
            mpfr_init2(args[i], global_precision);
            evaluate_ast(args[i], node->function.args[i]);
        }

        switch (node->function.func_type)
        {
        // Trigonometric functions
        case TOKEN_SIN:
            mpfr_sin(result, args[0], global_rounding);
            break;
        case TOKEN_COS:
            mpfr_cos(result, args[0], global_rounding);
            break;
        case TOKEN_TAN:
            mpfr_tan(result, args[0], global_rounding);
            break;

        // Inverse trigonometric functions
        case TOKEN_ASIN:
            mpfr_asin(result, args[0], global_rounding);
            break;
        case TOKEN_ACOS:
            mpfr_acos(result, args[0], global_rounding);
            break;
        case TOKEN_ATAN:
            mpfr_atan(result, args[0], global_rounding);
            break;
        case TOKEN_ATAN2:
            mpfr_atan2(result, args[0], args[1], global_rounding);
            break;

        // Hyperbolic functions
        case TOKEN_SINH:
            mpfr_sinh(result, args[0], global_rounding);
            break;
        case TOKEN_COSH:
            mpfr_cosh(result, args[0], global_rounding);
            break;
        case TOKEN_TANH:
            mpfr_tanh(result, args[0], global_rounding);
            break;

        // Inverse hyperbolic functions
        case TOKEN_ASINH:
            mpfr_asinh(result, args[0], global_rounding);
            break;
        case TOKEN_ACOSH:
            mpfr_acosh(result, args[0], global_rounding);
            break;
        case TOKEN_ATANH:
            mpfr_atanh(result, args[0], global_rounding);
            break;

        // Other mathematical functions
        case TOKEN_SQRT:
            mpfr_sqrt(result, args[0], global_rounding);
            break;
        case TOKEN_LOG:
            mpfr_log(result, args[0], global_rounding);
            break;
        case TOKEN_LOG10:
            mpfr_log10(result, args[0], global_rounding);
            break;
        case TOKEN_EXP:
            mpfr_exp(result, args[0], global_rounding);
            break;
        case TOKEN_ABS:
            mpfr_abs(result, args[0], global_rounding);
            break;
        case TOKEN_FLOOR:
            mpfr_floor(result, args[0]);
            break;
        case TOKEN_CEIL:
            mpfr_ceil(result, args[0]);
            break;
        case TOKEN_POW:
            mpfr_pow(result, args[0], args[1], global_rounding);
            break;
        default:
            fprintf(stderr, "Unknown function\n");
            mpfr_set_d(result, 0.0, global_rounding);
        }

        for (int i = 0; i < node->function.arg_count; i++)
        {
            mpfr_clear(args[i]);
        }
        break;
    }

    default:
        fprintf(stderr, "Unknown node type\n");
        mpfr_set_d(result, 0.0, global_rounding);
    }
}