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
 */
int test_enum_based_api(void)
{
    printf("Testing enum-based constant API...\n");

    // Test that constants are not cached initially
    TEST_ASSERT(!constants_is_cached_by_type(CONST_PI), "Pi should not be cached initially");
    TEST_ASSERT(!constants_is_cached_by_type(CONST_E), "E should not be cached initially");

    // Compute pi
    mpfr_t result;
    mpfr_init2(result, global_precision);
    constants_get_pi(result);

    TEST_ASSERT(constants_is_cached_by_type(CONST_PI), "Pi should be cached after computation");
    TEST_ASSERT(!constants_is_cached_by_type(CONST_E), "E should still not be cached");

    // Verify string API still works and is consistent
    TEST_ASSERT(constants_is_cached("pi"), "String API should find cached pi");
    TEST_ASSERT(!constants_is_cached("e"), "String API should not find uncached e");

    mpfr_clear(result);
    constants_clear_cache();

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
