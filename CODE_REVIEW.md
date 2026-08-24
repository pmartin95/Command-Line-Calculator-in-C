# Code Review: Command-Line Calculator in C

This is a well-structured arbitrary-precision calculator with a clean pipeline architecture: **Lexer → Parser → AST → Evaluator/Symbolic → Output**. It supports both a CLI (with readline) and a Qt6 GUI, and uses MPFR/GMP for arbitrary precision. Here's my review:

---

## Strengths

- **Clean modular architecture** — clear separation between core, lexer, parser, output, UI, and GUI modules
- **Metadata-driven constant system** — adding a new constant requires minimal code changes, well-documented in the README
- **Arbitrary precision** — proper use of MPFR with configurable precision (2–8192 bits), intermediate precision boosts for accuracy
- **Dual evaluation modes** — numeric and symbolic paths through the same AST, clean branching in the REPL
- **Solid test suite** — ~2,500 lines of tests covering arithmetic, precision, parsing, rationals, and integration scenarios
- **Good C hygiene** — consistent use of `snprintf`, `malloc`/`free` pairing, and input length limits

---

## Critical Issues

1. ~~**Hard-coded 2-argument function limit** — `src/core/evaluator.c:179` uses `mpfr_t args[2]`. Any future 3+ arg function would silently overflow this array. Should be dynamically sized or at least validated against `func_node->function.arg_count`.~~ **FIXED** — Dynamically allocated based on `arg_count` with bounds checking (max 16 args).

2. ~~**Unimplemented `evaluator_check_domain()`** — `src/core/evaluator.c:222` is a stub returning 0. Domain errors (e.g., `sqrt(-1)`, `log(0)`) are caught individually in some functions but not systematically. The public API promises something it doesn't deliver.~~ **FIXED** — Implemented with recursive AST walk, division-by-zero detection, and delegation to `functions_check_domain()`.

3. **Non-thread-safe global state everywhere** — `global_precision` in `src/core/precision.c`, `cached_constants` in `src/core/constants.c`, `eval_mode` in `src/ui/repl.c`, and `last_error` buffers in evaluator/functions. This blocks any future concurrent use (including the GUI calling the engine from a worker thread).

---

## High Severity

4. **Memory leak paths in command parsing** — `cmd.argument` is allocated in `src/ui/commands.c` but not all code paths guarantee it's freed. The REPL does free it, but the contract is fragile.

5. **Prompt pointer aliasing** — `src/ui/repl.c:18` initializes `repl_prompt` to a string literal `"> "`, but later code may reassign it to dynamically allocated memory, creating confusion about ownership.

6. **Static 256-byte error buffers** — `src/core/evaluator.c:10` and `src/core/functions.c` use fixed `char last_error[256]`. Long function names or deeply nested expressions could cause silent truncation of error messages.

---

## Medium Severity

7. **Symbolic simplification is incomplete** — The infrastructure (clone, equals, canonical ordering) is in place, but many simplification rules referenced in `ROADMAP.md` are not yet implemented. This means `mode symbolic` may produce unsimplified or surprising output.

8. ~~**Magic numbers** — `128` appears as a precision boost in both `src/core/constants.c` and `src/core/evaluator.c` without a shared `#define`. Same for `1024` (max input), `100` (max recursion depth), etc.~~ **FIXED** — Consolidated into `include/calc_limits.h`.

9. **Parser error recovery is skeletal** — `parser_synchronize()` and `parser_panic()` exist but don't do meaningful recovery. Error messages lack position info (no line/column tracking in the lexer).

---

## Test Coverage Gaps

| Area | Status |
|------|--------|
| Core arithmetic & precision | Well covered |
| Rational arithmetic | Well covered |
| Constant caching | Covered |
| ~~**Symbolic evaluation**~~ | ~~**No tests**~~ **FIXED** — 12 test groups covering clone/equals, predicates, all simplification rules |
| **GUI** | **No tests** |
| **Error/edge cases** (NaN, Inf, deep nesting) | **Minimal** |
| ~~**Lexer**~~ | ~~**6 lines only** (`tests/test_lexer.c`)~~ **FIXED** — 11 test groups covering integers, floats, scientific notation, operators, comparisons, functions, constants, identifiers, whitespace, full expressions, edge cases |
| ~~**Memory leak detection**~~ | ~~**None** (no valgrind integration)~~ **FIXED** — ASan option added (`-DENABLE_ASAN=ON`). Found 445 bytes leaked across token string allocations (pre-existing). |

---

## Minor / Style

- ~~`test_lexer.c` is essentially empty — just 6 lines with no real assertions~~ **FIXED**
- No `ast_clone()` in the parser module — the symbolic module reimplements deep copy
- Unicode output (π, √, ×, ÷) in `src/output/printer.c` is nice but may break on terminals without UTF-8
- ~~The old `Makefile` is still present alongside `CMakeLists.txt` — could cause confusion about which build system to use~~ **FIXED** — Makefile removed, consolidated on CMake
- `.calculator_history` and `.vscode/` are untracked — consider adding them to `.gitignore`

---

## Recommendations (prioritized)

1. ~~**Fix the args array** in evaluator.c — validate or dynamically allocate based on `arg_count`~~ **DONE**
2. ~~**Add symbolic mode tests** — this is the biggest coverage gap for actively developed code~~ **DONE**
3. ~~**Extract magic numbers** into named constants in a shared header~~ **DONE**
4. ~~**Implement or remove `evaluator_check_domain()`** — a stub in a public API is misleading~~ **DONE**
5. ~~**Add valgrind or ASan** to the test pipeline to catch memory issues~~ **DONE**
6. ~~**Flesh out lexer tests** — the lexer is a critical component with almost no test coverage~~ **DONE**
7. ~~**Clean up the dual build systems** — pick CMake and remove the Makefile, or document both~~ **DONE**

---

## Summary

Overall this is a solid, well-architected project. The core numeric calculator works well and the codebase is clean enough to extend. The main areas needing attention are the incomplete symbolic mode, test coverage gaps, and the hard-coded limits that could cause subtle bugs as features grow.
