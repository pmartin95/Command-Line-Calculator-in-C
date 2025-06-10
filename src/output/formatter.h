#ifndef FORMATTER_H
#define FORMATTER_H

#include <mpfr.h>

typedef enum
{
    FORMAT_AUTO,       // Automatically choose best format
    FORMAT_FIXED,      // Fixed decimal notation
    FORMAT_SCIENTIFIC, // Scientific notation
    FORMAT_SMART       // Smart formatting with trailing zero removal
} NumberFormat;

/**
 * Format and print an MPFR number with smart formatting
 * @param value The number to format
 */
void formatter_print_smart(const mpfr_t value);

/**
 * Format and print an MPFR number with specified format
 * @param value The number to format
 * @param format Output format style
 */
void formatter_print_number(const mpfr_t value, NumberFormat format);

/**
 * Format and print a calculation result
 * @param value The result to format
 * @param original_is_int Whether the input was originally an integer
 */
void formatter_print_result(const mpfr_t value, int original_is_int);

/**
 * Get string representation of number (caller must free)
 * @param value The number to format
 * @param format Output format style
 * @return Allocated string or NULL on failure
 */
char *formatter_to_string(const mpfr_t value, NumberFormat format);

/**
 * Set maximum number of decimal places to show
 * @param max_decimals Maximum decimal places
 */
void formatter_set_max_decimals(int max_decimals);

/**
 * Set threshold for switching to scientific notation
 * @param small_threshold Numbers smaller than this use scientific notation
 * @param large_threshold Numbers larger than this use scientific notation
 */
void formatter_set_scientific_thresholds(double small_threshold, double large_threshold);

#endif // FORMATTER_H