#include "constants.h"
#include "precision.h"
#include <stdio.h>
#include <string.h>

// Cached constants with their precision
typedef struct
{
    mpfr_t value;
    mpfr_prec_t precision;
    int is_initialized;
} CachedConstant;

static CachedConstant cached_pi = {0};
static CachedConstant cached_e = {0};
static CachedConstant cached_ln2 = {0};
static CachedConstant cached_ln10 = {0};
static CachedConstant cached_gamma = {0};

void constants_init(void)
{
    // Initialize cache structures
    cached_pi.is_initialized = 0;
    cached_e.is_initialized = 0;
    cached_ln2.is_initialized = 0;
    cached_ln10.is_initialized = 0;
    cached_gamma.is_initialized = 0;
}

static void ensure_constant_precision(CachedConstant *constant)
{
    if (!constant->is_initialized || constant->precision != global_precision)
    {
        if (constant->is_initialized)
        {
            mpfr_clear(constant->value);
        }
        mpfr_init2(constant->value, global_precision); // <-- THIS NEEDS TO HAPPEN
        constant->is_initialized = 1;
    }
    constant->precision = global_precision; // <-- Ensure this is always synced
}

void constants_get_pi(mpfr_t result)
{
    if (!cached_pi.is_initialized || cached_pi.precision != global_precision)
    {
        if (cached_pi.is_initialized)
        {
            mpfr_clear(cached_pi.value);
        }

        mpfr_init2(cached_pi.value, global_precision);
        mpfr_const_pi(cached_pi.value, global_rounding);
        cached_pi.precision = global_precision;
        cached_pi.is_initialized = 1;
    }

    mpfr_set(result, cached_pi.value, global_rounding);
}


void constants_get_e(mpfr_t result)
{
    ensure_constant_precision(&cached_e);

    mpfr_t one;
    mpfr_init2(one, global_precision);
    mpfr_set_d(one, 1.0, global_rounding);
    mpfr_exp(cached_e.value, one, global_rounding);
    mpfr_clear(one);

    mpfr_set(result, cached_e.value, global_rounding);
}

void constants_get_ln2(mpfr_t result)
{
    ensure_constant_precision(&cached_ln2);

    // Compute ln(2) with current precision if not cached

    mpfr_set(result, cached_ln2.value, global_rounding);
}

void constants_get_ln10(mpfr_t result)
{
    ensure_constant_precision(&cached_ln10);

    mpfr_set(result, cached_ln10.value, global_rounding);
}

void constants_get_gamma(mpfr_t result)
{
    ensure_constant_precision(&cached_gamma);

    mpfr_set(result, cached_gamma.value, global_rounding);
}

int constants_is_cached(const char *constant_name)
{
    if (!constant_name)
        return 0;

    if (strcmp(constant_name, "pi") == 0)
    {
        return cached_pi.is_initialized && cached_pi.precision == global_precision;
    }
    else if (strcmp(constant_name, "e") == 0)
    {
        return cached_e.is_initialized && cached_e.precision == global_precision;
    }
    else if (strcmp(constant_name, "ln2") == 0)
    {
        return cached_ln2.is_initialized && cached_ln2.precision == global_precision;
    }
    else if (strcmp(constant_name, "ln10") == 0)
    {
        return cached_ln10.is_initialized && cached_ln10.precision == global_precision;
    }
    else if (strcmp(constant_name, "gamma") == 0)
    {
        return cached_gamma.is_initialized && cached_gamma.precision == global_precision;
    }

    return 0;
}

void constants_clear_cache(void)
{
    if (cached_pi.is_initialized)
    {
        mpfr_clear(cached_pi.value);
        cached_pi.is_initialized = 0;
    }
    if (cached_e.is_initialized)
    {
        mpfr_clear(cached_e.value);
        cached_e.is_initialized = 0;
    }
    if (cached_ln2.is_initialized)
    {
        mpfr_clear(cached_ln2.value);
        cached_ln2.is_initialized = 0;
    }
    if (cached_ln10.is_initialized)
    {
        mpfr_clear(cached_ln10.value);
        cached_ln10.is_initialized = 0;
    }
    if (cached_gamma.is_initialized)
    {
        mpfr_clear(cached_gamma.value);
        cached_gamma.is_initialized = 0;
    }
}

void constants_cleanup(void)
{
    constants_clear_cache();
}