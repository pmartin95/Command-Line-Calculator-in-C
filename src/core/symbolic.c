#include "symbolic.h"
#include "precision.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Error handling
static char last_error[256] = {0};

// Forward declarations
static ASTNode *symbolic_simplify_binop(TokenType op, ASTNode *left, ASTNode *right);
static ASTNode *symbolic_simplify_unary(TokenType op, ASTNode *operand);
static ASTNode *symbolic_simplify_function(TokenType func_type, ASTNode **args, int arg_count);

// Helper function to check if node matches a specific constant
static int matches_constant(const ASTNode *node, const char *const_name)
{
    if (!node || !const_name)
        return 0;

    if (node->type == NODE_CONSTANT)
    {
        return strcmp(node->constant.name, const_name) == 0;
    }

    return 0;
}

// Helper function to check if node represents a division (fraction)
static int is_fraction(const ASTNode *node, ASTNode **num, ASTNode **denom)
{
    if (!node)
        return 0;

    if (node->type == NODE_BINOP && node->binop.op == TOKEN_SLASH)
    {
        if (num) *num = node->binop.left;
        if (denom) *denom = node->binop.right;
        return 1;
    }

    return 0;
}

// Helper function to compare nodes for canonical ordering
// Returns: -1 if a < b, 0 if a == b, 1 if a > b
static int node_compare(const ASTNode *a, const ASTNode *b)
{
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;

    // Order by node type first: NUMBER < CONSTANT < FUNCTION < BINOP < UNARY
    if (a->type != b->type)
    {
        return a->type - b->type;
    }

    switch (a->type)
    {
    case NODE_NUMBER:
        // Compare numeric values
        return mpfr_cmp(a->number.value, b->number.value);

    case NODE_CONSTANT:
        // Compare alphabetically
        return strcmp(a->constant.name, b->constant.name);

    case NODE_FUNCTION:
        if (a->function.func_type != b->function.func_type)
            return a->function.func_type - b->function.func_type;
        if (a->function.arg_count != b->function.arg_count)
            return a->function.arg_count - b->function.arg_count;
        // Compare arguments
        for (int i = 0; i < a->function.arg_count; i++)
        {
            int cmp = node_compare(a->function.args[i], b->function.args[i]);
            if (cmp != 0) return cmp;
        }
        return 0;

    case NODE_BINOP:
        if (a->binop.op != b->binop.op)
            return a->binop.op - b->binop.op;
        int left_cmp = node_compare(a->binop.left, b->binop.left);
        if (left_cmp != 0) return left_cmp;
        return node_compare(a->binop.right, b->binop.right);

    case NODE_UNARY:
        if (a->unary.op != b->unary.op)
            return a->unary.op - b->unary.op;
        return node_compare(a->unary.operand, b->unary.operand);

    default:
        return 0;
    }
}

// Helper function to check if operation is commutative
static int is_commutative(TokenType op)
{
    return (op == TOKEN_PLUS || op == TOKEN_STAR);
}

ASTNode *symbolic_eval(const ASTNode *node)
{
    symbolic_clear_error();

    if (!node)
    {
        return NULL;
    }

    switch (node->type)
    {
    case NODE_NUMBER:
    case NODE_CONSTANT:
        // Numbers and constants stay as-is
        return symbolic_clone(node);

    case NODE_BINOP:
    {
        // Recursively simplify children
        ASTNode *left = symbolic_eval(node->binop.left);
        ASTNode *right = symbolic_eval(node->binop.right);

        // Try to simplify the operation
        ASTNode *result = symbolic_simplify_binop(node->binop.op, left, right);

        return result;
    }

    case NODE_UNARY:
    {
        // Recursively simplify operand
        ASTNode *operand = symbolic_eval(node->unary.operand);

        // Try to simplify
        ASTNode *result = symbolic_simplify_unary(node->unary.op, operand);

        return result;
    }

    case NODE_FUNCTION:
    {
        // Recursively simplify arguments
        ASTNode **args = malloc(sizeof(ASTNode *) * node->function.arg_count);
        for (int i = 0; i < node->function.arg_count; i++)
        {
            args[i] = symbolic_eval(node->function.args[i]);
        }

        // Try to simplify function
        ASTNode *result = symbolic_simplify_function(node->function.func_type, args, node->function.arg_count);

        return result;
    }

    default:
        snprintf(last_error, sizeof(last_error), "Unknown node type in symbolic evaluation");
        return symbolic_clone(node);
    }
}

static ASTNode *symbolic_simplify_binop(TokenType op, ASTNode *left, ASTNode *right)
{
    if (!left || !right)
    {
        return ast_create_binop(op, left, right);
    }

    // Canonicalize commutative operations (ensure consistent ordering)
    if (is_commutative(op) && node_compare(left, right) > 0)
    {
        // Swap operands to maintain canonical order
        ASTNode *temp = left;
        left = right;
        right = temp;
    }

    // Arithmetic identity rules
    switch (op)
    {
    case TOKEN_PLUS:
        // x + 0 → x
        if (symbolic_is_zero(right))
        {
            ast_free(right);
            return left;
        }
        // 0 + x → x
        if (symbolic_is_zero(left))
        {
            ast_free(left);
            return right;
        }
        // a + b → c (if both are integers, compute the result)
        if (left->type == NODE_NUMBER && left->number.is_int &&
            right->type == NODE_NUMBER && right->number.is_int)
        {
            mpfr_t result;
            mpfr_init2(result, mpfr_get_prec(left->number.value));
            mpfr_add(result, left->number.value, right->number.value, global_rounding);

            char buffer[256];
            mpfr_sprintf(buffer, "%.0Rf", result);
            mpfr_clear(result);

            ast_free(left);
            ast_free(right);
            return ast_create_number(buffer, 1);
        }
        // x + x → 2×x (like-term collection)
        if (symbolic_equals(left, right))
        {
            ASTNode *two = ast_create_number("2", 1);
            ast_free(right);
            return symbolic_simplify_binop(TOKEN_STAR, two, left);
        }
        // (a×x) + x → (a+1)×x
        if (left->type == NODE_BINOP && left->binop.op == TOKEN_STAR &&
            symbolic_equals(left->binop.right, right))
        {
            ASTNode *a = left->binop.left;
            ASTNode *one = ast_create_number("1", 1);
            ASTNode *x = left->binop.right;
            ast_free(right);
            free(left);

            // Compute a + 1
            ASTNode *a_plus_1 = symbolic_simplify_binop(TOKEN_PLUS, a, one);
            return symbolic_simplify_binop(TOKEN_STAR, a_plus_1, x);
        }
        // x + (b×x) → (1+b)×x
        if (right->type == NODE_BINOP && right->binop.op == TOKEN_STAR &&
            symbolic_equals(left, right->binop.right))
        {
            ASTNode *b = right->binop.left;
            ASTNode *one = ast_create_number("1", 1);
            ASTNode *x = right->binop.right;
            ast_free(left);
            free(right);

            // Compute 1 + b
            ASTNode *one_plus_b = symbolic_simplify_binop(TOKEN_PLUS, one, b);
            return symbolic_simplify_binop(TOKEN_STAR, one_plus_b, x);
        }
        // (a×x) + (b×x) → (a+b)×x
        if (left->type == NODE_BINOP && left->binop.op == TOKEN_STAR &&
            right->type == NODE_BINOP && right->binop.op == TOKEN_STAR &&
            symbolic_equals(left->binop.right, right->binop.right))
        {
            ASTNode *a = left->binop.left;
            ASTNode *b = right->binop.left;
            ASTNode *x = symbolic_clone(left->binop.right);

            ast_free(left->binop.right);
            ast_free(right->binop.right);
            free(left);
            free(right);

            // Compute a + b
            ASTNode *a_plus_b = symbolic_simplify_binop(TOKEN_PLUS, a, b);
            return symbolic_simplify_binop(TOKEN_STAR, a_plus_b, x);
        }
        break;

    case TOKEN_MINUS:
        // x - 0 → x
        if (symbolic_is_zero(right))
        {
            ast_free(right);
            return left;
        }
        // x - x → 0 (if structurally equal)
        if (symbolic_equals(left, right))
        {
            ast_free(left);
            ast_free(right);
            return ast_create_number("0", 1);
        }
        break;

    case TOKEN_STAR:
        // x × 0 → 0
        if (symbolic_is_zero(left) || symbolic_is_zero(right))
        {
            ast_free(left);
            ast_free(right);
            return ast_create_number("0", 1);
        }
        // x × 1 → x
        if (symbolic_is_one(right))
        {
            ast_free(right);
            return left;
        }
        // 1 × x → x
        if (symbolic_is_one(left))
        {
            ast_free(left);
            return right;
        }
        // a × b → c (if both are integers, compute the result)
        if (left->type == NODE_NUMBER && left->number.is_int &&
            right->type == NODE_NUMBER && right->number.is_int)
        {
            mpfr_t result;
            mpfr_init2(result, mpfr_get_prec(left->number.value));
            mpfr_mul(result, left->number.value, right->number.value, global_rounding);

            char buffer[256];
            mpfr_sprintf(buffer, "%.0Rf", result);
            mpfr_clear(result);

            ast_free(left);
            ast_free(right);
            return ast_create_number(buffer, 1);
        }
        // sqrt(a) × sqrt(b) → sqrt(a×b)
        if (left->type == NODE_FUNCTION && left->function.func_type == TOKEN_SQRT &&
            right->type == NODE_FUNCTION && right->function.func_type == TOKEN_SQRT)
        {
            ASTNode *a = left->function.args[0];
            ASTNode *b = right->function.args[0];

            // Clone the arguments
            ASTNode *a_clone = symbolic_clone(a);
            ASTNode *b_clone = symbolic_clone(b);

            ast_free(left);
            ast_free(right);

            // Create a × b
            ASTNode *product = symbolic_simplify_binop(TOKEN_STAR, a_clone, b_clone);

            // Return sqrt(a × b)
            ASTNode **sqrt_args = malloc(sizeof(ASTNode *));
            sqrt_args[0] = product;
            return symbolic_simplify_function(TOKEN_SQRT, sqrt_args, 1);
        }
        break;

    case TOKEN_SLASH:
        // x ÷ 0 → error (division by zero)
        if (symbolic_is_zero(right))
        {
            snprintf(last_error, sizeof(last_error), "Division by zero");
            ast_free(left);
            ast_free(right);
            return ast_create_number("0", 1); // Return 0 as fallback
        }
        // x ÷ 1 → x
        if (symbolic_is_one(right))
        {
            ast_free(right);
            return left;
        }
        // x ÷ x → 1 (if structurally equal and not zero)
        if (symbolic_equals(left, right) && !symbolic_is_zero(left))
        {
            ast_free(left);
            ast_free(right);
            return ast_create_number("1", 1);
        }
        // 0 ÷ x → 0 (if x is not zero)
        if (symbolic_is_zero(left))
        {
            ast_free(left);
            ast_free(right);
            return ast_create_number("0", 1);
        }
        // (a × b) ÷ b → a (cancel common factor)
        if (left->type == NODE_BINOP && left->binop.op == TOKEN_STAR)
        {
            if (symbolic_equals(left->binop.right, right))
            {
                // Left side is a × b, and b matches denominator
                ASTNode *a = left->binop.left;
                ast_free(left->binop.right);
                ast_free(right);
                free(left);
                return a;
            }
            if (symbolic_equals(left->binop.left, right))
            {
                // Left side is a × b, and a matches denominator (commutative)
                ASTNode *b = left->binop.right;
                ast_free(left->binop.left);
                ast_free(right);
                free(left);
                return b;
            }
        }
        // (a ÷ b) ÷ b → a ÷ (b × b) - not implemented yet
        // b ÷ (a × b) → 1 ÷ a (cancel common factor in denominator)
        if (right->type == NODE_BINOP && right->binop.op == TOKEN_STAR)
        {
            if (symbolic_equals(right->binop.right, left))
            {
                // Denominator is a × b, numerator is b
                ASTNode *a = right->binop.left;
                ast_free(right->binop.right);
                ast_free(left);
                free(right);
                ASTNode *one = ast_create_number("1", 1);
                return symbolic_simplify_binop(TOKEN_SLASH, one, a);
            }
            if (symbolic_equals(right->binop.left, left))
            {
                // Denominator is a × b, numerator is a (commutative)
                ASTNode *b = right->binop.right;
                ast_free(right->binop.left);
                ast_free(left);
                free(right);
                ASTNode *one = ast_create_number("1", 1);
                return symbolic_simplify_binop(TOKEN_SLASH, one, b);
            }
        }
        // (a + b) ÷ c → a÷c + b÷c (distribute division over addition)
        if (left->type == NODE_BINOP && left->binop.op == TOKEN_PLUS)
        {
            ASTNode *a = symbolic_clone(left->binop.left);
            ASTNode *b = symbolic_clone(left->binop.right);
            ASTNode *c1 = symbolic_clone(right);
            ASTNode *c2 = symbolic_clone(right);

            ast_free(left);
            ast_free(right);

            // Recursively simplify a/c and b/c
            ASTNode *a_div_c = symbolic_simplify_binop(TOKEN_SLASH, a, c1);
            ASTNode *b_div_c = symbolic_simplify_binop(TOKEN_SLASH, b, c2);

            // Return a/c + b/c
            return symbolic_simplify_binop(TOKEN_PLUS, a_div_c, b_div_c);
        }
        // (a - b) ÷ c → a÷c - b÷c (distribute division over subtraction)
        if (left->type == NODE_BINOP && left->binop.op == TOKEN_MINUS)
        {
            ASTNode *a = symbolic_clone(left->binop.left);
            ASTNode *b = symbolic_clone(left->binop.right);
            ASTNode *c1 = symbolic_clone(right);
            ASTNode *c2 = symbolic_clone(right);

            ast_free(left);
            ast_free(right);

            // Recursively simplify a/c and b/c
            ASTNode *a_div_c = symbolic_simplify_binop(TOKEN_SLASH, a, c1);
            ASTNode *b_div_c = symbolic_simplify_binop(TOKEN_SLASH, b, c2);

            // Return a/c - b/c
            return symbolic_simplify_binop(TOKEN_MINUS, a_div_c, b_div_c);
        }
        // a ÷ b → c (if both are integers and division is exact)
        if (left->type == NODE_NUMBER && left->number.is_int &&
            right->type == NODE_NUMBER && right->number.is_int &&
            !symbolic_is_zero(right))
        {
            mpfr_t result;
            mpfr_init2(result, mpfr_get_prec(left->number.value));
            mpfr_div(result, left->number.value, right->number.value, global_rounding);

            // Only simplify if result is an integer (exact division)
            if (mpfr_integer_p(result))
            {
                char buffer[256];
                mpfr_sprintf(buffer, "%.0Rf", result);
                mpfr_clear(result);

                ast_free(left);
                ast_free(right);
                return ast_create_number(buffer, 1);
            }
            mpfr_clear(result);
        }
        // a ÷ sqrt(b) → a×sqrt(b) ÷ b (rationalize denominator)
        if (right->type == NODE_FUNCTION && right->function.func_type == TOKEN_SQRT)
        {
            ASTNode *b = right->function.args[0];

            // Clone the nodes we need
            ASTNode *a = left;
            ASTNode *sqrt_b = symbolic_clone(right);
            ASTNode *b_clone = symbolic_clone(b);

            ast_free(right);

            // Create a × sqrt(b)
            ASTNode *numerator = symbolic_simplify_binop(TOKEN_STAR, a, sqrt_b);

            // Return (a × sqrt(b)) ÷ b
            return symbolic_simplify_binop(TOKEN_SLASH, numerator, b_clone);
        }
        break;

    case TOKEN_CARET:
        // x^0 → 1
        if (symbolic_is_zero(right))
        {
            ast_free(left);
            ast_free(right);
            return ast_create_number("1", 1);
        }
        // x^1 → x
        if (symbolic_is_one(right))
        {
            ast_free(right);
            return left;
        }
        // 0^x → 0 (for positive x)
        if (symbolic_is_zero(left))
        {
            ast_free(left);
            ast_free(right);
            return ast_create_number("0", 1);
        }
        // 1^x → 1
        if (symbolic_is_one(left))
        {
            ast_free(left);
            ast_free(right);
            return ast_create_number("1", 1);
        }
        break;

    default:
        break;
    }

    // No simplification applied, create the binop node
    return ast_create_binop(op, left, right);
}

static ASTNode *symbolic_simplify_unary(TokenType op, ASTNode *operand)
{
    // For now, just create the unary operation node
    // Simplification rules will be added in Phase 2
    return ast_create_unary(op, operand);
}

static ASTNode *symbolic_simplify_function(TokenType func_type, ASTNode **args, int arg_count)
{
    if (!args || arg_count == 0)
    {
        return ast_create_function(func_type, args, arg_count);
    }

    ASTNode *arg = args[0];

    // Trigonometric simplification for exact values
    switch (func_type)
    {
    case TOKEN_SIN:
        // sin(0) → 0
        if (symbolic_is_zero(arg))
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("0", 1);
        }
        // sin(π) → 0
        if (matches_constant(arg, "pi"))
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("0", 1);
        }
        // sin(π/2) → 1
        ASTNode *num, *denom;
        if (is_fraction(arg, &num, &denom) &&
            matches_constant(num, "pi") &&
            arg->type == NODE_BINOP &&
            arg->binop.op == TOKEN_SLASH &&
            symbolic_is_one(denom) == 0 &&
            denom->type == NODE_NUMBER &&
            mpfr_cmp_ui(denom->number.value, 2) == 0)
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("1", 1);
        }
        break;

    case TOKEN_COS:
        // cos(0) → 1
        if (symbolic_is_zero(arg))
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("1", 1);
        }
        // cos(π) → -1
        if (matches_constant(arg, "pi"))
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("-1", 1);
        }
        // cos(π/2) → 0
        if (is_fraction(arg, &num, &denom) &&
            matches_constant(num, "pi") &&
            arg->type == NODE_BINOP &&
            arg->binop.op == TOKEN_SLASH &&
            denom->type == NODE_NUMBER &&
            mpfr_cmp_ui(denom->number.value, 2) == 0)
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("0", 1);
        }
        break;

    case TOKEN_TAN:
        // tan(0) → 0
        if (symbolic_is_zero(arg))
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("0", 1);
        }
        // tan(π) → 0
        if (matches_constant(arg, "pi"))
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("0", 1);
        }
        // tan(π/4) → 1
        if (is_fraction(arg, &num, &denom) &&
            matches_constant(num, "pi") &&
            arg->type == NODE_BINOP &&
            arg->binop.op == TOKEN_SLASH &&
            denom->type == NODE_NUMBER &&
            mpfr_cmp_ui(denom->number.value, 4) == 0)
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("1", 1);
        }
        break;

    case TOKEN_LOG:
        // log(1) → 0 (natural logarithm)
        if (symbolic_is_one(arg))
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("0", 1);
        }
        // log(e) → 1
        if (matches_constant(arg, "e"))
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("1", 1);
        }
        break;

    case TOKEN_LOG10:
        // log10(1) → 0
        if (symbolic_is_one(arg))
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("0", 1);
        }
        // log10(10) → 1
        if (arg->type == NODE_NUMBER && arg->number.is_int &&
            mpfr_cmp_ui(arg->number.value, 10) == 0)
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("1", 1);
        }
        break;

    case TOKEN_EXP:
        // exp(0) → 1
        if (symbolic_is_zero(arg))
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("1", 1);
        }
        // exp(1) → e
        if (symbolic_is_one(arg))
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_constant("e");
        }
        break;

    case TOKEN_ABS:
        // abs(0) → 0
        if (symbolic_is_zero(arg))
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("0", 1);
        }
        // abs(x) → x (if x is a positive number)
        if (arg->type == NODE_NUMBER && mpfr_sgn(arg->number.value) >= 0)
        {
            // Already positive, return as-is
            free(args);
            return arg;
        }
        // abs(x) → -x (if x is a negative number)
        if (arg->type == NODE_NUMBER && mpfr_sgn(arg->number.value) < 0)
        {
            mpfr_t result;
            mpfr_init2(result, mpfr_get_prec(arg->number.value));
            mpfr_abs(result, arg->number.value, global_rounding);

            char buffer[256];
            mpfr_sprintf(buffer, "%.0Rf", result);
            mpfr_clear(result);

            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number(buffer, arg->number.is_int);
        }
        break;

    case TOKEN_SQRT:
        // sqrt(0) → 0
        if (symbolic_is_zero(arg))
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("0", 1);
        }
        // sqrt(1) → 1
        if (symbolic_is_one(arg))
        {
            for (int i = 0; i < arg_count; i++)
                ast_free(args[i]);
            free(args);
            return ast_create_number("1", 1);
        }
        // sqrt(4) → 2, sqrt(9) → 3, etc. (perfect squares)
        // Also: sqrt(8) → 2×sqrt(2), sqrt(12) → 2×sqrt(3) (radicand simplification)
        if (arg->type == NODE_NUMBER && arg->number.is_int)
        {
            mpfr_t sqrt_val;
            mpfr_init2(sqrt_val, mpfr_get_prec(arg->number.value));
            mpfr_sqrt(sqrt_val, arg->number.value, global_rounding);

            // Check if it's a perfect square
            if (mpfr_integer_p(sqrt_val))
            {
                for (int i = 0; i < arg_count; i++)
                    ast_free(args[i]);
                free(args);

                // Convert to string and create number node
                char buffer[256];
                mpfr_sprintf(buffer, "%.0Rf", sqrt_val);
                mpfr_clear(sqrt_val);
                return ast_create_number(buffer, 1);
            }

            // Not a perfect square - try to simplify radicand
            // Find largest perfect square factor
            unsigned long n = mpfr_get_ui(arg->number.value, global_rounding);
            if (n > 1)
            {
                unsigned long largest_square = 1;
                unsigned long largest_root = 1;

                // Check for perfect square factors
                for (unsigned long i = 2; i * i <= n; i++)
                {
                    if (n % (i * i) == 0)
                    {
                        largest_square = i * i;
                        largest_root = i;
                    }
                }

                // If we found a perfect square factor, simplify
                if (largest_square > 1)
                {
                    unsigned long remaining = n / largest_square;

                    for (int i = 0; i < arg_count; i++)
                        ast_free(args[i]);
                    free(args);

                    // Create: largest_root × sqrt(remaining)
                    char root_str[64], remaining_str[64];
                    snprintf(root_str, sizeof(root_str), "%lu", largest_root);
                    snprintf(remaining_str, sizeof(remaining_str), "%lu", remaining);

                    ASTNode *root_node = ast_create_number(root_str, 1);
                    ASTNode **sqrt_args = malloc(sizeof(ASTNode *));
                    sqrt_args[0] = ast_create_number(remaining_str, 1);
                    ASTNode *sqrt_node = ast_create_function(TOKEN_SQRT, sqrt_args, 1);

                    return ast_create_binop(TOKEN_STAR, root_node, sqrt_node);
                }
            }

            mpfr_clear(sqrt_val);
        }
        break;

    default:
        break;
    }

    // No simplification applied
    return ast_create_function(func_type, args, arg_count);
}

ASTNode *symbolic_clone(const ASTNode *node)
{
    if (!node)
    {
        return NULL;
    }

    switch (node->type)
    {
    case NODE_NUMBER:
    {
        // Create new number node with same value
        ASTNode *clone = malloc(sizeof(ASTNode));
        if (!clone)
            return NULL;

        clone->type = NODE_NUMBER;
        mpfr_init2(clone->number.value, mpfr_get_prec(node->number.value));
        mpfr_set(clone->number.value, node->number.value, global_rounding);
        clone->number.is_int = node->number.is_int;

        return clone;
    }

    case NODE_CONSTANT:
    {
        // Create new constant node with same name
        return ast_create_constant(node->constant.name);
    }

    case NODE_BINOP:
    {
        // Recursively clone children
        ASTNode *left = symbolic_clone(node->binop.left);
        ASTNode *right = symbolic_clone(node->binop.right);
        return ast_create_binop(node->binop.op, left, right);
    }

    case NODE_UNARY:
    {
        // Recursively clone operand
        ASTNode *operand = symbolic_clone(node->unary.operand);
        return ast_create_unary(node->unary.op, operand);
    }

    case NODE_FUNCTION:
    {
        // Recursively clone arguments
        ASTNode **args = malloc(sizeof(ASTNode *) * node->function.arg_count);
        for (int i = 0; i < node->function.arg_count; i++)
        {
            args[i] = symbolic_clone(node->function.args[i]);
        }
        return ast_create_function(node->function.func_type, args, node->function.arg_count);
    }

    default:
        return NULL;
    }
}

int symbolic_equals(const ASTNode *a, const ASTNode *b)
{
    if (!a && !b)
        return 1;
    if (!a || !b)
        return 0;

    if (a->type != b->type)
        return 0;

    switch (a->type)
    {
    case NODE_NUMBER:
        return mpfr_equal_p(a->number.value, b->number.value) &&
               (a->number.is_int == b->number.is_int);

    case NODE_CONSTANT:
        return strcmp(a->constant.name, b->constant.name) == 0;

    case NODE_BINOP:
        return (a->binop.op == b->binop.op) &&
               symbolic_equals(a->binop.left, b->binop.left) &&
               symbolic_equals(a->binop.right, b->binop.right);

    case NODE_UNARY:
        return (a->unary.op == b->unary.op) &&
               symbolic_equals(a->unary.operand, b->unary.operand);

    case NODE_FUNCTION:
        if (a->function.func_type != b->function.func_type)
            return 0;
        if (a->function.arg_count != b->function.arg_count)
            return 0;
        for (int i = 0; i < a->function.arg_count; i++)
        {
            if (!symbolic_equals(a->function.args[i], b->function.args[i]))
                return 0;
        }
        return 1;

    default:
        return 0;
    }
}

int symbolic_is_zero(const ASTNode *node)
{
    if (!node)
        return 0;

    if (node->type == NODE_NUMBER)
    {
        return mpfr_zero_p(node->number.value);
    }

    return 0;
}

int symbolic_is_one(const ASTNode *node)
{
    if (!node)
        return 0;

    if (node->type == NODE_NUMBER)
    {
        return mpfr_cmp_ui(node->number.value, 1) == 0;
    }

    return 0;
}

int symbolic_is_integer(const ASTNode *node)
{
    if (!node)
        return 0;

    if (node->type == NODE_NUMBER)
    {
        return node->number.is_int && mpfr_integer_p(node->number.value);
    }

    return 0;
}

const char *symbolic_get_last_error(void)
{
    return strlen(last_error) > 0 ? last_error : NULL;
}

void symbolic_clear_error(void)
{
    last_error[0] = '\0';
}
