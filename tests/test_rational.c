#include "rational.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void test_rational_creation(void)
{
    printf("Testing rational creation...\n");

    Rational r;
    rational_init(&r);
    assert(rational_is_zero(&r));
    assert(rational_is_integer(&r));
    rational_clear(&r);

    rational_init_from_int(&r, 6, 8);
    // Should be reduced to 3/4
    assert(mpz_cmp_ui(r.numerator, 3) == 0);
    assert(mpz_cmp_ui(r.denominator, 4) == 0);
    rational_clear(&r);

    // Test negative denominator normalization
    rational_init_from_int(&r, 1, -2);
    assert(mpz_cmp_si(r.numerator, -1) == 0);
    assert(mpz_cmp_ui(r.denominator, 2) == 0);
    rational_clear(&r);

    printf("  ✓ Rational creation tests passed\n");
}

void test_rational_simplification(void)
{
    printf("Testing rational simplification...\n");

    Rational r;
    rational_init_from_int(&r, 12, 16);
    // Should be 3/4
    assert(mpz_cmp_ui(r.numerator, 3) == 0);
    assert(mpz_cmp_ui(r.denominator, 4) == 0);
    rational_clear(&r);

    rational_init_from_int(&r, 100, 25);
    // Should be 4/1 = 4
    assert(mpz_cmp_ui(r.numerator, 4) == 0);
    assert(mpz_cmp_ui(r.denominator, 1) == 0);
    assert(rational_is_integer(&r));
    rational_clear(&r);

    printf("  ✓ Rational simplification tests passed\n");
}

void test_rational_addition(void)
{
    printf("Testing rational addition...\n");

    Rational a, b, result;
    rational_init_from_int(&a, 1, 2);    // 1/2
    rational_init_from_int(&b, 1, 3);    // 1/3
    rational_init(&result);

    rational_add(&result, &a, &b);
    // 1/2 + 1/3 = 5/6
    assert(mpz_cmp_ui(result.numerator, 5) == 0);
    assert(mpz_cmp_ui(result.denominator, 6) == 0);

    rational_clear(&a);
    rational_clear(&b);
    rational_clear(&result);

    printf("  ✓ Rational addition tests passed\n");
}

void test_rational_subtraction(void)
{
    printf("Testing rational subtraction...\n");

    Rational a, b, result;
    rational_init_from_int(&a, 3, 4);    // 3/4
    rational_init_from_int(&b, 1, 4);    // 1/4
    rational_init(&result);

    rational_sub(&result, &a, &b);
    // 3/4 - 1/4 = 2/4 = 1/2
    assert(mpz_cmp_ui(result.numerator, 1) == 0);
    assert(mpz_cmp_ui(result.denominator, 2) == 0);

    rational_clear(&a);
    rational_clear(&b);
    rational_clear(&result);

    printf("  ✓ Rational subtraction tests passed\n");
}

void test_rational_multiplication(void)
{
    printf("Testing rational multiplication...\n");

    Rational a, b, result;
    rational_init_from_int(&a, 2, 3);    // 2/3
    rational_init_from_int(&b, 3, 4);    // 3/4
    rational_init(&result);

    rational_mul(&result, &a, &b);
    // 2/3 * 3/4 = 6/12 = 1/2
    assert(mpz_cmp_ui(result.numerator, 1) == 0);
    assert(mpz_cmp_ui(result.denominator, 2) == 0);

    rational_clear(&a);
    rational_clear(&b);
    rational_clear(&result);

    printf("  ✓ Rational multiplication tests passed\n");
}

void test_rational_division(void)
{
    printf("Testing rational division...\n");

    Rational a, b, result;
    rational_init_from_int(&a, 1, 2);    // 1/2
    rational_init_from_int(&b, 1, 3);    // 1/3
    rational_init(&result);

    rational_div(&result, &a, &b);
    // (1/2) / (1/3) = (1/2) * (3/1) = 3/2
    assert(mpz_cmp_ui(result.numerator, 3) == 0);
    assert(mpz_cmp_ui(result.denominator, 2) == 0);

    rational_clear(&a);
    rational_clear(&b);
    rational_clear(&result);

    printf("  ✓ Rational division tests passed\n");
}

void test_rational_comparison(void)
{
    printf("Testing rational comparison...\n");

    Rational a, b;
    rational_init_from_int(&a, 1, 2);    // 1/2
    rational_init_from_int(&b, 1, 3);    // 1/3

    assert(rational_cmp(&a, &b) > 0);    // 1/2 > 1/3
    assert(rational_cmp(&b, &a) < 0);    // 1/3 < 1/2
    assert(rational_cmp(&a, &a) == 0);   // 1/2 == 1/2

    rational_clear(&a);
    rational_clear(&b);

    printf("  ✓ Rational comparison tests passed\n");
}

void test_rational_to_mpfr_conversion(void)
{
    printf("Testing rational to MPFR conversion...\n");

    Rational r;
    mpfr_t result;
    mpfr_init2(result, 256);

    rational_init_from_int(&r, 1, 2);    // 1/2
    rational_to_mpfr(result, &r);

    // Check that result is 0.5
    mpfr_t expected;
    mpfr_init2(expected, 256);
    mpfr_set_d(expected, 0.5, MPFR_RNDN);

    assert(mpfr_equal_p(result, expected));

    mpfr_clear(result);
    mpfr_clear(expected);
    rational_clear(&r);

    printf("  ✓ Rational to MPFR conversion tests passed\n");
}

void test_rational_predicates(void)
{
    printf("Testing rational predicates...\n");

    Rational r;

    rational_init_from_int(&r, 0, 1);
    assert(rational_is_zero(&r));
    assert(!rational_is_one(&r));
    assert(rational_is_integer(&r));
    rational_clear(&r);

    rational_init_from_int(&r, 1, 1);
    assert(!rational_is_zero(&r));
    assert(rational_is_one(&r));
    assert(rational_is_integer(&r));
    rational_clear(&r);

    rational_init_from_int(&r, 3, 4);
    assert(!rational_is_zero(&r));
    assert(!rational_is_one(&r));
    assert(!rational_is_integer(&r));
    rational_clear(&r);

    rational_init_from_int(&r, 5, 1);
    assert(!rational_is_zero(&r));
    assert(!rational_is_one(&r));
    assert(rational_is_integer(&r));
    rational_clear(&r);

    printf("  ✓ Rational predicate tests passed\n");
}

void test_rational_negation(void)
{
    printf("Testing rational negation...\n");

    Rational r, result;
    rational_init_from_int(&r, 3, 4);
    rational_init(&result);

    rational_neg(&result, &r);
    assert(mpz_cmp_si(result.numerator, -3) == 0);
    assert(mpz_cmp_ui(result.denominator, 4) == 0);

    rational_clear(&r);
    rational_clear(&result);

    printf("  ✓ Rational negation tests passed\n");
}

void test_rational_to_string(void)
{
    printf("Testing rational to string...\n");

    Rational r;
    char *str;

    rational_init_from_int(&r, 3, 4);
    str = rational_to_string(&r);
    assert(strcmp(str, "3/4") == 0);
    free(str);
    rational_clear(&r);

    rational_init_from_int(&r, 5, 1);
    str = rational_to_string(&r);
    assert(strcmp(str, "5") == 0);
    free(str);
    rational_clear(&r);

    printf("  ✓ Rational to string tests passed\n");
}

int main(void)
{
    printf("Running Rational Number Tests\n");
    printf("==============================\n\n");

    test_rational_creation();
    test_rational_simplification();
    test_rational_addition();
    test_rational_subtraction();
    test_rational_multiplication();
    test_rational_division();
    test_rational_comparison();
    test_rational_to_mpfr_conversion();
    test_rational_predicates();
    test_rational_negation();
    test_rational_to_string();

    printf("\n==============================\n");
    printf("All rational tests passed! ✓\n");

    return 0;
}
