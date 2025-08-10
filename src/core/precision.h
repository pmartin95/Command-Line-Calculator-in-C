#ifndef PRECISION_H
#define PRECISION_H

#include <mpfr.h>

#define DEFAULT_PRECISION 256 // bits of precision (roughly 77 decimal digits)
#define MIN_PRECISION 2      // double precision is 53
#define MAX_PRECISION 8192    // reasonable upper limit

// Global precision settings
extern mpfr_prec_t global_precision;
extern mpfr_rnd_t global_rounding;

/**
 * Initialize precision system with default values
 */
void precision_init(void);

/**
 * Set calculation precision
 * @param prec Precision in bits (clamped to valid range)
 */
void set_precision(mpfr_prec_t prec);

/**
 * Get current precision
 * @return Current precision in bits
 */
mpfr_prec_t get_precision(void);

/**
 * Get equivalent decimal digits for current precision
 * @return Approximate decimal digits
 */
long get_decimal_digits(void);

/**
 * Print current precision information
 */
void print_precision_info(void);

/**
 * Cleanup precision system
 */
void precision_cleanup(void);

#endif // PRECISION_H