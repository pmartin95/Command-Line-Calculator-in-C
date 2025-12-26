CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
SRC_DIR = src
TEST_DIR = tests
OBJ_DIR = obj
BIN_DIR = bin
INCLUDE_DIR = include

# Core module directories
CORE_DIR = $(SRC_DIR)/core
PARSER_DIR = $(SRC_DIR)/parser
LEXER_DIR = $(SRC_DIR)/lexer
OUTPUT_DIR = $(SRC_DIR)/output
UI_DIR = $(SRC_DIR)/ui

# Include paths for all modules
INCLUDES = -I$(INCLUDE_DIR) -I$(CORE_DIR) -I$(PARSER_DIR) -I$(LEXER_DIR) -I$(OUTPUT_DIR) -I$(UI_DIR)
CFLAGS += $(INCLUDES)

# Source files by module
CORE_SOURCES = $(wildcard $(CORE_DIR)/*.c)
PARSER_SOURCES = $(wildcard $(PARSER_DIR)/*.c)
LEXER_SOURCES = $(wildcard $(LEXER_DIR)/*.c)
OUTPUT_SOURCES = $(wildcard $(OUTPUT_DIR)/*.c)
UI_SOURCES = $(filter-out $(UI_DIR)/main.c, $(wildcard $(UI_DIR)/*.c))

# All library sources (excluding main)
LIB_SOURCES = $(CORE_SOURCES) $(PARSER_SOURCES) $(LEXER_SOURCES) $(OUTPUT_SOURCES) $(UI_SOURCES)
LIB_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIB_SOURCES))

# Main program
MAIN_SOURCE = $(UI_DIR)/main.c
MAIN_OBJECT = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(MAIN_SOURCE))
TARGET = $(BIN_DIR)/calculator

# Test files
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJECTS = $(patsubst $(TEST_DIR)/%.c, $(OBJ_DIR)/test_%.o, $(TEST_SOURCES))
TEST_TARGET = $(BIN_DIR)/test_calculator

# Check for dependencies
READLINE_CHECK := $(shell pkg-config --exists readline 2>/dev/null && echo "yes" || echo "no")
MPFR_CHECK := $(shell pkg-config --exists mpfr 2>/dev/null && echo "yes" || echo "no")

# Set up readline flags if available
ifeq ($(READLINE_CHECK),yes)
    CFLAGS += -DHAVE_READLINE $(shell pkg-config --cflags readline 2>/dev/null)
    LDFLAGS += $(shell pkg-config --libs readline 2>/dev/null)
    READLINE_STATUS = "enabled"
else
    READLINE_STATUS = "disabled (install libreadline-dev for history support)"
endif

# Set up MPFR flags
ifeq ($(MPFR_CHECK),yes)
    CFLAGS += $(shell pkg-config --cflags mpfr 2>/dev/null)
    LDFLAGS += $(shell pkg-config --libs mpfr 2>/dev/null)
    MPFR_STATUS = "enabled"
else
    LDFLAGS += -lmpfr -lgmp
    MPFR_STATUS = "enabled - manual linking"
endif

# Always link math library
LDFLAGS += -lm

.PHONY: clean help info test run-tests all modules debug release install uninstall

# Default target
all: info $(TARGET)

# Release build (optimized)
release: CFLAGS += -O3 -DNDEBUG
release: clean $(TARGET)

# Debug build
debug: CFLAGS += -g -O0 -DDEBUG -DDEBUG_AST
debug: clean $(TARGET)

# Main calculator program
$(TARGET): $(LIB_OBJECTS) $(MAIN_OBJECT) | $(BIN_DIR)
	@echo "Linking calculator..."
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
	@echo "✅ Calculator built successfully: $@"

# Test program
$(TEST_TARGET): $(LIB_OBJECTS) $(TEST_OBJECTS) | $(BIN_DIR)
	@echo "Linking test suite..."
	$(CC) $(CFLAGS) -DTESTING $^ $(LDFLAGS) -o $@
	@echo "✅ Test suite built successfully: $@"

# Pattern rule for core module objects
$(OBJ_DIR)/core/%.o: $(CORE_DIR)/%.c | $(OBJ_DIR)/core
	@echo "Compiling core module: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for parser module objects
$(OBJ_DIR)/parser/%.o: $(PARSER_DIR)/%.c | $(OBJ_DIR)/parser
	@echo "Compiling parser module: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for lexer module objects
$(OBJ_DIR)/lexer/%.o: $(LEXER_DIR)/%.c | $(OBJ_DIR)/lexer
	@echo "Compiling lexer module: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for output module objects
$(OBJ_DIR)/output/%.o: $(OUTPUT_DIR)/%.c | $(OBJ_DIR)/output
	@echo "Compiling output module: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for UI module objects
$(OBJ_DIR)/ui/%.o: $(UI_DIR)/%.c | $(OBJ_DIR)/ui
	@echo "Compiling UI module: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Test object files
$(OBJ_DIR)/test_%.o: $(TEST_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling test: $<"
	$(CC) $(CFLAGS) -DTESTING -c $< -o $@

# Directory creation
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/core: | $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/core

$(OBJ_DIR)/parser: | $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/parser

$(OBJ_DIR)/lexer: | $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/lexer

$(OBJ_DIR)/output: | $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/output

$(OBJ_DIR)/ui: | $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/ui

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Create module directories
modules:
	@echo "Creating modular directory structure..."
	mkdir -p $(CORE_DIR) $(PARSER_DIR) $(LEXER_DIR) $(OUTPUT_DIR) $(UI_DIR)
	mkdir -p $(TEST_DIR)
	mkdir -p $(INCLUDE_DIR)
	@echo "✅ Module directories created"

# Test targets
test: $(TEST_TARGET)
	@echo "🧪 Running test suite..."
	@./$(TEST_TARGET)

test-verbose: $(TEST_TARGET)
	@echo "🧪 Running test suite (verbose)..."
	@./$(TEST_TARGET) -v

test-quiet: $(TEST_TARGET)
	@./$(TEST_TARGET) -q

# Run specific test suites
test-lexer: $(TEST_TARGET)
	@echo "🧪 Running lexer tests..."
	@./$(TEST_TARGET) lexer

test-parser: $(TEST_TARGET)
	@echo "🧪 Running parser tests..."
	@./$(TEST_TARGET) parser

test-evaluator: $(TEST_TARGET)
	@echo "🧪 Running evaluator tests..."
	@./$(TEST_TARGET) evaluator

test-precision: $(TEST_TARGET)
	@echo "🧪 Running precision tests..."
	@./$(TEST_TARGET) precision

test-integration: $(TEST_TARGET)
	@echo "🧪 Running integration tests..."
	@./$(TEST_TARGET) integration

run-tests: test

# Force build without readline
basic: CFLAGS = -Wall -Wextra -pedantic -std=c11 $(INCLUDES)
basic: LDFLAGS = -lmpfr -lgmp -lm
basic: clean $(TARGET)
	@echo "✅ Built basic version (no readline support)"

# Force build with readline
readline: CFLAGS += -DHAVE_READLINE
readline: LDFLAGS += -lreadline
readline: clean $(TARGET)
	@echo "✅ Built with readline support"

# Build everything including tests
full: $(TARGET) $(TEST_TARGET)
	@echo "✅ Built calculator and tests"

# Install target (requires sudo)
install: $(TARGET)
	@echo "Installing calculator to /usr/local/bin..."
	sudo cp $(TARGET) /usr/local/bin/calculator
	@echo "✅ Calculator installed successfully"
	@echo "Run 'calculator' to start the high-precision calculator"

# Uninstall target
uninstall:
	@echo "Removing calculator from /usr/local/bin..."
	sudo rm -f /usr/local/bin/calculator
	@echo "✅ Calculator uninstalled"

# Clean targets
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "✅ Clean complete"

clean-tests:
	rm -f $(TEST_OBJECTS) $(TEST_TARGET)

# Development targets
run: $(TARGET)
	./$(TARGET)

run-debug: debug
	gdb ./$(TARGET)

valgrind: debug
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# Code analysis
analyze: 
	@echo "Running static analysis..."
	cppcheck --enable=all --std=c11 $(SRC_DIR)

# Count lines of code
count:
	@echo "Lines of code by module:"
	@echo "Core:    $$(find $(CORE_DIR) -name '*.c' -o -name '*.h' | xargs wc -l | tail -1)"
	@echo "Parser:  $$(find $(PARSER_DIR) -name '*.c' -o -name '*.h' | xargs wc -l | tail -1)"
	@echo "Lexer:   $$(find $(LEXER_DIR) -name '*.c' -o -name '*.h' | xargs wc -l | tail -1)"
	@echo "Output:  $$(find $(OUTPUT_DIR) -name '*.c' -o -name '*.h' | xargs wc -l | tail -1)"
	@echo "UI:      $$(find $(UI_DIR) -name '*.c' -o -name '*.h' | xargs wc -l | tail -1)"
	@echo "Tests:   $$(find $(TEST_DIR) -name '*.c' | xargs wc -l | tail -1)"
	@echo "Total:   $$(find $(SRC_DIR) $(TEST_DIR) -name '*.c' -o -name '*.h' | xargs wc -l | tail -1)"

info:
	@echo "Building modular high-precision calculator..."
	@echo 'Readline support: $(READLINE_STATUS)'
	@echo 'MPFR support: $(MPFR_STATUS)'
	@echo ""
	@echo "Module structure:"
	@echo "  Core:   $(words $(CORE_SOURCES)) files"
	@echo "  Parser: $(words $(PARSER_SOURCES)) files"
	@echo "  Lexer:  $(words $(LEXER_SOURCES)) files"
	@echo "  Output: $(words $(OUTPUT_SOURCES)) files"
	@echo "  UI:     $(words $(UI_SOURCES)) files"
	@echo "  Tests:  $(words $(TEST_SOURCES)) files"
	@echo ""

help:
	@echo "High-Precision Calculator Build System"
	@echo "======================================"
	@echo ""
	@echo "Main Targets:"
	@echo "  make all         - Build calculator (default)"
	@echo "  make release     - Build optimized release version"
	@echo "  make debug       - Build debug version with symbols"
	@echo "  make full        - Build calculator and tests"
	@echo ""
	@echo "Test Targets:"
	@echo "  make test        - Build and run all tests"
	@echo "  make test-lexer  - Run only lexer tests"
	@echo "  make test-parser - Run only parser tests"
	@echo "  make test-evaluator - Run only evaluator tests"
	@echo "  make test-precision - Run only precision tests"
	@echo "  make test-integration - Run only integration tests"
	@echo ""
	@echo "Development Targets:"
	@echo "  make run         - Build and run calculator"
	@echo "  make run-debug   - Run calculator in debugger"
	@echo "  make valgrind    - Run with memory leak detection"
	@echo "  make analyze     - Run static code analysis"
	@echo "  make count       - Count lines of code by module"
	@echo ""
	@echo "Build Variants:"
	@echo "  make basic       - Build without readline support"
	@echo "  make readline    - Force build with readline"
	@echo ""
	@echo "Installation:"
	@echo "  make install     - Install to /usr/local/bin (requires sudo)"
	@echo "  make uninstall   - Remove from /usr/local/bin"
	@echo ""
	@echo "Utility:"
	@echo "  make modules     - Create modular directory structure"
	@echo "  make clean       - Remove all build artifacts"
	@echo "  make info        - Show build configuration"
	@echo "  make help        - Show this help"
	@echo ""
	@echo "Module Structure:"
	@echo "  src/core/        - Mathematical operations (precision, constants, functions)"
	@echo "  src/parser/      - AST and parsing logic"
	@echo "  src/lexer/       - Tokenization and function tables"
	@echo "  src/output/      - Number formatting and display"
	@echo "  src/ui/          - User interface and REPL"
	@echo "  tests/           - Unit and integration tests"
	@echo ""
	@echo "Dependencies:"
	@echo "  Required: libmpfr-dev, libgmp-dev"
	@echo "  Optional: libreadline-dev (for command history)"
	@echo ""
	@echo "Installation commands:"
	@echo "  Ubuntu/Debian: sudo apt-get install libmpfr-dev libgmp-dev libreadline-dev"
	@echo "  macOS:         brew install mpfr readline"
	@echo "  CentOS/RHEL:   sudo yum install mpfr-devel gmp-devel readline-devel"