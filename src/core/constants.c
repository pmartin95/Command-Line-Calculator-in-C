#include "constants.h"
#include "precision.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

// Forward declarations for compute functions
static void compute_pi(mpfr_t result, mpfr_prec_t prec, mpfr_rnd_t rnd);
static void compute_e(mpfr_t result, mpfr_prec_t prec, mpfr_rnd_t rnd);
static void compute_ln2(mpfr_t result, mpfr_prec_t prec, mpfr_rnd_t rnd);
static void compute_ln10(mpfr_t result, mpfr_prec_t prec, mpfr_rnd_t rnd);
static void compute_gamma(mpfr_t result, mpfr_prec_t prec, mpfr_rnd_t rnd);
static void compute_sqrt2(mpfr_t result, mpfr_prec_t prec, mpfr_rnd_t rnd);

/**
 * Metadata for each constant
 */
typedef struct
{
    const char *name;
    void (*compute_fn)(mpfr_t result, mpfr_prec_t prec, mpfr_rnd_t rnd);
} ConstantMetadata;

// Single source of truth: metadata table
static const ConstantMetadata constant_metadata[CONST_COUNT] = {
    [CONST_PI]    = {"pi",    compute_pi},
    [CONST_E]     = {"e",     compute_e},
    [CONST_LN2]   = {"ln2",   compute_ln2},
    [CONST_LN10]  = {"ln10",  compute_ln10},
    [CONST_GAMMA] = {"gamma", compute_gamma},
    [CONST_SQRT2] = {"sqrt2", compute_sqrt2}
};

// Array-based storage indexed by ConstantType
static CachedConstant cached_constants[CONST_COUNT] = {0};

void constants_init(void)
{
    // Initialize all constants in the array
    for (int i = 0; i < CONST_COUNT; i++)
    {
        cached_constants[i].is_initialized = 0;
    }
}

static void ensure_constant_precision(CachedConstant *constant)
{
    if (!constant->is_initialized || constant->precision != global_precision)
    {
        if (constant->is_initialized)
        {
            mpfr_clear(constant->value);
        }
        mpfr_init2(constant->value, global_precision);
        constant->is_initialized = 1;
    }
    constant->precision = global_precision;
}

// Compute functions for each constant
static void compute_pi(mpfr_t result, mpfr_prec_t prec, mpfr_rnd_t rnd)
{
    (void)prec;  // Unused - precision is set via mpfr_t initialization
    mpfr_const_pi(result, rnd);
}

static void compute_e(mpfr_t result, mpfr_prec_t prec, mpfr_rnd_t rnd)
{
    mpfr_t one;
    mpfr_init2(one, prec);
    mpfr_set_d(one, 1.0, rnd);
    mpfr_exp(result, one, rnd);
    mpfr_clear(one);
}

static void compute_ln2(mpfr_t result, mpfr_prec_t prec, mpfr_rnd_t rnd)
{
    (void)prec;  // Unused - precision is set via mpfr_t initialization
    mpfr_const_log2(result, rnd);
}

static void compute_ln10(mpfr_t result, mpfr_prec_t prec, mpfr_rnd_t rnd)
{
    (void)prec;  // Unused - precision is set via mpfr_t initialization
    mpfr_set_ui(result, 10, rnd);
    mpfr_log(result, result, rnd);
}

static void compute_gamma(mpfr_t result, mpfr_prec_t prec, mpfr_rnd_t rnd)
{
    (void)prec;  // Unused - precision is set via mpfr_t initialization
    mpfr_const_euler(result, rnd);
}

static void compute_sqrt2(mpfr_t result, mpfr_prec_t prec, mpfr_rnd_t rnd)
{
    (void)prec;  // Unused - precision is set via mpfr_t initialization
    mpfr_sqrt_ui(result, 2, rnd);
}

// Extra precision bits to add for more accurate constant computation
#define CONSTANT_PRECISION_BOOST 128

// Generic getter function using enum
static void constants_get_by_type(mpfr_t result, ConstantType type)
{
    if (type < 0 || type >= CONST_COUNT)
    {
        return;
    }

    // Compute constant at higher precision for better accuracy
    mpfr_prec_t high_prec = global_precision + CONSTANT_PRECISION_BOOST;
    mpfr_t high_precision_value;
    mpfr_init2(high_precision_value, high_prec);

    // Compute the constant at high precision
    constant_metadata[type].compute_fn(high_precision_value, high_prec, global_rounding);

    // Round down to the user's requested precision
    mpfr_set(result, high_precision_value, global_rounding);

    mpfr_clear(high_precision_value);
}

// Convenience functions for specific constants
void constants_get_pi(mpfr_t result)
{
    constants_get_by_type(result, CONST_PI);
}

void constants_get_e(mpfr_t result)
{
    constants_get_by_type(result, CONST_E);
}

void constants_get_ln2(mpfr_t result)
{
    constants_get_by_type(result, CONST_LN2);
}

void constants_get_ln10(mpfr_t result)
{
    constants_get_by_type(result, CONST_LN10);
}

void constants_get_gamma(mpfr_t result)
{
    constants_get_by_type(result, CONST_GAMMA);
}

void constants_get_sqrt2(mpfr_t result)
{
    constants_get_by_type(result, CONST_SQRT2);
}

int constants_is_cached_by_type(ConstantType type)
{
    if (type < 0 || type >= CONST_COUNT)
    {
        return 0;
    }

    CachedConstant *constant = &cached_constants[type];
    return constant->is_initialized && constant->precision == global_precision;
}

int constants_is_cached(const char *constant_name)
{
    if (!constant_name)
    {
        return 0;
    }

    // Look up the constant name in the metadata table
    for (int i = 0; i < CONST_COUNT; i++)
    {
        if (strcmp(constant_metadata[i].name, constant_name) == 0)
        {
            return constants_is_cached_by_type(i);
        }
    }

    return 0;
}

int constants_get_by_name(mpfr_t result, const char *constant_name)
{
    if (!constant_name)
    {
        return 0;
    }

    // Look up the constant in the metadata table (case-insensitive)
    for (int i = 0; i < CONST_COUNT; i++)
    {
        if (strcasecmp(constant_metadata[i].name, constant_name) == 0)
        {
            // Found it! Use the internal getter
            constants_get_by_type(result, i);
            return 1;
        }
    }

    // Unknown constant
    return 0;
}

void clear_cached(CachedConstant *constant)
{
    if (constant->is_initialized)
    {
        mpfr_clear(constant->value);
        constant->is_initialized = 0;
    }
}

void constants_clear_cache(void)
{
    for (int i = 0; i < CONST_COUNT; i++)
    {
        clear_cached(&cached_constants[i]);
    }
}

void constants_cleanup(void)
{
    constants_clear_cache();
}