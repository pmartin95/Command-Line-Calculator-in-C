#ifndef LEXER_H
#define LEXER_H

#include "tokens.h"
#include <stddef.h>

typedef struct
{
    const char *text;
    size_t pos;
    size_t input_length;
    char current_char;
} Lexer;

/**
 * Initialize lexer with input string
 * @param lexer Lexer instance
 * @param input Input string to tokenize
 */
void lexer_init(Lexer *lexer, const char *input);

/**
 * Get the next token from the input
 * @param lexer Lexer instance
 * @return Next token
 */
Token lexer_get_next_token(Lexer *lexer);

/**
 * Peek at the next character without consuming it
 * @param lexer Lexer instance
 * @return Next character or '\0' if at end
 */
char lexer_peek(Lexer *lexer);

/**
 * Peek ahead multiple characters
 * @param lexer Lexer instance
 * @param offset Number of characters to peek ahead
 * @return Character at offset or '\0' if beyond end
 */
char lexer_peek_ahead(Lexer *lexer, int offset);

/**
 * Check if lexer is at end of input
 * @param lexer Lexer instance
 * @return 1 if at end, 0 otherwise
 */
int lexer_at_end(Lexer *lexer);

/**
 * Get current position in input
 * @param lexer Lexer instance
 * @return Current position
 */
size_t lexer_get_position(Lexer *lexer);

/**
 * Get remaining input length
 * @param lexer Lexer instance
 * @return Remaining characters
 */
size_t lexer_remaining_length(Lexer *lexer);

#endif // LEXER_H