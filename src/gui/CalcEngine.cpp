#include "CalcEngine.h"

// mpfr.h/gmp.h already included via CalcEngine.h outside extern "C"
extern "C" {
#include "precision.h"
#include "constants.h"
#include "functions.h"
#include "function_table.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "evaluator.h"
#include "formatter.h"
#include "symbolic.h"
#include "printer.h"
}

CalcEngine::CalcEngine(QObject *parent)
    : QObject(parent)
    , m_hasError(false)
    , m_evalMode(Numeric)
{
    precision_init();
    constants_init();
    functions_init();
    function_table_init();
}

CalcEngine::~CalcEngine()
{
    constants_cleanup();
    functions_cleanup();
    precision_cleanup();
}

QString CalcEngine::evaluate(const QString &expression)
{
    m_hasError = false;
    m_lastError.clear();

    QByteArray utf8 = expression.trimmed().toUtf8();
    const char *input = utf8.constData();

    if (utf8.isEmpty()) {
        m_hasError = true;
        m_lastError = QStringLiteral("Empty expression");
        return {};
    }

    // Lexer
    Lexer lexer;
    lexer_init(&lexer, input);

    // Parser
    Parser parser;
    parser_init(&parser, &lexer);
    ASTNode *ast = parser_parse_expression(&parser);

    if (parser_has_error(&parser) || !ast) {
        const char *msg = parser_get_error_message(&parser);
        m_hasError = true;
        m_lastError = msg ? QString::fromUtf8(msg) : QStringLiteral("Parse error");
        if (ast) ast_free(ast);
        return {};
    }

    QString output;

    if (m_evalMode == Symbolic) {
        // Symbolic evaluation
        symbolic_clear_error();
        ASTNode *simplified = symbolic_eval(ast);

        const char *symError = symbolic_get_last_error();
        if (symError) {
            m_hasError = true;
            m_lastError = QString::fromUtf8(symError);
        } else {
            char *str = printer_ast_infix_to_string(simplified);
            if (str) {
                output = QString::fromUtf8(str);
                free(str);
            }
        }

        ast_free(simplified);
    } else {
        // Numeric evaluation
        mpfr_t result;
        mpfr_init2(result, get_precision());
        evaluator_clear_error();
        evaluator_eval(result, ast);

        const char *evalError = evaluator_get_last_error();
        if (evalError) {
            m_hasError = true;
            m_lastError = QString::fromUtf8(evalError);
        } else {
            int original_is_int = (ast->type == NODE_NUMBER && ast->number.is_int);
            char *formatted = formatter_result_to_string(result, original_is_int);
            if (formatted) {
                output = QString::fromUtf8(formatted);
                free(formatted);
            } else {
                char buf[256];
                mpfr_snprintf(buf, sizeof(buf), "%Re", result);
                output = QString::fromUtf8(buf);
            }
        }

        mpfr_clear(result);
    }

    m_lastResult = output;
    ast_free(ast);

    return output;
}

void CalcEngine::setPrecision(int bits)
{
    set_precision(static_cast<mpfr_prec_t>(bits));
    constants_clear_cache();
    emit precisionChanged(bits);
}

int CalcEngine::precision() const
{
    return static_cast<int>(get_precision());
}

int CalcEngine::decimalDigits() const
{
    return static_cast<int>(get_decimal_digits());
}

void CalcEngine::setFormat(NumberFormat format)
{
    formatter_set_default_mode(format);
    emit formatChanged(format);
}

NumberFormat CalcEngine::format() const
{
    return formatter_get_default_mode();
}

void CalcEngine::setEvalMode(EvalModeQt mode)
{
    m_evalMode = mode;
    emit evalModeChanged(mode);
}

CalcEngine::EvalModeQt CalcEngine::evalMode() const
{
    return m_evalMode;
}

bool CalcEngine::hasError() const
{
    return m_hasError;
}

QString CalcEngine::lastError() const
{
    return m_lastError;
}

QString CalcEngine::lastResult() const
{
    return m_lastResult;
}
