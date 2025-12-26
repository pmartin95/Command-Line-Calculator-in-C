#include "precision.h"
#include "constants.h"
#include <stdio.h>
#include <mpfr.h>

#define TEST_ASSERT(condition, message)         \
    do                                          \
    {                                           \
        if (!(condition))                       \
        {                                       \
            printf("  ❌ FAIL: %s\n", message); \
            return 0;                           \
        }                                       \
    } while (0)

/**
 * Test that the clear_cached() helper function is accessible
 * and works correctly with CachedConstant structures.
 */
int test_clear_cached_helper(void)
{
    printf("Testing clear_cached() helper function...\n");

    // Create a test cached constant
    CachedConstant test_constant = {0};

    // Initialize it
    mpfr_init2(test_constant.value, 128);
    mpfr_set_d(test_constant.value, 3.14159, MPFR_RNDN);
    test_constant.precision = 128;
    test_constant.is_initialized = 1;

    TEST_ASSERT(test_constant.is_initialized, "Constant should be initialized");

    // Clear it using the public helper function
    clear_cached(&test_constant);

    TEST_ASSERT(!test_constant.is_initialized, "Constant should be cleared after clear_cached()");

    printf("  ✅ clear_cached() helper tests passed\n");
    return 1;
}

/**
 * Test enum-based API for constants
 * NOTE: With high-precision constant computation, we no longer cache constants
 * at user precision. This test now verifies constants are computed correctly.
 */
int test_enum_based_api(void)
{
    printf("Testing enum-based constant API...\n");

    // Compute pi and e
    mpfr_t pi_result, e_result;
    mpfr_init2(pi_result, global_precision);
    mpfr_init2(e_result, global_precision);

    constants_get_pi(pi_result);
    constants_get_e(e_result);

    // Verify the constants are computed correctly (basic sanity checks)
    TEST_ASSERT(mpfr_cmp_d(pi_result, 3.0) > 0 && mpfr_cmp_d(pi_result, 3.2) < 0,
                "Pi should be approximately 3.14159...");
    TEST_ASSERT(mpfr_cmp_d(e_result, 2.7) > 0 && mpfr_cmp_d(e_result, 2.8) < 0,
                "E should be approximately 2.71828...");

    mpfr_clear(pi_result);
    mpfr_clear(e_result);

    printf("  ✅ Enum-based API tests passed\n");
    return 1;
}

int run_clear_cached_tests(void)
{
    printf("\n=== Running clear_cached Tests ===\n");

    precision_init();
    constants_init();

    int passed = 0;
    int total = 2;

    if (test_clear_cached_helper())
        passed++;
    if (test_enum_based_api())
        passed++;

    precision_cleanup();
    constants_cleanup();

    printf("\n%d/%d tests passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}
