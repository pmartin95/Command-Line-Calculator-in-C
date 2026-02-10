#include "rational.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void rational_init(Rational *r)
{
    mpz_init_set_ui(r->numerator, 0);
    mpz_init_set_ui(r->denominator, 1);
}

void rational_init_from_int(Rational *r, long num, long denom)
{
    mpz_init_set_si(r->numerator, num);
    mpz_init_set_si(r->denominator, denom);

    // Handle negative denominator
    if (denom < 0)
    {
        mpz_neg(r->numerator, r->numerator);
        mpz_neg(r->denominator, r->denominator);
    }

    rational_simplify(r);
}

void rational_init_from_mpz(Rational *r, const mpz_t num, const mpz_t denom)
{
    mpz_init_set(r->numerator, num);
    mpz_init_set(r->denominator, denom);

    // Handle negative denominator
    if (mpz_sgn(r->denominator) < 0)
    {
        mpz_neg(r->numerator, r->numerator);
        mpz_neg(r->denominator, r->denominator);
    }

    rational_simplify(r);
}

void rational_clear(Rational *r)
{
    mpz_clear(r->numerator);
    mpz_clear(r->denominator);
}

void rational_set_from_int(Rational *r, long num, long denom)
{
    mpz_set_si(r->numerator, num);
    mpz_set_si(r->denominator, denom);

    // Handle negative denominator
    if (denom < 0)
    {
        mpz_neg(r->numerator, r->numerator);
        mpz_neg(r->denominator, r->denominator);
    }

    rational_simplify(r);
}

void rational_set(Rational *dest, const Rational *src)
{
    mpz_set(dest->numerator, src->numerator);
    mpz_set(dest->denominator, src->denominator);
}

void rational_add(Rational *result, const Rational *a, const Rational *b)
{
    // result = a + b = (a.num * b.denom + b.num * a.denom) / (a.denom * b.denom)
    mpz_t temp1, temp2, num, denom;
    mpz_init(temp1);
    mpz_init(temp2);
    mpz_init(num);
    mpz_init(denom);

    // temp1 = a.num * b.denom
    mpz_mul(temp1, a->numerator, b->denominator);

    // temp2 = b.num * a.denom
    mpz_mul(temp2, b->numerator, a->denominator);

    // num = temp1 + temp2
    mpz_add(num, temp1, temp2);

    // denom = a.denom * b.denom
    mpz_mul(denom, a->denominator, b->denominator);

    // Set result
    mpz_set(result->numerator, num);
    mpz_set(result->denominator, denom);

    rational_simplify(result);

    mpz_clear(temp1);
    mpz_clear(temp2);
    mpz_clear(num);
    mpz_clear(denom);
}

void rational_sub(Rational *result, const Rational *a, const Rational *b)
{
    // result = a - b = (a.num * b.denom - b.num * a.denom) / (a.denom * b.denom)
    mpz_t temp1, temp2, num, denom;
    mpz_init(temp1);
    mpz_init(temp2);
    mpz_init(num);
    mpz_init(denom);

    // temp1 = a.num * b.denom
    mpz_mul(temp1, a->numerator, b->denominator);

    // temp2 = b.num * a.denom
    mpz_mul(temp2, b->numerator, a->denominator);

    // num = temp1 - temp2
    mpz_sub(num, temp1, temp2);

    // denom = a.denom * b.denom
    mpz_mul(denom, a->denominator, b->denominator);

    // Set result
    mpz_set(result->numerator, num);
    mpz_set(result->denominator, denom);

    rational_simplify(result);

    mpz_clear(temp1);
    mpz_clear(temp2);
    mpz_clear(num);
    mpz_clear(denom);
}

void rational_mul(Rational *result, const Rational *a, const Rational *b)
{
    // result = a * b = (a.num * b.num) / (a.denom * b.denom)
    mpz_t num, denom;
    mpz_init(num);
    mpz_init(denom);

    mpz_mul(num, a->numerator, b->numerator);
    mpz_mul(denom, a->denominator, b->denominator);

    mpz_set(result->numerator, num);
    mpz_set(result->denominator, denom);

    rational_simplify(result);

    mpz_clear(num);
    mpz_clear(denom);
}

void rational_div(Rational *result, const Rational *a, const Rational *b)
{
    // result = a / b = (a.num * b.denom) / (a.denom * b.num)
    mpz_t num, denom;
    mpz_init(num);
    mpz_init(denom);

    mpz_mul(num, a->numerator, b->denominator);
    mpz_mul(denom, a->denominator, b->numerator);

    // Handle negative denominator
    if (mpz_sgn(denom) < 0)
    {
        mpz_neg(num, num);
        mpz_neg(denom, denom);
    }

    mpz_set(result->numerator, num);
    mpz_set(result->denominator, denom);

    rational_simplify(result);

    mpz_clear(num);
    mpz_clear(denom);
}

void rational_neg(Rational *result, const Rational *r)
{
    mpz_neg(result->numerator, r->numerator);
    mpz_set(result->denominator, r->denominator);
}

void rational_simplify(Rational *r)
{
    // Find GCD of numerator and denominator
    mpz_t gcd;
    mpz_init(gcd);

    mpz_gcd(gcd, r->numerator, r->denominator);

    // Divide both by GCD
    mpz_divexact(r->numerator, r->numerator, gcd);
    mpz_divexact(r->denominator, r->denominator, gcd);

    // Ensure denominator is positive
    if (mpz_sgn(r->denominator) < 0)
    {
        mpz_neg(r->numerator, r->numerator);
        mpz_neg(r->denominator, r->denominator);
    }

    mpz_clear(gcd);
}

int rational_is_integer(const Rational *r)
{
    return mpz_cmp_ui(r->denominator, 1) == 0;
}

int rational_is_zero(const Rational *r)
{
    return mpz_sgn(r->numerator) == 0;
}

int rational_is_one(const Rational *r)
{
    return mpz_cmp_ui(r->numerator, 1) == 0 &&
           mpz_cmp_ui(r->denominator, 1) == 0;
}

int rational_cmp(const Rational *a, const Rational *b)
{
    // Compare a/b with c/d by comparing a*d with b*c
    mpz_t temp1, temp2;
    mpz_init(temp1);
    mpz_init(temp2);

    mpz_mul(temp1, a->numerator, b->denominator);
    mpz_mul(temp2, b->numerator, a->denominator);

    int result = mpz_cmp(temp1, temp2);

    mpz_clear(temp1);
    mpz_clear(temp2);

    return result;
}

void rational_to_mpfr(mpfr_t result, const Rational *r)
{
    mpfr_t num, denom;
    mpfr_init2(num, mpfr_get_prec(result));
    mpfr_init2(denom, mpfr_get_prec(result));

    mpfr_set_z(num, r->numerator, MPFR_RNDN);
    mpfr_set_z(denom, r->denominator, MPFR_RNDN);

    mpfr_div(result, num, denom, MPFR_RNDN);

    mpfr_clear(num);
    mpfr_clear(denom);
}

char *rational_to_string(const Rational *r)
{
    // Get string representations of numerator and denominator
    char *num_str = mpz_get_str(NULL, 10, r->numerator);
    char *denom_str = mpz_get_str(NULL, 10, r->denominator);

    // Allocate result string
    size_t len = strlen(num_str) + strlen(denom_str) + 2; // +2 for '/' and '\0'
    char *result = malloc(len);

    if (rational_is_integer(r))
    {
        // Just return numerator if denominator is 1
        strcpy(result, num_str);
    }
    else
    {
        // Format as "num/denom"
        sprintf(result, "%s/%s", num_str, denom_str);
    }

    // Free GMP-allocated strings
    free(num_str);
    free(denom_str);

    return result;
}
