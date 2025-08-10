#include "repl.h"
#include "input.h"
#include "commands.h"
#include "lexer.h"
#include "parser.h"
#include "evaluator.h"
#include "formatter.h"
#include "precision.h"
#include "constants.h"
#include "functions.h"
#include "function_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *repl_prompt = "> ";
static int repl_echo = 0;

int repl_init(void)
{
    // Initialize all subsystems
    precision_init();
    constants_init();
    functions_init();
    function_table_init();

    // Initialize input system
    if (input_init() != 0)
    {
        fprintf(stderr, "Failed to initialize input system\n");
        return -1;
    }

    return 0;
}

int repl_run(void)
{
    printf("High-Precision Mathematical Calculator\n");

    if (input_has_readline_support())
    {
        printf("Type 'quit' to exit, 'help' for examples\n");
        printf("Use Up/Down arrows to browse history\n");
    }
    else
    {
        printf("Type 'quit' to exit, 'help' for examples\n");
        printf("Note: Compile with -DHAVE_READLINE -lreadline for history support\n");
    }

    print_precision_info();
    printf("\n");

    while (1)
    {
        char *input = input_read_line(repl_prompt);
        if (!input)
        {
            // EOF or error
            printf("\n");
            break;
        }

        if (repl_echo)
        {
            printf("Input: %s\n", input);
        }

        ReplResult result = repl_process_line(input);
        free(input);

        switch (result)
        {
        case REPL_EXIT:
            return 0;
        case REPL_ERROR:
            return 1;
        case REPL_CONTINUE:
        default:
            continue;
        }
    }

    return 0;
}

ReplResult repl_process_line(const char *input)
{
    if (!input)
    {
        return REPL_CONTINUE;
    }

    if (strlen(input) == 0)
    {
        return REPL_CONTINUE;
    }

    input_add_to_history(input);

    if (commands_is_command(input))
    {
        Command cmd = commands_parse(input);
        int cmd_result = commands_execute(&cmd);

        if (cmd.argument)
        {
            free(cmd.argument);
        }

        if (cmd_result > 0)
            return REPL_EXIT;
        if (cmd_result < 0)
            return REPL_ERROR;
        return REPL_CONTINUE;
    }

    Lexer lexer;
    lexer_init(&lexer, input);

    if (lexer_remaining_length(&lexer) == 0 && strlen(input) > 0)
    {
        printf("Input too long or invalid\n");
        return REPL_CONTINUE;
    }

    Parser parser;
    parser_init(&parser, &lexer);
    ASTNode *ast = parser_parse_expression(&parser);

    if (!ast || parser_has_error(&parser))
    {
        printf("Parse error\n");
        ast_free(ast);
        return REPL_CONTINUE;
    }

    if (parser.current_token.type == TOKEN_INVALID)
    {
        printf("Invalid token encountered\n");
        ast_free(ast);
        return REPL_CONTINUE;
    }

    if (parser.current_token.type != TOKEN_EOF)
    {
        printf("Unexpected token at end: %s\n",
               token_type_str(parser.current_token.type));
        ast_free(ast);
        return REPL_CONTINUE;
    }

    mpfr_t result;
    mpfr_init2(result, global_precision);
    evaluator_eval(result, ast);

    const char *eval_error = evaluator_get_last_error();
    if (eval_error)
    {
        printf("Evaluation error: %s\n", eval_error);
    }
    else
    {
        printf("= ");
        formatter_print_result_with_mode(result,
                                         ast->type == NODE_NUMBER && ast->number.is_int);
        printf("\n");
    }

    mpfr_clear(result);
    ast_free(ast);
    return REPL_CONTINUE;
}

void repl_set_prompt(const char *prompt)
{
    if (prompt)
    {
        repl_prompt = (char *)prompt;
    }
}

void repl_set_echo(int echo)
{
    repl_echo = echo;
}

void repl_add_history(const char *line)
{
    input_add_to_history(line);
}

void repl_cleanup(void)
{
    input_cleanup();
    constants_cleanup();
    functions_cleanup();
    precision_cleanup();
}
