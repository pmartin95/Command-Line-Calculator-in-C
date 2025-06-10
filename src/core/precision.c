#include "precision.h"
#include <stdio.h>

// Global precision settings
mpfr_prec_t global_precision = DEFAULT_PRECISION;
mpfr_rnd_t global_rounding = MPFR_RNDN; // Round to nearest

void precision_init(void)
{
    mpfr_set_default_prec(DEFAULT_PRECISION);
    global_precision = DEFAULT_PRECISION;
    global_rounding = MPFR_RNDN;
}

void set_precision(mpfr_prec_t prec)
{
    if (prec < MIN_PRECISION)
        prec = MIN_PRECISION;
    if (prec > MAX_PRECISION)
        prec = MAX_PRECISION;
    
    global_precision = prec;
    mpfr_set_default_prec(prec);
}

mpfr_prec_t get_precision(void)
{
    return global_precision;
}

long get_decimal_digits(void)
{
    return (long)(global_precision * 0.30103); // log10(2) ≈ 0.30103
}

void print_precision_info(void)
{
    printf("Current precision: %ld bits (approximately %ld decimal digits)\n",
           (long)global_precision, get_decimal_digits());
}

void precision_cleanup(void)
{
    mpfr_free_cache();
}