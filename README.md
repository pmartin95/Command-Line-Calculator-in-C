# Build Requirements
- [GNU MPFR Library](https://www.mpfr.org/)
- [GNU Readline Library](https://tiswww.case.edu/php/chet/readline/rltop.html)

---

# Adding Custom Constants

The calculator uses a metadata-driven architecture that makes adding new mathematical constants simple. You only need to update **3 files** to add a constant.

## Quick Start: Adding a New Constant

Let's say you want to add the golden ratio (φ ≈ 1.618) as a constant.

### Step 1: Add to the Function Table (`src/lexer/function_table.c`)

Add your constant name(s) to the `function_table` array so the lexer can recognize it:

```c
// Mathematical constants (all use TOKEN_CONSTANT now)
{"pi", TOKEN_CONSTANT, -1},
{"PI", TOKEN_CONSTANT, -1},
{"e", TOKEN_CONSTANT, -1},
{"E", TOKEN_CONSTANT, -1},
// ... other constants ...
{"phi", TOKEN_CONSTANT, -1},    // <- Add lowercase
{"PHI", TOKEN_CONSTANT, -1},    // <- Add uppercase (optional)
```

**That's the only file outside the constants system you need to touch!**

### Step 2: Add to the Enum (`src/core/constants.h`)

Add your constant to the `ConstantType` enum before `CONST_COUNT`:

```c
typedef enum
{
    CONST_PI,
    CONST_E,
    CONST_LN2,
    CONST_LN10,
    CONST_GAMMA,
    CONST_SQRT2,
    CONST_PHI,       // <- Add your new constant here
    CONST_COUNT      // Keep this last!
} ConstantType;
```

### Step 3: Implement in Constants Module (`src/core/constants.c`)

Add two things in `constants.c`:

**A) Write the compute function:**

```c
static void compute_phi(mpfr_t result, mpfr_prec_t prec, mpfr_rnd_t rnd)
{
    // Golden ratio: φ = (1 + sqrt(5)) / 2
    mpfr_t sqrt5;
    mpfr_init2(sqrt5, prec);
    mpfr_sqrt_ui(sqrt5, 5, rnd);
    mpfr_add_ui(result, sqrt5, 1, rnd);
    mpfr_div_ui(result, result, 2, rnd);
    mpfr_clear(sqrt5);
}
```

**B) Add to the metadata table:**

```c
static const ConstantMetadata constant_metadata[CONST_COUNT] = {
    [CONST_PI]    = {"pi",    compute_pi},
    [CONST_E]     = {"e",     compute_e},
    [CONST_LN2]   = {"ln2",   compute_ln2},
    [CONST_LN10]  = {"ln10",  compute_ln10},
    [CONST_GAMMA] = {"gamma", compute_gamma},
    [CONST_SQRT2] = {"sqrt2", compute_sqrt2},
    [CONST_PHI]   = {"phi",   compute_phi},  // <- Add here
};
```

**Note:** The string name in the metadata table should match the lowercase version you added to the function table. The lookup is case-insensitive, so "PHI" will automatically match "phi".

That's it! Now you can rebuild and use your constant:

```bash
make clean && make
./bin/calculator
> phi
= 1.6180339887498948482045868343656381177203091798057628621354486227052604628189
> PHI * 2
= 3.2360679774997896964091736687312762354406183596115257242708972454105209256379
> phi^2 - phi - 1
= 0
```

## What You Get Automatically

Once you've added a constant following these three steps, the following features work automatically without any additional code:

- ✅ **Lexer Recognition** - The lexer automatically recognizes your constant by name
- ✅ **Parser Integration** - The parser automatically creates constant AST nodes
- ✅ **Evaluator Support** - The evaluator automatically looks up and computes your constant
- ✅ **Case Insensitivity** - Both "phi" and "PHI" work automatically
- ✅ **Caching** - Constants are computed once per precision level and cached
- ✅ **Cache Management** - Automatically cleared when precision changes
- ✅ **String Lookup API** - `constants_get_by_name(result, "phi")` works automatically
- ✅ **Enum Lookup API** - `constants_get_by_type(result, CONST_PHI)` works automatically
- ✅ **Initialization** - `constants_init()` handles your constant
- ✅ **Cleanup** - `constants_cleanup()` handles your constant

## Current Built-in Constants

The calculator currently includes these mathematical constants:

- `pi` - π (3.14159...)
- `e` - Euler's number (2.71828...)
- `ln2` - Natural logarithm of 2 (0.69314...)
- `ln10` - Natural logarithm of 10 (2.30258...)
- `gamma` - Euler-Mascheroni constant γ (0.57721...)
- `sqrt2` - √2 (1.41421...)

All constants support both lowercase and uppercase (e.g., `pi` or `PI`).

## Examples of Constants You Could Add

- **Golden ratio**: `φ = (1 + √5) / 2`
- **More square roots**: `√3`, `√5`, `√7`
- **Catalan's constant**: `G ≈ 0.915965594`
- **Apéry's constant**: `ζ(3) ≈ 1.202056903`
- **Physical constants**: Speed of light, Planck's constant, Boltzmann constant, etc.
- **Conversion factors**: Degrees to radians (`pi/180`), etc.

## Why This Design Works

The calculator uses a unified constant token system (TOKEN_CONSTANT) that carries the constant name as a string through the parsing pipeline. This allows all parsing, evaluation, and display code to automatically handle any constant you add to the three files above, without requiring changes to the lexer, parser, AST, or evaluator infrastructure.

---

# Project Roadmap: High-Precision Command-Line Calculator

This document outlines current development tasks, bug fixes, refactors, and enhancements across all major subsystems of the calculator. It is intended as a long-term roadmap and a guide for contributors or future refactors.

---

## Core System

### Constants System (`constants.c`, `constants.h`)
- [x] Refactor `constants_get_pi()` to use `ensure_constant_precision()` for consistency.
- [x] Implement missing constant initializers: `mpfr_const_log2`, `mpfr_const_log10`, `mpfr_gamma`.
- [x] Add a helper function `clear_cached(CachedConstant *)` to reduce repetition.
- [x] Add constant support (`ln2`, `ln10`, `gamma`) to `evaluator_eval_constant()`.
- [x] Switch from string-based lookup to `enum`-based lookup for performance and safety.

### Evaluator System (`evaluator.c`, `evaluator.h`)
- [ ] Remove hardcoded 2-argument limit in `evaluator_eval_function()` by dynamically allocating `mpfr_t *args`.
- [ ] Add support for all constants in the `NODE_CONSTANT` case.
- [ ] Implement or replace the `evaluator_check_domain()` stub with real domain logic or a clear comment.
- [ ] Add a debug or trace mode for verbose evaluation diagnostics (optional).

### Function Engine (`functions.c`, `functions.h`)
- [ ] Create `#define CHECK_ARGS(n)` to simplify argument validation.
- [ ] Replace generic return values (`0/1`) with meaningful `enum FuncResult` codes.
- [ ] Include function name or token in `last_error` for better diagnostics.
- [ ] Remove or document `functions_cleanup()` if no persistent state is used.
- [ ] Consider function registration for extensibility (user-defined or plugin functions).

### Precision System (`precision.c`, `precision.h`)
- [ ] Add `set_rounding(mpfr_rnd_t)` to make rounding mode user-configurable.
- [ ] Replace hardcoded constants (e.g. `0.30103`) with named `#define` values.
- [ ] Log or warn when precision is clamped beyond allowed bounds.
- [ ] Clarify that `global_precision` affects only newly initialized variables.

---

## Lexical Analysis

### Lexer (`lexer.c`, `lexer.h`)
- [ ] Add line/column tracking to tokens.
- [ ] Add comment support (`//`, `#`).
- [ ] Improve lexer error reporting via `lexer_get_last_error()`.
- [ ] Define and use constants like `MAX_IDENTIFIER_LENGTH`.
- [ ] Validate edge-case identifiers (e.g., `_`, `__123`, `x_1_2`).

### Token System (`tokens.c`, `tokens.h`)
- [ ] Ensure `token_free()` correctly handles all allocations.
- [ ] Add unit tests for token fallback logic.
- [ ] Optionally extend `token_type_str()` to include aliases or symbols.
- [ ] Include position metadata in token structs.

### Function Table (`function_table.c`, `function_table.h`)
- [ ] Optimize name/ID lookup with a reverse map or hash.
- [ ] Support alias management with a “preferred name” field (optional).
- [ ] Optionally include token position for error context.

---

## Output System

### Formatter (`formatter.c`)
- [ ] Fully implement `formatter_to_string()` for non-stdout output.
- [ ] Standardize rounding behavior across all formatter modes.
- [ ] Support digit grouping (e.g. `1_000_000`) for readability (optional).
- [ ] Improve float edge case handling (e.g. underflow, overflow, subnormals).

### Printer (`printer.c`)
- [ ] Add parentheses for unary operations where needed.
- [ ] Respect `debug_level` to control verbosity.
- [ ] Support colored token/AST output (optional).
- [ ] Optionally implement a JSON or tree serialization format.

### Cross-Cutting
- [ ] Expose formatter/debug settings via REPL or CLI flags.
- [ ] Add property-based tests with extreme float cases.

---

## Parser and AST

### AST and Node Management
- [ ] Add line/column metadata to all `ASTNode` types.
- [ ] Add `NodeType` → string functions for debugging.
- [ ] Implement `ast_clone()` for optimizer/transformation use.
- [ ] Validate argument counts in `ast_create_function()`.

### Parser Structure
- [ ] Consider refactoring `parser.c` into logical subfiles.
- [ ] Document and unify precedence/associativity handling.
- [ ] Track position metadata in `Parser` state for error reporting.

### Error Recovery
- [ ] Expand `parser_synchronize()` with additional recovery tokens.
- [ ] Use `parser_utils_get_context()` in all error output.
- [ ] Add central `parser_report_error()` for consistent messaging.

### Testing
- [ ] Add AST round-trip consistency tests.
- [ ] Create invalid syntax test suite.
- [ ] Add stress tests for deeply nested expressions.

### Memory Safety
- [ ] Audit AST construction and ownership patterns.
- [ ] Track allocation counts in debug mode to detect leaks.
- [ ] Implement “shallow” AST printing for large cases.

### Feature Ideas
- [ ] Support function overloads (`min(x)` vs `min(x, y)`).
- [ ] Add constant folding or AST simplification (e.g., `x * 1 → x`).
- [ ] Add implicit cast warnings or coercion detection.

---

## User Interface and REPL

### General Improvements
- [ ] Implement a `fgets` fallback if `readline` is unavailable.
- [ ] Enable persistent command history save/load.
- [ ] Allow user-defined command aliases (e.g. `q → quit`).
- [ ] Make REPL commands case-insensitive.

### Input Handling
- [ ] Add multiline input with continuation prompts.
- [ ] Allow cursor navigation/history via `readline`.

### REPL Engine
- [ ] Show command result values (currently only expression results shown).
- [ ] Highlight commands vs expressions using color/tags (optional).
- [ ] Support CLI execution: `./calc "2 + 2"` batch mode.
- [ ] Add `--help` and `--version` flags to CLI.

### Help and Feedback
- [ ] Improve unknown command handling with suggestions.
- [ ] Add contextual help for specific command arguments.
- [ ] Add stricter input validation for argument values.

### Stability and Testing
- [ ] Ensure memory for `Command.argument` is freed.
- [ ] Add logging/debug mode toggle in REPL.
- [ ] Gracefully handle interrupt signals (`Ctrl+C`, `SIGINT`).

### Optional Enhancements
- [ ] Color-coded output (e.g., red errors, green success).
- [ ] Pipe expression results to files or shell commands.
- [ ] Implement command macros or user-defined scripts.

---
