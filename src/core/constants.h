#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <mpfr.h>

/**
 * Enumeration of all available constants
 */
typedef enum
{
    CONST_PI,
    CONST_E,
    CONST_LN2,
    CONST_LN10,
    CONST_GAMMA,
    CONST_SQRT2,
    CONST_COUNT  // Total number of constants (keep last)
} ConstantType;

/**
 * Cached constant with precision tracking
 */
typedef struct
{
    mpfr_t value;           // High-precision value
    mpfr_prec_t precision;  // Precision level for this cached value
    int is_initialized;     // Whether mpfr_t is initialized
} CachedConstant;

/**
 * Initialize constants system
 */
void constants_init(void);

/**
 * Get high-precision π
 * @param result Output variable for π
 */
void constants_get_pi(mpfr_t result);

/**
 * Get high-precision e
 * @param result Output variable for e
 */
void constants_get_e(mpfr_t result);

/**
 * Get high-precision natural log of 2
 * @param result Output variable for ln(2)
 */
void constants_get_ln2(mpfr_t result);

/**
 * Get high-precision natural log of 10
 * @param result Output variable for ln(10)
 */
void constants_get_ln10(mpfr_t result);

/**
 * Get Euler-Mascheroni constant γ
 * @param result Output variable for γ
 */
void constants_get_gamma(mpfr_t result);

/**
 * Get square root of 2
 * @param result Output variable for √2
 */
void constants_get_sqrt2(mpfr_t result);

/**
 * Check if a constant value has been computed for current precision (enum-based)
 * @param type Type of the constant
 * @return 1 if cached, 0 if needs recomputation
 */
int constants_is_cached_by_type(ConstantType type);

/**
 * Check if a constant value has been computed for current precision (string-based)
 * @param constant_name Name of the constant
 * @return 1 if cached, 0 if needs recomputation
 */
int constants_is_cached(const char *constant_name);

/**
 * Get a constant by name
 * @param result Output variable for the constant value
 * @param constant_name Name of the constant (e.g., "pi", "e", "sqrt2")
 * @return 1 if found and computed, 0 if unknown constant
 */
int constants_get_by_name(mpfr_t result, const char *constant_name);

/**
 * Clear a single cached constant
 * @param constant Pointer to the cached constant to clear
 */
void clear_cached(CachedConstant *constant);

/**
 * Clear cached constants (call when precision changes)
 */
void constants_clear_cache(void);

/**
 * Cleanup constants system
 */
void constants_cleanup(void);

#endif // CONSTANTS_H