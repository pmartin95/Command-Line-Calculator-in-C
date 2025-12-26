#include "../src/core/constants.h"
#include "../src/core/precision.h"
#include <stdio.h>
#include <mpfr.h>

/**
 * Test that the clear_cached() helper function is accessible
 * and works correctly with CachedConstant structures.
 */
int main(void)
{
    printf("Testing clear_cached() helper function...\n");

    // Initialize precision system
    precision_init();
    constants_init();

    // Create a test cached constant
    CachedConstant test_constant = {0};

    // Initialize it
    mpfr_init2(test_constant.value, 128);
    mpfr_set_d(test_constant.value, 3.14159, MPFR_RNDN);
    test_constant.precision = 128;
    test_constant.is_initialized = 1;

    printf("  ✓ Created and initialized test constant\n");

    // Verify it's initialized
    if (!test_constant.is_initialized) {
        printf("  ✗ FAILED: Constant should be initialized\n");
        return 1;
    }

    // Clear it using the public helper function
    clear_cached(&test_constant);
    printf("  ✓ Called clear_cached() on test constant\n");

    // Verify it's now cleared
    if (test_constant.is_initialized) {
        printf("  ✗ FAILED: Constant should be cleared\n");
        return 1;
    }

    printf("  ✓ Constant is_initialized flag correctly set to 0\n");

    // Test with NULL should not crash (defensive)
    // Note: This may cause undefined behavior depending on implementation
    // clear_cached(NULL);  // Uncomment if you want to test null safety

    printf("\n✅ All clear_cached() tests passed!\n");

    precision_cleanup();
    constants_cleanup();

    return 0;
}
