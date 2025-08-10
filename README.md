# Build Requirements
- [GNU MPFR Library](https://www.mpfr.org/)


# Project Roadmap: High-Precision Command-Line Calculator

This document outlines current development tasks, bug fixes, refactors, and enhancements across all major subsystems of the calculator. It is intended as a long-term roadmap and a guide for contributors or future refactors.

---

## Core System

### Constants System (`constants.c`, `constants.h`)
- [x] Refactor `constants_get_pi()` to use `ensure_constant_precision()` for consistency.
- [x] Implement missing constant initializers: `mpfr_const_log2`, `mpfr_const_log10`, `mpfr_gamma`.
- [ ] Add a helper function `clear_cached(CachedConstant *)` to reduce repetition.
- [x] Add constant support (`ln2`, `ln10`, `gamma`) to `evaluator_eval_constant()`.
- [ ] Consider switching from string-based lookup to `enum`-based lookup for performance and safety.

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
