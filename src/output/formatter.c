#include "formatter.h"
#include "precision.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for static functions
static void formatter_print_scientific(const mpfr_t value);
static void formatter_print_fixed(const mpfr_t value);
static void formatter_print_smart_impl(const mpfr_t value);

// Formatting configuration
static int max_decimal_places = -1; // -1 means auto
static double small_threshold = 1e-6;
static double large_threshold = 1e15;

void formatter_print_smart(const mpfr_t value)
{
    formatter_print_number(value, FORMAT_SMART);
}

void formatter_print_result(const mpfr_t value, int original_is_int)
{
    // Check if the number is an integer and not too large
    if (original_is_int && mpfr_integer_p(value))
    {
        // Try to print as integer if it fits in long long
        if (mpfr_fits_slong_p(value, global_rounding))
        {
            long int_val = mpfr_get_si(value, global_rounding);
            printf("= %ld\n", int_val);
            return;
        }
    }

    printf("= ");
    formatter_print_smart(value);
    printf("\n");
}

void formatter_print_number(const mpfr_t value, NumberFormat format)
{
    if (mpfr_zero_p(value))
    {
        printf("0");
        return;
    }

    // Get absolute value for threshold checks
    mpfr_t abs_val;
    mpfr_init2(abs_val, global_precision);
    mpfr_abs(abs_val, value, global_rounding);

    NumberFormat chosen_format = format;

    // Auto-select format if needed
    if (format == FORMAT_AUTO)
    {
        if (!mpfr_zero_p(value) &&
            (mpfr_cmp_d(abs_val, small_threshold) < 0 ||
             mpfr_cmp_d(abs_val, large_threshold) > 0))
        {
            chosen_format = FORMAT_SCIENTIFIC;
        }
        else
        {
            chosen_format = FORMAT_SMART;
        }
    }

    switch (chosen_format)
    {
    case FORMAT_SCIENTIFIC:
        formatter_print_scientific(value);
        break;
    case FORMAT_FIXED:
        formatter_print_fixed(value);
        break;
    case FORMAT_SMART:
    default:
        formatter_print_smart_impl(value);
        break;
    }

    mpfr_clear(abs_val);
}

static void formatter_print_scientific(const mpfr_t value)
{
    long decimal_digits = get_decimal_digits();
    if (max_decimal_places > 0 && max_decimal_places < decimal_digits)
        decimal_digits = max_decimal_places;

    char *str = NULL;
    mpfr_exp_t exp;

    str = mpfr_get_str(NULL, &exp, 10, decimal_digits, value, global_rounding);
    if (str)
    {
        // Remove leading minus for separate handling
        int is_negative = (str[0] == '-');
        char *digits = is_negative ? str + 1 : str;

        if (strlen(digits) > 0)
        {
            long exponent = (long)(exp - 1);
            
            // For small exponents (-3 to 3), consider using normal notation
            if (exponent >= -3 && exponent <= 3)
            {
                // Fall back to smart formatting for readability
                formatter_print_smart_impl(value);
                mpfr_free_str(str);
                return;
            }
            
            // Find last significant digit (remove trailing zeros)
            size_t len = strlen(digits);
            size_t last_significant = len - 1;
            
            while (last_significant > 0 && digits[last_significant] == '0')
            {
                last_significant--;
            }
            
            // Print the result
            printf("%s%c", is_negative ? "-" : "", digits[0]);
            
            if (last_significant > 0)
            {
                printf(".");
                for (size_t i = 1; i <= last_significant; i++)
                {
                    printf("%c", digits[i]);
                }
            }
            
            printf("e%ld", exponent);
        }
        mpfr_free_str(str);
    }
}

static void formatter_print_fixed(const mpfr_t value)
{
    long decimal_digits = get_decimal_digits();
    if (max_decimal_places > 0 && max_decimal_places < decimal_digits)
    {
        decimal_digits = max_decimal_places;
    }

    mpfr_printf("%.*Rf", (int)decimal_digits, value);
}

static void formatter_print_smart_impl(const mpfr_t value)
{
    long decimal_digits = get_decimal_digits();
    if (max_decimal_places > 0 && max_decimal_places < decimal_digits)
        decimal_digits = max_decimal_places;
    const long MAX_ZERO_RUN = 500;
    mpfr_exp_t exp;
    char *str = mpfr_get_str(NULL, &exp, 10, decimal_digits, value, global_rounding);

    if (!str)
    {
        printf("[error formatting number]");
        return;
    }

    // If exponent is too big/small, bail to scientific
    if (exp > MAX_ZERO_RUN || exp < -MAX_ZERO_RUN)
    {
        mpfr_free_str(str);
        formatter_print_scientific(value);
        return;
    }
    int is_negative = (str[0] == '-');
    char *digits = is_negative ? str + 1 : str;

    size_t len = strlen(digits);
    size_t last_significant = len - 1;
    while (last_significant > 0 && digits[last_significant] == '0')
    {
        last_significant--;
    }

    printf("%s", is_negative ? "-" : "");

    if (exp <= 0)
    {
        printf("0.");
        for (mpfr_exp_t i = 0; i < -exp; i++)
        {
            printf("0");
        }
        for (size_t i = 0; i <= last_significant; i++)
        {
            printf("%c", digits[i]);
        }
    }
    else if ((size_t)exp >= last_significant + 1)
    {
        // All digits are in the integer part
        for (size_t i = 0; i <= last_significant; i++)
        {
            printf("%c", digits[i]);
        }
        for (mpfr_exp_t i = last_significant + 1; i < (mpfr_exp_t)exp; i++)
        {
            printf("0");
        }
    }
    else
    {
        // Mixed integer and fractional
        for (mpfr_exp_t i = 0; i < exp; i++)
        {
            printf("%c", digits[i]);
        }
        printf(".");
        for (size_t i = exp; i <= last_significant; i++)
        {
            printf("%c", digits[i]);
        }
    }

    mpfr_free_str(str);
}

char *formatter_to_string(const mpfr_t value, NumberFormat format)
{
    // Implementation would capture printf output to string
    // For now, return NULL as placeholder
    (void)value;
    (void)format;
    return NULL;
}

void formatter_set_max_decimals(int max_decimals)
{
    max_decimal_places = max_decimals;
}

void formatter_set_scientific_thresholds(double small, double large)
{
    small_threshold = small;
    large_threshold = large;
}

// Add static variable for default mode
static NumberFormat default_mode = FORMAT_SMART;

void formatter_set_default_mode(NumberFormat format)
{
    default_mode = format;
}

NumberFormat formatter_get_default_mode(void)
{
    return default_mode;
}

void formatter_print_current_mode(void)
{
    const char *mode_name;
    switch (default_mode)
    {
    case FORMAT_SCIENTIFIC:
        mode_name = "scientific";
        break;
    case FORMAT_FIXED:
        mode_name = "fixed";
        break;
    case FORMAT_SMART:
        mode_name = "normal (smart)";
        break;
    case FORMAT_AUTO:
        mode_name = "auto";
        break;
    default:
        mode_name = "unknown";
        break;
    }

    printf("Current display mode: %s\n", mode_name);

    if (default_mode == FORMAT_SCIENTIFIC)
    {
        printf("All results will be displayed in scientific notation (e.g., 1.23e+05)\n");
    }
    else
    {
        printf("Results will be displayed in normal notation when appropriate\n");
    }
}

void formatter_print_result_with_mode(const mpfr_t value, int original_is_int)
{
    // For integers in normal mode, still try to show as integer if reasonable
    if (default_mode == FORMAT_SMART && original_is_int && mpfr_integer_p(value))
    {
        if (mpfr_fits_slong_p(value, global_rounding))
        {
            long int_val = mpfr_get_si(value, global_rounding);
            printf("= %ld\n", int_val);
            return;
        }
    }

    formatter_print_number(value, default_mode);
    printf("\n");
}