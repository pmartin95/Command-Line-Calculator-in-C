#include "printer.h"
#include "precision.h"
#include "function_table.h"
#include <stdio.h>

static int debug_level = 0;

void printer_print_ast(const ASTNode *node, int depth)
{
    if (!node) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("(null)\n");
        return;
    }

    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    switch (node->type) {
    case NODE_NUMBER:
        if (node->number.is_int && mpfr_fits_slong_p(node->number.value, global_rounding)) {
            long val = mpfr_get_si(node->number.value, global_rounding);
            printf("NUMBER: %ld\n", val);
        } else {
            mpfr_printf("NUMBER: %.6Rf\n", node->number.value);
        }
        break;

    case NODE_CONSTANT:
        printf("CONSTANT: %s\n", token_type_str(node->constant.const_type));
        break;

    case NODE_BINOP:
        printf("BINOP: %s\n", token_type_str(node->binop.op));
        printer_print_ast(node->binop.left, depth + 1);
        printer_print_ast(node->binop.right, depth + 1);
        break;

    case NODE_UNARY:
        printf("UNARY: %s\n", token_type_str(node->unary.op));
        printer_print_ast(node->unary.operand, depth + 1);
        break;

    case NODE_FUNCTION:
        printf("FUNCTION: %s (%d args)\n",
               token_type_str(node->function.func_type),
               node->function.arg_count);
        for (int i = 0; i < node->function.arg_count; i++) {
            printer_print_ast(node->function.args[i], depth + 1);
        }
        break;
    }
}

void printer_print_ast_compact(const ASTNode *node)
{
    if (!node) {
        printf("(null)");
        return;
    }

    switch (node->type) {
    case NODE_NUMBER:
        if (node->number.is_int && mpfr_fits_slong_p(node->number.value, global_rounding)) {
            long val = mpfr_get_si(node->number.value, global_rounding);
            printf("%ld", val);
        } else {
            mpfr_printf("%.6Rf", node->number.value);
        }
        break;

    case NODE_CONSTANT:
        printf("%s", token_type_str(node->constant.const_type));
        break;

    case NODE_BINOP:
        printf("(");
        printer_print_ast_compact(node->binop.left);
        printf(" %s ", token_type_str(node->binop.op));
        printer_print_ast_compact(node->binop.right);
        printf(")");
        break;

    case NODE_UNARY:
        printf("(%s", token_type_str(node->unary.op));
        printer_print_ast_compact(node->unary.operand);
        printf(")");
        break;

    case NODE_FUNCTION:
        printf("%s(", function_table_get_name(node->function.func_type));
        for (int i = 0; i < node->function.arg_count; i++) {
            if (i > 0) printf(", ");
            printer_print_ast_compact(node->function.args[i]);
        }
        printf(")");
        break;
    }
}

void printer_print_ast_infix(const ASTNode *node)
{
    if (!node) {
        printf("(null)");
        return;
    }

    switch (node->type) {
    case NODE_NUMBER:
        if (node->number.is_int && mpfr_fits_slong_p(node->number.value, global_rounding)) {
            long val = mpfr_get_si(node->number.value, global_rounding);
            printf("%ld", val);
        } else {
            mpfr_printf("%.6Rf", node->number.value);
        }
        break;

    case NODE_CONSTANT:
        switch (node->constant.const_type) {
        case TOKEN_PI:
            printf("π");
            break;
        case TOKEN_E:
            printf("e");
            break;
        default:
            printf("%s", token_type_str(node->constant.const_type));
        }
        break;

    case NODE_BINOP:
        // Add parentheses based on operator precedence
        {
            int need_left_parens = 0;
            int need_right_parens = 0;
            
            if (node->binop.left->type == NODE_BINOP) {
                int left_prec = token_get_precedence(node->binop.left->binop.op);
                int curr_prec = token_get_precedence(node->binop.op);
                need_left_parens = (left_prec < curr_prec);
            }
            
            if (node->binop.right->type == NODE_BINOP) {
                int right_prec = token_get_precedence(node->binop.right->binop.op);
                int curr_prec = token_get_precedence(node->binop.op);
                need_right_parens = (right_prec < curr_prec) || 
                                   (right_prec == curr_prec && !token_is_right_associative(node->binop.op));
            }
            
            if (need_left_parens) printf("(");
            printer_print_ast_infix(node->binop.left);
            if (need_left_parens) printf(")");
            
            // Print operator with appropriate spacing
            switch (node->binop.op) {
            case TOKEN_PLUS: printf(" + "); break;
            case TOKEN_MINUS: printf(" - "); break;
            case TOKEN_STAR: printf(" × "); break;
            case TOKEN_SLASH: printf(" ÷ "); break;
            case TOKEN_CARET: printf("^"); break;
            case TOKEN_EQ: printf(" = "); break;
            case TOKEN_NEQ: printf(" ≠ "); break;
            case TOKEN_LT: printf(" < "); break;
            case TOKEN_LTE: printf(" ≤ "); break;
            case TOKEN_GT: printf(" > "); break;
            case TOKEN_GTE: printf(" ≥ "); break;
            default: printf(" %s ", token_type_str(node->binop.op));
            }
            
            if (need_right_parens) printf("(");
            printer_print_ast_infix(node->binop.right);
            if (need_right_parens) printf(")");
        }
        break;

    case NODE_UNARY:
        if (node->unary.op == TOKEN_MINUS) {
            printf("-");
        } else if (node->unary.op == TOKEN_PLUS) {
            printf("+");
        }
        
        // Add parentheses if operand is a binary operation
        if (node->unary.operand->type == NODE_BINOP) {
            printf("(");
            printer_print_ast_infix(node->unary.operand);
            printf(")");
        } else {
            printer_print_ast_infix(node->unary.operand);
        }
        break;

    case NODE_FUNCTION:
        printf("%s(", function_table_get_name(node->function.func_type));
        for (int i = 0; i < node->function.arg_count; i++) {
            if (i > 0) printf(", ");
            printer_print_ast_infix(node->function.args[i]);
        }
        printf(")");
        break;
    }
}

void printer_print_token(const Token *token)
{
    if (!token) {
        printf("Token: (null)\n");
        return;
    }

    printf("Token: %s", token_type_str(token->type));
    
    switch (token->type) {
    case TOKEN_INT:
        printf(" (value: %d)", token->int_value);
        break;
    case TOKEN_FLOAT:
        printf(" (value: %g)", token->float_value);
        break;
    case TOKEN_IDENTIFIER:
        printf(" (string: \"%s\")", token->string_value ? token->string_value : "(null)");
        break;
    default:
        break;
    }
    
    if (token->number_string) {
        printf(" (number_string: \"%s\")", token->number_string);
    }
    
    printf("\n");
}

void printer_print_lexer_state(const void *lexer_ptr)
{
    // This would need the actual Lexer struct definition
    // For now, just indicate debug info is available
    printf("Lexer state: [debug info available at level %d]\n", debug_level);
    (void)lexer_ptr; // Suppress unused warning
}

void printer_print_parser_state(const void *parser_ptr)
{
    // This would need the actual Parser struct definition  
    // For now, just indicate debug info is available
    printf("Parser state: [debug info available at level %d]\n", debug_level);
    (void)parser_ptr; // Suppress unused warning
}

void printer_set_debug_level(int level)
{
    if (level < 0) level = 0;
    if (level > 3) level = 3;
    debug_level = level;
}

int printer_get_debug_level(void)
{
    return debug_level;
}