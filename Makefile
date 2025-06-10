CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -Iinclude
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))
TARGET = $(BIN_DIR)/program

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

.PHONY: clean help info basic readline

# Default target
all: info $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -lm $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(INC_DIR):
	mkdir -p $(INC_DIR)

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

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

info:
	@echo "Building calculator..."
	@echo "Readline support: $(READLINE_STATUS)"
	@echo ""

help:
	@echo "Calculator Build Targets:"
	@echo "  make all      - Build with readline if available, basic otherwise"
	@echo "  make readline - Force build with readline support"
	@echo "  make basic    - Force build without readline"
	@echo "  make clean    - Remove built files"
	@echo "  make info     - Show build configuration"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Current readline status: $(READLINE_STATUS)"
	@echo ""
	@echo "To install readline on Ubuntu/Debian:"
	@echo "  sudo apt-get install libreadline-dev"
	@echo ""
	@echo "To install readline on macOS:"
	@echo "  brew install readline"
	@echo ""
	@echo "To install readline on CentOS/RHEL:"
	@echo "  sudo yum install readline-devel"