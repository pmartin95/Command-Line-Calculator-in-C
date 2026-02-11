#ifndef CALC_LIMITS_H
#define CALC_LIMITS_H

// Extra precision bits added to intermediate calculations to minimize rounding errors.
// Used by the evaluator (binop, unary, function args) and constant computation.
#define PRECISION_BOOST 128

// Maximum length of input expressions (bytes). Prevents excessive memory use.
#define MAX_INPUT_LENGTH 1024

// Maximum parser recursion depth. Prevents stack overflow on deeply nested expressions.
#define MAX_RECURSION_DEPTH 100

// Maximum number of arguments a function call can accept.
#define MAX_FUNCTION_ARGS 16

// Size of static error message buffers throughout the codebase.
#define ERROR_BUFFER_SIZE 256

#endif // CALC_LIMITS_H
