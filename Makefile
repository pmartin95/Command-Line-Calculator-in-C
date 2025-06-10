CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -Iinclude
SRC_DIR = src
TEST_DIR = tests
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include

# Source files (excluding main.c for library)
LIB_SOURCES = $(filter-out $(SRC_DIR)/main.c, $(wildcard $(SRC_DIR)/*.c))
LIB_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIB_SOURCES))

# Main program
MAIN_OBJECT = $(OBJ_DIR)/main.o
TARGET = $(BIN_DIR)/calculator

# Test files
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJECTS = $(patsubst $(TEST_DIR)/%.c, $(OBJ_DIR)/test_%.o, $(TEST_SOURCES))
TEST_TARGET = $(BIN_DIR)/test_calculator

# Check if readline is available
READLINE_CHECK := $(shell pkg-config --exists readline 2>/dev/null && echo "yes" || echo "no")

# Set up readline flags if available
ifeq ($(READLINE_CHECK),yes)
    CFLAGS += -DHAVE_READLINE $(shell pkg-config --cflags readline 2>/dev/null)
    LDFLAGS += $(shell pkg-config --libs readline 2>/dev/null)
    READLINE_STATUS = "enabled"
else
    READLINE_STATUS = "disabled (install libreadline-dev for history support)"
endif

# Check if MPFR is available
MPFR_CHECK := $(shell pkg-config --exists mpfr 2>/dev/null && echo "yes" || echo "no")

# Set up MPFR flags if available
ifeq ($(MPFR_CHECK),yes)
    CFLAGS += $(shell pkg-config --cflags mpfr 2>/dev/null)
    LDFLAGS += $(shell pkg-config --libs mpfr 2>/dev/null)
    MPFR_STATUS = "enabled"
else
    # Try manual linking as fallback
    LDFLAGS += -lmpfr -lgmp
    MPFR_STATUS = "enabled - manual linking"
endif

.PHONY: clean help info basic readline test run-tests all

# Default target
all: info $(TARGET)

# Main calculator program
$(TARGET): $(LIB_OBJECTS) $(MAIN_OBJECT) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -lm $(LDFLAGS) -o $@

# Test program
$(TEST_TARGET): $(LIB_OBJECTS) $(TEST_OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -lm $(LDFLAGS) -o $@

# Library object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Test object files
$(OBJ_DIR)/test_%.o: $(TEST_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Directory creation
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

# Test targets
test: $(TEST_TARGET)
	@echo "Running test suite..."
	@./$(TEST_TARGET)

run-tests: test

# Force build without readline
basic: CFLAGS = -Wall -Wextra -pedantic -std=c11 -Iinclude
basic: LDFLAGS = 
basic: clean $(TARGET)
	@echo "Built basic version (no readline support)"

# Force build with readline
readline: CFLAGS += -DHAVE_READLINE
readline: LDFLAGS += -lreadline
readline: clean $(TARGET)
	@echo "Built with readline support"

# Build everything including tests
full: $(TARGET) $(TEST_TARGET)
	@echo "Built calculator and tests"

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

info:
	@echo "Building calculator..."
	@echo "Readline support: $(READLINE_STATUS)"
	@echo "MPFR support: $(MPFR_STATUS)"
	@echo ""

help:
	@echo "High-Precision Calculator Build Targets:"
	@echo "  make all         - Build calculator (default)"
	@echo "  make test        - Build and run unit tests"
	@echo "  make run-tests   - Alias for 'make test'"
	@echo "  make full        - Build calculator and tests"
	@echo "  make readline    - Force build with readline support"
	@echo "  make basic       - Force build without readline"
	@echo "  make clean       - Remove built files"
	@echo "  make info        - Show build configuration"
	@echo "  make help        - Show this help"
	@echo ""
	@echo "Current readline status: $(READLINE_STATUS)"
	@echo "Current MPFR status: $(MPFR_STATUS)"
	@echo ""
	@echo "To install MPFR on Ubuntu/Debian:"
	@echo "  sudo apt-get install libmpfr-dev"
	@echo ""
	@echo "To install MPFR on macOS:"
	@echo "  brew install mpfr"
	@echo ""
	@echo "To install MPFR on CentOS/RHEL:"
	@echo "  sudo yum install mpfr-devel"