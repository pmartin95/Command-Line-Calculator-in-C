# Symbolic Computation Mode - Implementation Roadmap

## Project Overview

This roadmap outlines the implementation of a symbolic computation mode for the calculator. This feature will allow expressions to be simplified algebraically rather than evaluated numerically, enabling operations like:

```
sin(0) + sin(pi) + 2 * sin(sqrt(2))/2 = sin(sqrt(2))
(sqrt(2) + 5)/sqrt(2) = 1 + 5*sqrt(2)/2
```

**Estimated Total Effort:** 15-25 days (3-4 weeks)
**Core Complexity:** Medium
**Integration Complexity:** Low (thanks to clean existing architecture)

---

## Progress Tracker

### Phase 1: Foundation (Week 1-2)
- [ ] Create evaluation mode infrastructure
- [ ] Implement rational number system
- [ ] Create symbolic evaluator skeleton
- [ ] Add mode-switching UI commands
- [ ] Set up basic testing framework

### Phase 2: Core Simplifications (Week 2-3)
- [ ] Implement arithmetic simplification rules
- [ ] Implement trigonometric simplification rules
- [ ] Implement radical/sqrt simplification rules
- [ ] Implement fraction rationalization
- [ ] Add pattern matching infrastructure

### Phase 3: Advanced Features (Week 3-4)
- [ ] Add expression canonicalization
- [ ] Implement like-term collection
- [ ] Add symbolic expression comparison
- [ ] Handle edge cases (division by zero, domain errors)
- [ ] Expand rule set for common cases

### Phase 4: Testing & Polish (Week 4)
- [ ] Comprehensive unit tests for all simplification rules
- [ ] Integration tests for mode switching
- [ ] Test user's example expressions
- [ ] Documentation updates
- [ ] Performance optimization

---

## Implementation Details

### New Files to Create

#### 1. `src/core/symbolic.c` & `src/core/symbolic.h`
**Purpose:** Main symbolic evaluation engine
**Estimated Size:** ~400 lines
**Complexity:** Medium

**Functionality:**
- `ASTNode* symbolic_eval(const ASTNode *node)` - Main entry point for symbolic evaluation
- Traverses AST without computing numeric values
- Applies simplification rules recursively
- Returns simplified AST (not a numeric result)
- Manages expression canonicalization
- Handles symbolic expression comparison

**Key Functions:**
```c
// Main symbolic evaluator
ASTNode* symbolic_eval(const ASTNode *node);

// Helper functions
ASTNode* symbolic_simplify_binop(TokenType op, ASTNode *left, ASTNode *right);
ASTNode* symbolic_simplify_function(TokenType func, ASTNode **args, int arg_count);
ASTNode* symbolic_simplify_unary(TokenType op, ASTNode *operand);

// Expression utilities
int symbolic_is_zero(const ASTNode *node);
int symbolic_is_one(const ASTNode *node);
int symbolic_is_integer(const ASTNode *node);
int symbolic_equals(const ASTNode *a, const ASTNode *b);
ASTNode* symbolic_clone(const ASTNode *node);
```

---

#### 2. `src/core/rational.c` & `src/core/rational.h`
**Purpose:** Exact rational number arithmetic
**Estimated Size:** ~300 lines
**Complexity:** Low-Medium

**Functionality:**
- Represents fractions as exact numerator/denominator pairs
- Uses GMP `mpz_t` for arbitrary precision integers
- Automatic reduction to lowest terms
- Arithmetic operations preserving exactness

**Data Structure:**
```c
typedef struct {
    mpz_t numerator;      // GMP arbitrary precision integer
    mpz_t denominator;    // GMP arbitrary precision integer
} Rational;
```

**Key Functions:**
```c
// Creation and destruction
void rational_init(Rational *r);
void rational_init_from_int(Rational *r, long num, long denom);
void rational_init_from_mpz(Rational *r, const mpz_t num, const mpz_t denom);
void rational_clear(Rational *r);

// Arithmetic operations
void rational_add(Rational *result, const Rational *a, const Rational *b);
void rational_sub(Rational *result, const Rational *a, const Rational *b);
void rational_mul(Rational *result, const Rational *a, const Rational *b);
void rational_div(Rational *result, const Rational *a, const Rational *b);

// Utilities
void rational_simplify(Rational *r);  // Reduce to lowest terms
int rational_is_integer(const Rational *r);
int rational_is_zero(const Rational *r);
int rational_is_one(const Rational *r);
int rational_cmp(const Rational *a, const Rational *b);
void rational_to_mpfr(mpfr_t result, const Rational *r);  // For numeric evaluation
```

---

#### 3. `src/core/simplify_rules.c` & `src/core/simplify_rules.h`
**Purpose:** Pattern matching and simplification rules
**Estimated Size:** ~600 lines
**Complexity:** Medium-High

**Functionality:**
- Rule-based pattern matching for algebraic simplification
- Trigonometric identity application
- Radical simplification
- Fraction rationalization

**Rule Categories:**

**A. Arithmetic Rules:**
```c
ASTNode* rule_add_zero(ASTNode *left, ASTNode *right);        // x + 0 → x
ASTNode* rule_mul_zero(ASTNode *left, ASTNode *right);        // x * 0 → 0
ASTNode* rule_mul_one(ASTNode *left, ASTNode *right);         // x * 1 → x
ASTNode* rule_div_one(ASTNode *left, ASTNode *right);         // x / 1 → x
ASTNode* rule_sub_self(ASTNode *left, ASTNode *right);        // x - x → 0
ASTNode* rule_div_self(ASTNode *left, ASTNode *right);        // x / x → 1
```

**B. Trigonometric Rules:**
```c
ASTNode* rule_sin_exact(ASTNode *arg);    // sin(0)→0, sin(π/2)→1, sin(π)→0, etc.
ASTNode* rule_cos_exact(ASTNode *arg);    // cos(0)→1, cos(π/2)→0, cos(π)→-1, etc.
ASTNode* rule_tan_exact(ASTNode *arg);    // tan(0)→0, tan(π/4)→1, etc.
```

**C. Radical Rules:**
```c
ASTNode* rule_sqrt_square(ASTNode *arg);         // sqrt(x^2) → |x|
ASTNode* rule_sqrt_product(ASTNode *arg);        // sqrt(a*b) → sqrt(a)*sqrt(b)
ASTNode* rule_sqrt_fraction(ASTNode *arg);       // sqrt(a/b) → sqrt(a)/sqrt(b)
ASTNode* rule_sqrt_simplify_radicand(ASTNode *arg); // sqrt(8) → 2*sqrt(2)
```

**D. Rationalization Rules:**
```c
ASTNode* rule_rationalize_denominator(ASTNode *num, ASTNode *denom);
// a/sqrt(b) → a*sqrt(b)/b
// (a+b)/(c+sqrt(d)) → (a+b)(c-sqrt(d))/(c²-d)  [conjugate multiplication]
```

**E. Power Rules:**
```c
ASTNode* rule_pow_zero(ASTNode *base, ASTNode *exp);   // x^0 → 1
ASTNode* rule_pow_one(ASTNode *base, ASTNode *exp);    // x^1 → x
ASTNode* rule_pow_of_pow(ASTNode *expr);                // (x^a)^b → x^(a*b)
```

**Key Functions:**
```c
// Main rule application
ASTNode* apply_all_rules(const ASTNode *node);

// Rule dispatcher by category
ASTNode* apply_arithmetic_rules(TokenType op, ASTNode *left, ASTNode *right);
ASTNode* apply_trig_rules(TokenType func, ASTNode **args, int arg_count);
ASTNode* apply_radical_rules(TokenType func, ASTNode **args, int arg_count);
ASTNode* apply_power_rules(ASTNode *base, ASTNode *exp);

// Pattern matching utilities
int matches_exact_value(const ASTNode *node, double value);
int matches_constant(const ASTNode *node, const char *const_name);
int is_sqrt_node(const ASTNode *node);
int is_power_node(const ASTNode *node);
```

---

#### 4. `tests/test_symbolic.c`
**Purpose:** Unit tests for symbolic evaluation
**Estimated Size:** ~200 lines
**Complexity:** Medium

**Test Coverage:**
```c
// Basic arithmetic simplification
void test_add_zero(void);
void test_mul_zero(void);
void test_mul_one(void);
void test_sub_self(void);

// Trigonometric simplification
void test_sin_exact_values(void);
void test_cos_exact_values(void);
void test_tan_exact_values(void);

// Radical simplification
void test_sqrt_simplification(void);
void test_rationalize_denominator(void);

// User's example expressions
void test_user_example_1(void);  // sin(0) + sin(pi) + 2*sin(sqrt(2))/2
void test_user_example_2(void);  // (sqrt(2) + 5)/sqrt(2)

// Edge cases
void test_division_by_zero_symbolic(void);
void test_complex_nested_expressions(void);
void test_mode_switching(void);
```

---

#### 5. `tests/test_rational.c`
**Purpose:** Unit tests for rational arithmetic
**Estimated Size:** ~150 lines
**Complexity:** Low

**Test Coverage:**
```c
void test_rational_creation(void);
void test_rational_simplification(void);
void test_rational_addition(void);
void test_rational_subtraction(void);
void test_rational_multiplication(void);
void test_rational_division(void);
void test_rational_comparison(void);
void test_rational_to_mpfr_conversion(void);
```

---

### Files to Modify

#### 1. `src/parser/ast.h` & `src/parser/ast.c`
**Current Purpose:** AST node definitions and creation functions
**Current Size:** ~4.8K
**Changes Required:** Minor additions

**Modifications:**
- **Option A (Recommended - Lightweight):** No changes to AST structure, use evaluation mode flag
- **Option B (If needed):** Add symbolic expression support to node union

**Recommended approach (Option A):**
```c
// In ast.h - No structural changes needed!
// Existing AST is perfect for symbolic computation
// Just add utility functions in ast.c:

// Clone an AST (needed for symbolic transformations)
ASTNode *ast_clone(const ASTNode *node);

// Deep comparison of two ASTs
int ast_equals(const ASTNode *a, const ASTNode *b);

// Check if AST represents a constant value
int ast_is_constant_value(const ASTNode *node);
```

**Estimated Changes:** +50 lines in `ast.c`, +5 function declarations in `ast.h`

---

#### 2. `src/ui/repl.c` & `src/ui/repl.h`
**Current Purpose:** Main REPL loop and expression processing
**Current Size:** ~4K
**Changes Required:** Add mode-switching logic

**Modifications:**

**In `repl.h`:**
```c
// Add evaluation mode enum
typedef enum {
    EVAL_MODE_NUMERIC,    // Current behavior
    EVAL_MODE_SYMBOLIC    // New symbolic mode
} EvalMode;

// Add mode management functions
void repl_set_eval_mode(EvalMode mode);
EvalMode repl_get_eval_mode(void);
```

**In `repl.c`:**
```c
// Add global state (near line 16)
static EvalMode eval_mode = EVAL_MODE_NUMERIC;

// Modify repl_process_line() around line 154-169:
if (eval_mode == EVAL_MODE_SYMBOLIC) {
    // New symbolic path
    ASTNode *simplified = symbolic_eval(ast);

    const char *symbolic_error = symbolic_get_last_error();
    if (symbolic_error) {
        printf("Symbolic error: %s\n", symbolic_error);
    } else {
        printf("= ");
        printer_print_ast_infix(simplified);
        printf("\n");
    }

    ast_free(simplified);
} else {
    // Existing numeric evaluation code (lines 154-169)
    mpfr_t result;
    mpfr_init2(result, global_precision);
    evaluator_eval(result, ast);
    // ... rest of existing code
}

// Add mode management functions
void repl_set_eval_mode(EvalMode mode) {
    eval_mode = mode;
}

EvalMode repl_get_eval_mode(void) {
    return eval_mode;
}
```

**Estimated Changes:** +40 lines

---

#### 3. `src/ui/commands.c` & `src/ui/commands.h`
**Current Purpose:** Built-in command handling (quit, help, precision, etc.)
**Current Size:** ~12.8K
**Changes Required:** Add `mode` command

**Modifications:**

**In `commands.c`:**
```c
// Add to command execution (around line 100-200 in commands_execute):
if (strcmp(cmd->name, "mode") == 0) {
    if (!cmd->argument) {
        // Display current mode
        EvalMode current = repl_get_eval_mode();
        printf("Current mode: %s\n",
               current == EVAL_MODE_NUMERIC ? "numeric" : "symbolic");
        printf("Available modes: numeric, symbolic\n");
        return 0;
    }

    if (strcmp(cmd->argument, "numeric") == 0) {
        repl_set_eval_mode(EVAL_MODE_NUMERIC);
        printf("Switched to numeric evaluation mode\n");
        return 0;
    } else if (strcmp(cmd->argument, "symbolic") == 0) {
        repl_set_eval_mode(EVAL_MODE_SYMBOLIC);
        printf("Switched to symbolic evaluation mode\n");
        return 0;
    } else {
        printf("Unknown mode: %s\n", cmd->argument);
        printf("Available modes: numeric, symbolic\n");
        return -1;
    }
}

// Add to help command documentation:
printf("  mode [numeric|symbolic] - Switch evaluation mode\n");
```

**Estimated Changes:** +30 lines

---

#### 4. `src/core/evaluator.h`
**Current Purpose:** Interface for numeric evaluation
**Current Size:** Small header file
**Changes Required:** Minimal, possibly none

**Possible Addition (if desired):**
```c
// Optional: Add evaluation mode parameter variant
void evaluator_eval_with_mode(mpfr_t result, const ASTNode *node, EvalMode mode);
```

**Note:** May not be needed if mode is handled at REPL level. Keep evaluator focused on numeric evaluation.

**Estimated Changes:** 0-10 lines (optional)

---

#### 5. `Makefile`
**Current Purpose:** Build configuration
**Changes Required:** Add new source files to build

**Modifications:**
```makefile
# Add new source files to SOURCES variable (around line 20-30):
SOURCES += src/core/symbolic.c
SOURCES += src/core/rational.c
SOURCES += src/core/simplify_rules.c

# Add new test files to TEST_SOURCES (around line 40-50):
TEST_SOURCES += tests/test_symbolic.c
TEST_SOURCES += tests/test_rational.c
```

**Estimated Changes:** +5 lines

---

#### 6. `README.md`
**Current Purpose:** Project documentation
**Changes Required:** Document new symbolic mode feature

**New Section to Add:**
```markdown
## Symbolic Computation Mode

The calculator supports both numeric and symbolic evaluation modes.

### Switching Modes

- `mode numeric` - Evaluate expressions numerically (default)
- `mode symbolic` - Simplify expressions symbolically
- `mode` - Show current mode

### Symbolic Mode Examples

In symbolic mode, expressions are simplified algebraically rather than evaluated:

```
> mode symbolic
Switched to symbolic evaluation mode

> sin(0) + sin(pi) + 2 * sin(sqrt(2))/2
= sin(√2)

> (sqrt(2) + 5)/sqrt(2)
= 1 + 5√2 ÷ 2

> cos(pi/3)
= 1/2
```

Switch back to numeric mode to evaluate:

```
> mode numeric
Switched to numeric evaluation mode

> sin(sqrt(2))
= 0.9877659459927355809449224892456
```

### Symbolic Simplification Rules

The symbolic evaluator applies the following simplifications:

- **Arithmetic identities**: x + 0 → x, x × 1 → x, x × 0 → 0
- **Trigonometric values**: sin(0) → 0, cos(π) → -1, tan(π/4) → 1
- **Radical simplification**: √(x²) → |x|, √8 → 2√2
- **Rationalization**: a/√b → a√b/b
- **Fraction arithmetic**: Exact rational number operations
```

**Estimated Changes:** +50 lines

---

### Files That Remain Unchanged (Reused As-Is)

These files provide functionality that symbolic mode leverages without modification:

#### 1. `src/output/printer.c` ✅
**Why unchanged:** Already has `printer_print_ast_infix()` that beautifully displays symbolic expressions with Unicode (π, √, ×, ÷). This is **perfect** for symbolic output.

#### 2. `src/parser/parser.c` ✅
**Why unchanged:** Parsing logic is independent of evaluation mode. AST construction works identically for both modes.

#### 3. `src/lexer/lexer.c` ✅
**Why unchanged:** Tokenization is evaluation-agnostic. Same tokens work for both numeric and symbolic modes.

#### 4. `src/core/evaluator.c` ✅
**Why unchanged:** Stays focused on numeric evaluation. Symbolic evaluation is in separate module (`symbolic.c`).

#### 5. `src/core/functions.c` ✅
**Why unchanged:** Numeric function implementations remain as-is. Symbolic mode uses rule-based simplification instead.

#### 6. `src/core/constants.c` ✅
**Why unchanged:** Constant value computation unchanged. Symbolic mode references constants by name in AST.

#### 7. `src/core/precision.c` ✅
**Why unchanged:** MPFR precision settings independent of evaluation mode.

---

## Architecture Overview

### Current System (Numeric Mode)
```
Input String
    ↓
Lexer (lexer.c) → Tokens
    ↓
Parser (parser.c) → AST
    ↓
Evaluator (evaluator.c) → mpfr_t result
    ↓
Formatter (formatter.c) → Formatted output
    ↓
Output to user
```

### New System (Dual Mode)
```
Input String
    ↓
Lexer (lexer.c) → Tokens
    ↓
Parser (parser.c) → AST
    ↓
┌─────────────────────┴──────────────────────┐
│                                             │
│ REPL Mode Check                             │
│                                             │
├─────────────────┬───────────────────────────┤
│                 │                           │
│  NUMERIC MODE   │     SYMBOLIC MODE         │
│                 │                           │
│  evaluator.c    │     symbolic.c            │
│      ↓          │          ↓                │
│  mpfr_t result  │  Simplified AST           │
│      ↓          │          ↓                │
│  formatter.c    │  printer.c (infix)        │
│                 │                           │
└─────────────────┴───────────────────────────┘
                  ↓
            Output to user
```

---

## Detailed Implementation Phases

### Phase 1: Foundation (Week 1-2)

#### Task 1.1: Create Evaluation Mode Infrastructure
**Files:** `src/ui/repl.h`, `src/ui/repl.c`

**Steps:**
1. Add `EvalMode` enum to `repl.h`
2. Add global `eval_mode` variable to `repl.c`
3. Add `repl_set_eval_mode()` and `repl_get_eval_mode()` functions
4. Test mode switching (no-op for now, just state management)

**Acceptance Criteria:**
- [ ] Can switch between modes programmatically
- [ ] Mode state persists across REPL iterations
- [ ] Default mode is numeric

---

#### Task 1.2: Implement Rational Number System
**Files:** `src/core/rational.c`, `src/core/rational.h`, `tests/test_rational.c`

**Steps:**
1. Define `Rational` struct using GMP `mpz_t`
2. Implement init/clear functions
3. Implement arithmetic operations (+, -, ×, ÷)
4. Implement `rational_simplify()` using GCD
5. Implement comparison and utility functions
6. Write comprehensive unit tests
7. Add to Makefile

**Acceptance Criteria:**
- [ ] All arithmetic operations work correctly
- [ ] Fractions automatically reduce to lowest terms
- [ ] Can convert between rational and MPFR
- [ ] All unit tests pass
- [ ] No memory leaks (verify with valgrind)

---

#### Task 1.3: Create Symbolic Evaluator Skeleton
**Files:** `src/core/symbolic.c`, `src/core/symbolic.h`

**Steps:**
1. Create basic `symbolic_eval()` function
2. Implement AST traversal without simplification
3. Add `symbolic_clone()` helper (deep copy AST)
4. Add error handling infrastructure
5. Implement basic passthrough (returns clone of input)

**Acceptance Criteria:**
- [ ] `symbolic_eval()` compiles and links
- [ ] Returns valid AST clone
- [ ] No memory leaks
- [ ] Basic error handling works

---

#### Task 1.4: Add Mode-Switching UI Commands
**Files:** `src/ui/commands.c`, `src/ui/commands.h`

**Steps:**
1. Add `mode` command to command parser
2. Implement mode switching logic
3. Add to help text
4. Test mode command with various inputs

**Acceptance Criteria:**
- [ ] `mode` command shows current mode
- [ ] `mode numeric` switches to numeric mode
- [ ] `mode symbolic` switches to symbolic mode
- [ ] Invalid mode names show error message
- [ ] Help text documents new command

---

#### Task 1.5: Integrate Mode Switching into REPL
**Files:** `src/ui/repl.c`

**Steps:**
1. Modify `repl_process_line()` to check mode
2. Add symbolic evaluation branch
3. Use `printer_print_ast_infix()` for symbolic output
4. Test with simple passthrough (no simplification yet)

**Acceptance Criteria:**
- [ ] Numeric mode works exactly as before
- [ ] Symbolic mode displays parsed expression (unsimplified)
- [ ] Can switch modes during REPL session
- [ ] No crashes or memory leaks

---

#### Task 1.6: Set Up Testing Framework
**Files:** `tests/test_symbolic.c`, `Makefile`

**Steps:**
1. Create test file structure
2. Add helper functions for AST comparison
3. Write first trivial test (passthrough)
4. Add to Makefile test target
5. Verify test runs and passes

**Acceptance Criteria:**
- [ ] Test framework compiles
- [ ] Can run symbolic tests independently
- [ ] First test passes

---

### Phase 2: Core Simplifications (Week 2-3)

#### Task 2.1: Implement Arithmetic Simplification Rules
**Files:** `src/core/simplify_rules.c`, `src/core/simplify_rules.h`, `tests/test_symbolic.c`

**Steps:**
1. Create rule infrastructure
2. Implement: x + 0 → x
3. Implement: x - 0 → x
4. Implement: x × 0 → 0
5. Implement: x × 1 → x
6. Implement: x ÷ 1 → x
7. Implement: x - x → 0
8. Implement: x ÷ x → 1
9. Implement: 0 + x → x (commutative)
10. Write tests for each rule

**Acceptance Criteria:**
- [ ] All arithmetic identity rules work
- [ ] Rules handle both left and right positions
- [ ] Tests pass for all rules
- [ ] Can simplify: `x + 0 - 0 * y` → `x`

---

#### Task 2.2: Implement Trigonometric Simplification Rules
**Files:** `src/core/simplify_rules.c`, `tests/test_symbolic.c`

**Steps:**
1. Create lookup table of exact trig values
2. Implement sin exact values: 0, π/6, π/4, π/3, π/2, π, 2π, etc.
3. Implement cos exact values
4. Implement tan exact values
5. Handle negative angles
6. Handle angle arithmetic (detect π/2, etc.)
7. Write comprehensive tests

**Known Exact Values:**
```
sin(0) = 0          cos(0) = 1          tan(0) = 0
sin(π/6) = 1/2      cos(π/6) = √3/2     tan(π/6) = √3/3
sin(π/4) = √2/2     cos(π/4) = √2/2     tan(π/4) = 1
sin(π/3) = √3/2     cos(π/3) = 1/2      tan(π/3) = √3
sin(π/2) = 1        cos(π/2) = 0        tan(π/2) = undefined
sin(π) = 0          cos(π) = -1         tan(π) = 0
```

**Acceptance Criteria:**
- [ ] All exact trig values simplify correctly
- [ ] `sin(0) + sin(pi)` → `0`
- [ ] Tests cover all major angles
- [ ] Handles both constant and computed angles (π/2, etc.)

---

#### Task 2.3: Implement Radical/Sqrt Simplification Rules
**Files:** `src/core/simplify_rules.c`, `tests/test_symbolic.c`

**Steps:**
1. Implement: sqrt(0) → 0
2. Implement: sqrt(1) → 1
3. Implement: sqrt(4) → 2 (perfect squares)
4. Implement: sqrt(x²) → |x|
5. Implement: sqrt(x) × sqrt(x) → x
6. Implement: sqrt(a×b) → sqrt(a) × sqrt(b)
7. Implement: sqrt(a/b) → sqrt(a) / sqrt(b)
8. Implement radicand simplification: sqrt(8) → 2×sqrt(2)
9. Write tests

**Acceptance Criteria:**
- [ ] Perfect squares simplify to integers
- [ ] Radicand simplification works (extract perfect square factors)
- [ ] Product/quotient rules work
- [ ] Tests cover edge cases (sqrt(0), sqrt(1))

---

#### Task 2.4: Implement Fraction Rationalization
**Files:** `src/core/simplify_rules.c`, `tests/test_symbolic.c`

**Steps:**
1. Detect division by sqrt
2. Implement: a / sqrt(b) → a×sqrt(b) / b
3. Implement distribution: (a+b)/c → a/c + b/c
4. Implement fraction combination: a/c + b/c → (a+b)/c
5. Handle nested radicals
6. Write tests including user's example

**Acceptance Criteria:**
- [ ] `(sqrt(2) + 5) / sqrt(2)` → `1 + 5×sqrt(2)/2`
- [ ] Simple rationalization works: `1/sqrt(2)` → `sqrt(2)/2`
- [ ] Tests pass for various fraction forms

---

#### Task 2.5: Add Pattern Matching Infrastructure
**Files:** `src/core/simplify_rules.c`, `src/core/symbolic.c`

**Steps:**
1. Implement `matches_exact_value()`
2. Implement `matches_constant()`
3. Implement `is_sqrt_node()`
4. Implement `is_power_node()`
5. Implement `ast_equals()` in ast.c
6. Implement `symbolic_is_zero()`, `symbolic_is_one()`
7. Integrate pattern matching into rule application

**Acceptance Criteria:**
- [ ] Can detect if node matches specific value
- [ ] Can detect if two ASTs are structurally equal
- [ ] Pattern matching works for complex nested expressions
- [ ] Tests verify pattern matching accuracy

---

#### Task 2.6: Integrate Rules into Symbolic Evaluator
**Files:** `src/core/symbolic.c`

**Steps:**
1. Add rule application to `symbolic_simplify_binop()`
2. Add rule application to `symbolic_simplify_function()`
3. Add recursive simplification
4. Add iterative simplification (apply until no changes)
5. Test with user's examples

**Acceptance Criteria:**
- [ ] User's example 1 works: `sin(0) + sin(pi) + 2*sin(sqrt(2))/2` → `sin(sqrt(2))`
- [ ] User's example 2 works: `(sqrt(2) + 5)/sqrt(2)` → `1 + 5*sqrt(2)/2`
- [ ] Complex nested expressions simplify correctly
- [ ] No infinite loops in simplification

---

### Phase 3: Advanced Features (Week 3-4)

#### Task 3.1: Add Expression Canonicalization
**Files:** `src/core/symbolic.c`, `src/parser/ast.c`

**Steps:**
1. Implement canonical ordering (sort commutative operations)
2. Implement term collection (combine like terms)
3. Implement constant folding (evaluate numeric subexpressions)
4. Implement expression normalization
5. Write tests

**Acceptance Criteria:**
- [ ] `x + y` and `y + x` canonicalize to same form
- [ ] `2 + 3 + x` → `5 + x`
- [ ] Like terms combine: `2x + 3x` → `5x` (if x is symbolic)

---

#### Task 3.2: Implement Like-Term Collection
**Files:** `src/core/simplify_rules.c`

**Steps:**
1. Detect like terms (same symbolic part)
2. Combine coefficients
3. Handle addition and subtraction
4. Handle multiplication (distribute)
5. Write tests

**Acceptance Criteria:**
- [ ] `sqrt(2) + sqrt(2)` → `2×sqrt(2)`
- [ ] `3×sin(x) + 2×sin(x)` → `5×sin(x)`
- [ ] Tests verify correctness

---

#### Task 3.3: Add Symbolic Expression Comparison
**Files:** `src/parser/ast.c`

**Steps:**
1. Implement deep structural comparison
2. Handle floating-point comparison (within epsilon)
3. Handle canonical form comparison
4. Write tests

**Acceptance Criteria:**
- [ ] Can detect if two ASTs represent same expression
- [ ] Handles reordered commutative operations
- [ ] Tests cover edge cases

---

#### Task 3.4: Handle Edge Cases
**Files:** `src/core/symbolic.c`, `tests/test_symbolic.c`

**Steps:**
1. Division by zero detection (symbolic)
2. Domain errors (sqrt of negative, etc.)
3. Undefined operations (tan(π/2), etc.)
4. Very deep recursion
5. Write tests for all edge cases

**Acceptance Criteria:**
- [ ] `1/0` produces error or stays symbolic
- [ ] `sqrt(-1)` produces error or stays symbolic
- [ ] `tan(pi/2)` handled gracefully
- [ ] Deep nesting doesn't crash
- [ ] All edge case tests pass

---

#### Task 3.5: Expand Rule Set for Common Cases
**Files:** `src/core/simplify_rules.c`, `tests/test_symbolic.c`

**Steps:**
1. Add power rules (x^0 → 1, x^1 → x)
2. Add logarithm rules (log(1) → 0, log(e) → 1)
3. Add exponential rules (e^0 → 1, e^(ln(x)) → x)
4. Add absolute value rules
5. Write tests

**Acceptance Criteria:**
- [ ] Common power patterns simplify
- [ ] Common log/exp patterns simplify
- [ ] Tests verify all new rules

---

### Phase 4: Testing & Polish (Week 4)

#### Task 4.1: Comprehensive Unit Tests
**Files:** `tests/test_symbolic.c`, `tests/test_rational.c`

**Steps:**
1. Ensure >90% code coverage
2. Add stress tests (deeply nested expressions)
3. Add property-based tests (simplification idempotence)
4. Add regression tests for any bugs found
5. Test with valgrind for memory leaks

**Acceptance Criteria:**
- [ ] All unit tests pass
- [ ] Code coverage >90%
- [ ] No memory leaks detected
- [ ] Stress tests pass

---

#### Task 4.2: Integration Tests for Mode Switching
**Files:** New `tests/test_integration.c`

**Steps:**
1. Test switching modes mid-session
2. Test evaluating same expression in both modes
3. Test REPL command integration
4. Test with example scripts

**Acceptance Criteria:**
- [ ] Can switch modes without crashes
- [ ] Same input produces different output in different modes
- [ ] Mode persists correctly
- [ ] Integration tests pass

---

#### Task 4.3: Test User's Example Expressions
**Files:** `tests/test_symbolic.c`

**Steps:**
1. Create dedicated test for each user example
2. Verify exact expected output
3. Test variations of examples
4. Document test cases

**Acceptance Criteria:**
- [ ] `sin(0) + sin(pi) + 2*sin(sqrt(2))/2 = sin(sqrt(2))` ✓
- [ ] `(sqrt(2) + 5)/sqrt(2) = 1 + 5*sqrt(2)/2` ✓
- [ ] Variations of examples work
- [ ] Tests documented clearly

---

#### Task 4.4: Documentation Updates
**Files:** `README.md`, new `docs/SYMBOLIC_MODE.md`

**Steps:**
1. Update README with symbolic mode section
2. Create detailed symbolic mode documentation
3. Document simplification rules
4. Add usage examples
5. Update help command text

**Acceptance Criteria:**
- [ ] README includes symbolic mode overview
- [ ] Detailed docs explain all features
- [ ] Examples are clear and comprehensive
- [ ] Help command shows mode usage

---

#### Task 4.5: Performance Optimization
**Files:** `src/core/symbolic.c`, `src/core/simplify_rules.c`

**Steps:**
1. Profile symbolic evaluation
2. Optimize hot paths
3. Add memoization if needed
4. Limit simplification iterations
5. Benchmark against baseline

**Acceptance Criteria:**
- [ ] Symbolic mode performs reasonably (< 100ms for typical expressions)
- [ ] No exponential blowup in simplification
- [ ] Benchmarks show acceptable performance

---

## Technical Decisions & Trade-offs

### Decision 1: AST Structure
**Chosen:** Reuse existing AST, add evaluation mode flag
**Alternative:** Create separate symbolic expression type
**Rationale:** Minimal changes, leverages existing infrastructure, simpler implementation

### Decision 2: Rational Arithmetic
**Chosen:** GMP `mpz_t` for exact numerator/denominator
**Alternative:** Use MPFR with very high precision
**Rationale:** True exactness for fractions, already have GMP dependency

### Decision 3: Rule Application Strategy
**Chosen:** Iterative rule application until no changes
**Alternative:** Single-pass transformation
**Rationale:** More thorough simplification, handles transitive rules

### Decision 4: Mode Switching
**Chosen:** Global REPL state variable
**Alternative:** Per-expression mode parameter
**Rationale:** Simpler UX, matches calculator workflow (change mode, then evaluate many expressions)

### Decision 5: Expression Output
**Chosen:** Reuse existing `printer_print_ast_infix()`
**Alternative:** Create new symbolic printer
**Rationale:** Already perfect, uses Unicode, handles precedence correctly

---

## Success Criteria

The symbolic mode implementation will be considered complete when:

- [ ] All checkboxes in all phases are checked
- [ ] User's example expressions produce correct output
- [ ] All unit tests pass
- [ ] Integration tests pass
- [ ] No memory leaks (verified with valgrind)
- [ ] Documentation is complete and accurate
- [ ] Code review completed
- [ ] Performance is acceptable (< 100ms for typical expressions)

---

## Future Enhancements (Out of Scope for Initial Release)

Ideas for future expansion:

- [ ] Symbolic differentiation
- [ ] Symbolic integration (simple cases)
- [ ] Equation solving (isolate variable)
- [ ] Matrix operations
- [ ] Complex number support
- [ ] Variable substitution
- [ ] Expression simplification preferences (user-configurable)
- [ ] LaTeX output for symbolic expressions
- [ ] Graph plotting of symbolic expressions

---

## Resources & References

### Mathematical Resources
- **Trigonometric Identities:** https://en.wikipedia.org/wiki/List_of_trigonometric_identities
- **Radical Simplification:** https://en.wikipedia.org/wiki/Nth_root#Simplified_form_of_a_radical_expression
- **Rationalization:** https://en.wikipedia.org/wiki/Rationalisation_(mathematics)

### Libraries Used
- **GMP (GNU Multiple Precision):** https://gmplib.org/
- **MPFR (Multiple Precision Floating-Point Reliable):** https://www.mpfr.org/

### Testing Tools
- **Valgrind (memory leak detection):** https://valgrind.org/
- **GDB (debugging):** https://www.gnu.org/software/gdb/

---

## Notes

- This roadmap assumes one developer working full-time
- Estimates may vary based on familiarity with codebase and mathematical algorithms
- Phases can overlap; testing should be continuous throughout
- Regular commits and incremental progress recommended
- Consider feature branches for major components

---

**Last Updated:** 2025-12-26
**Status:** Planning Phase
**Next Milestone:** Phase 1 - Foundation
