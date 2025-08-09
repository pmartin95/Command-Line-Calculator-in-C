#include "parser_utils.h"
#include "parser.h" // Need full definition for Parser struct access

int parser_utils_should_insert_multiplication(Parser *parser)
{
    if (!parser)
    {
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

int parser_utils_validate_token_sequence(Parser *parser)
{
    if (!parser)
    {
        return 0;
    }

    TokenType prev = parser->previous_token.type;
    TokenType curr = parser->current_token.type;

    // Check for invalid sequences
    if (prev == TOKEN_INVALID || curr == TOKEN_INVALID)
    {
        return 0;
    }

    // Two consecutive binary operators (except after unary)
    if (token_is_binary_op(prev) && token_is_binary_op(curr) &&
        !token_is_unary_op(curr))
    {
        return 0;
    }

    // Function followed by non-parenthesis (except constants that don't need parens)
    if (token_is_function(prev) && curr != TOKEN_LPAREN)
    {
        return 0;
    }

    // Comma not in function context would need more context to validate

    return 1; // Valid by default
}

void parser_utils_skip_to_statement_boundary(Parser *parser)
{
    if (!parser)
    {
        return;
    }

    // Skip tokens until we find a statement boundary
    while (parser->current_token.type != TOKEN_EOF &&
           parser->current_token.type != TOKEN_RPAREN && // End of expression group
           parser->current_token.type != TOKEN_COMMA)
    { // Potential argument separator
        parser_advance(parser);
    }
}

int parser_utils_at_expression_end(Parser *parser)
{
    if (!parser)
    {
        return 1;
    }

    TokenType curr = parser->current_token.type;

    return (curr == TOKEN_EOF ||
            curr == TOKEN_RPAREN ||
            curr == TOKEN_COMMA ||
            token_is_comparison_op(curr)); // Comparison ops have lower precedence
}

const char *parser_utils_get_context(Parser *parser)
{
    if (!parser)
    {
        return "unknown context";
    }

    TokenType curr = parser->current_token.type;

    if (curr == TOKEN_EOF)
    {
        return "end of input";
    }
    else if (token_is_function(curr))
    {
        return "function call";
    }
    else if (token_is_binary_op(curr))
    {
        return "binary operation";
    }
    else if (curr == TOKEN_LPAREN)
    {
        return "parenthesized expression";
    }
    else if (curr == TOKEN_INT || curr == TOKEN_FLOAT)
    {
        return "number";
    }
    else if (token_is_constant(curr))
    {
        return "constant";
    }
    else
    {
        return "expression";
    }
}