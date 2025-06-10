#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <mpfr.h>

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
 * Check if a constant value has been computed for current precision
 * @param constant_name Name of the constant
 * @return 1 if cached, 0 if needs recomputation
 */
int constants_is_cached(const char *constant_name);

/**
 * Clear cached constants (call when precision changes)
 */
void constants_clear_cache(void);

/**
 * Cleanup constants system
 */
void constants_cleanup(void);

#endif // CONSTANTS_H