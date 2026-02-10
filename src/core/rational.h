#ifndef RATIONAL_H
#define RATIONAL_H

#include <gmp.h>
#include <mpfr.h>

/**
 * Exact rational number representation using GMP integers
 * Automatically maintained in lowest terms
 */
typedef struct
{
    mpz_t numerator;   // Arbitrary precision integer numerator
    mpz_t denominator; // Arbitrary precision integer denominator (always > 0)
} Rational;

/**
 * Initialize a rational number to 0/1
 * @param r Rational to initialize
 */
void rational_init(Rational *r);

/**
 * Initialize a rational number from integer values
 * @param r Rational to initialize
 * @param num Numerator
 * @param denom Denominator (must not be zero)
 */
void rational_init_from_int(Rational *r, long num, long denom);

/**
 * Initialize a rational number from GMP integers
 * @param r Rational to initialize
 * @param num Numerator (mpz_t)
 * @param denom Denominator (mpz_t, must not be zero)
 */
void rational_init_from_mpz(Rational *r, const mpz_t num, const mpz_t denom);

/**
 * Clear/free a rational number's resources
 * @param r Rational to clear
 */
void rational_clear(Rational *r);

/**
 * Set a rational number from integer values
 * @param r Rational to set
 * @param num Numerator
 * @param denom Denominator (must not be zero)
 */
void rational_set_from_int(Rational *r, long num, long denom);

/**
 * Set a rational number from another rational
 * @param dest Destination rational
 * @param src Source rational
 */
void rational_set(Rational *dest, const Rational *src);

/**
 * Add two rational numbers
 * @param result Result (can be same as a or b)
 * @param a First operand
 * @param b Second operand
 */
void rational_add(Rational *result, const Rational *a, const Rational *b);

/**
 * Subtract two rational numbers (a - b)
 * @param result Result (can be same as a or b)
 * @param a First operand
 * @param b Second operand
 */
void rational_sub(Rational *result, const Rational *a, const Rational *b);

/**
 * Multiply two rational numbers
 * @param result Result (can be same as a or b)
 * @param a First operand
 * @param b Second operand
 */
void rational_mul(Rational *result, const Rational *a, const Rational *b);

/**
 * Divide two rational numbers (a / b)
 * @param result Result (can be same as a or b)
 * @param a First operand (dividend)
 * @param b Second operand (divisor, must not be zero)
 */
void rational_div(Rational *result, const Rational *a, const Rational *b);

/**
 * Negate a rational number
 * @param result Result (can be same as r)
 * @param r Rational to negate
 */
void rational_neg(Rational *result, const Rational *r);

/**
 * Reduce a rational to lowest terms
 * @param r Rational to simplify
 */
void rational_simplify(Rational *r);

/**
 * Check if a rational is an integer
 * @param r Rational to check
 * @return 1 if integer, 0 otherwise
 */
int rational_is_integer(const Rational *r);

/**
 * Check if a rational is zero
 * @param r Rational to check
 * @return 1 if zero, 0 otherwise
 */
int rational_is_zero(const Rational *r);

/**
 * Check if a rational is one
 * @param r Rational to check
 * @return 1 if one, 0 otherwise
 */
int rational_is_one(const Rational *r);

/**
 * Compare two rational numbers
 * @param a First rational
 * @param b Second rational
 * @return Negative if a < b, 0 if a == b, positive if a > b
 */
int rational_cmp(const Rational *a, const Rational *b);

/**
 * Convert rational to MPFR floating-point
 * @param result MPFR result (must be initialized)
 * @param r Rational to convert
 */
void rational_to_mpfr(mpfr_t result, const Rational *r);

/**
 * Get string representation of rational
 * @param r Rational to convert
 * @return Newly allocated string (caller must free)
 */
char *rational_to_string(const Rational *r);

#endif // RATIONAL_H
