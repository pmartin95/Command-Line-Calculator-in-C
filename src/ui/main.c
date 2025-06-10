#include "repl.h"
#include "input.h"
#include "precision.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    // Parse command line arguments
    int show_help = 0;
    int set_precision_arg = 0;
    mpfr_prec_t initial_precision = DEFAULT_PRECISION;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            show_help = 1;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--precision") == 0) {
            if (i + 1 < argc) {
                long prec = strtol(argv[i + 1], NULL, 10);
                if (prec > 0) {
                    initial_precision = (mpfr_prec_t)prec;
                    set_precision_arg = 1;
                    i++; // Skip the next argument
                } else {
                    fprintf(stderr, "Invalid precision value: %s\n", argv[i + 1]);
                    return 1;
                }
            } else {
                fprintf(stderr, "Option %s requires an argument\n", argv[i]);
                return 1;
            }
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("High-Precision Calculator v1.0\n");
            printf("Built with MPFR for arbitrary precision arithmetic\n");
            printf("Supports functions, constants, and complex expressions\n");
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            show_help = 1;
            break;
        }
    }
    
    if (show_help) {
        printf("High-Precision Mathematical Calculator\n\n");
        printf("Usage: %s [options]\n\n", argv[0]);
        printf("Options:\n");
        printf("  -h, --help              Show this help message\n");
        printf("  -v, --version           Show version information\n");
        printf("  -p, --precision <bits>  Set initial precision (53-8192 bits)\n");
        printf("\nExamples:\n");
        printf("  %s                      # Start with default precision\n", argv[0]);
        printf("  %s -p 256              # Start with 256-bit precision\n", argv[0]);
        printf("  %s --precision 512     # Start with 512-bit precision\n", argv[0]);
        printf("\nSupported Features:\n");
        printf("  • Arbitrary precision arithmetic using MPFR\n");
        printf("  • Mathematical functions (sin, cos, tan, sqrt, log, etc.)\n");
        printf("  • Mathematical constants (pi, e)\n");
        printf("  • Implicit multiplication (2pi, 3(x+y), etc.)\n");
        printf("  • Scientific notation (1.5e10, 2.3e-5)\n");
        printf("  • Comparison operators (==, !=, <, >, <=, >=)\n");
        if (input_has_readline_support()) {
            printf("  • Command history and line editing (readline)\n");
        }
        printf("\nCommands:\n");
        printf("  help                    Show detailed help\n");
        printf("  precision               Show current precision\n");
        printf("  precision <bits>        Set precision\n");
        printf("  test                    Run precision tests\n");
        printf("  quit                    Exit calculator\n");
        return 0;
    }
    
    // Initialize the REPL system
    if (repl_init() != 0) {
        fprintf(stderr, "Failed to initialize calculator\n");
        return 1;
    }
    
    // Set initial precision if specified
    if (set_precision_arg) {
        set_precision(initial_precision);
    }
    
    // Run the main REPL loop
    int exit_code = repl_run();
    
    // Cleanup
    repl_cleanup();
    
    return exit_code;
}