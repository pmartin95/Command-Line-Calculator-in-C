#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test suite function declarations
extern int run_lexer_tests(void);
extern int run_parser_tests(void);
extern int run_evaluator_tests(void);
extern int run_precision_tests(void);
extern int run_integration_tests(void);
extern int run_clear_cached_tests(void);

typedef struct
{
    const char *name;
    int (*test_function)(void);
} TestSuite;

static TestSuite test_suites[] = {
    {"lexer", run_lexer_tests},
    {"parser", run_parser_tests},
    {"evaluator", run_evaluator_tests},
    {"precision", run_precision_tests},
    {"integration", run_integration_tests},
    {"constants", run_clear_cached_tests},
    {NULL, NULL}};

void print_usage(const char *program_name)
{
    printf("Usage: %s [options] [test_suite...]\n", program_name);
    printf("\nOptions:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --verbose  Enable verbose output\n");
    printf("  -q, --quiet    Suppress non-essential output\n");
    printf("\nTest Suites:\n");
    for (int i = 0; test_suites[i].name != NULL; i++)
    {
        printf("  %-12s Run %s tests\n", test_suites[i].name, test_suites[i].name);
    }
    printf("\nIf no test suites are specified, all tests will be run.\n");
    printf("\nExamples:\n");
    printf("  %s                    # Run all tests\n", program_name);
    printf("  %s lexer parser       # Run only lexer and parser tests\n", program_name);
    printf("  %s -v integration     # Run integration tests with verbose output\n", program_name);
}

int run_test_suite(const char *name, int verbose, int quiet)
{
    for (int i = 0; test_suites[i].name != NULL; i++)
    {
        if (strcmp(test_suites[i].name, name) == 0)
        {
            if (!quiet)
            {
                printf("\n🚀 Starting %s test suite...\n", name);
            }

            int result = test_suites[i].test_function();

            if (!quiet)
            {
                if (result == 0)
                {
                    printf("✅ %s tests: PASSED\n", name);
                }
                else
                {
                    printf("❌ %s tests: FAILED\n", name);
                }
            }

            return result;
        }
    }

    printf("❌ Unknown test suite: %s\n", name);
    return 1;
}

int run_all_tests(int verbose, int quiet)
{
    int total_failures = 0;
    int total_suites = 0;

    if (!quiet)
    {
        printf("🧪 Running Complete Test Suite\n");
        printf("===============================\n");
    }

    for (int i = 0; test_suites[i].name != NULL; i++)
    {
        total_suites++;
        int result = run_test_suite(test_suites[i].name, verbose, quiet);
        if (result != 0)
        {
            total_failures++;
        }
    }

    if (!quiet)
    {
        printf("\n===============================\n");
        printf("📊 Test Summary\n");
        printf("===============================\n");
        printf("Total test suites: %d\n", total_suites);
        printf("Passed: %d\n", total_suites - total_failures);
        printf("Failed: %d\n", total_failures);

        if (total_failures == 0)
        {
            printf("\n🎉 All tests passed!\n");
        }
        else
        {
            printf("\n💥 %d test suite(s) failed\n", total_failures);
        }
    }

    return total_failures;
}

int main(int argc, char *argv[])
{
    int verbose = 0;
    int quiet = 0;
    int help = 0;
    char **test_names = NULL;
    int test_count = 0;

    // Parse command line arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            help = 1;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
        {
            verbose = 1;
        }
        else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0)
        {
            quiet = 1;
        }
        else if (argv[i][0] != '-')
        {
            // This is a test suite name
            if (test_names == NULL)
            {
                test_names = malloc(argc * sizeof(char *));
                if (!test_names)
                {
                    fprintf(stderr, "Memory allocation failed\n");
                    return 1;
                }
            }
            test_names[test_count++] = argv[i];
        }
        else
        {
            printf("Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (help)
    {
        print_usage(argv[0]);
        free(test_names);
        return 0;
    }

    // Validate verbose and quiet aren't both set
    if (verbose && quiet)
    {
        printf("Error: --verbose and --quiet cannot be used together\n");
        free(test_names);
        return 1;
    }

    int exit_code = 0;

    if (test_count == 0)
    {
        // Run all tests
        exit_code = run_all_tests(verbose, quiet);
    }
    else
    {
        // Run specific test suites
        int total_failures = 0;

        for (int i = 0; i < test_count; i++)
        {
            int result = run_test_suite(test_names[i], verbose, quiet);
            if (result != 0)
            {
                total_failures++;
            }
        }

        if (!quiet && test_count > 1)
        {
            printf("\n📊 Summary for selected test suites:\n");
            printf("Ran %d test suite(s)\n", test_count);
            printf("Failed: %d\n", total_failures);

            if (total_failures == 0)
            {
                printf("🎉 All selected tests passed!\n");
            }
            else
            {
                printf("💥 %d test suite(s) failed\n", total_failures);
            }
        }

        exit_code = total_failures;
    }

    free(test_names);
    return exit_code;
}