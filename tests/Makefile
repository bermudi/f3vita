# f3vita Pattern Module Tests - Makefile
#
# Usage:
#   make        - Build test executable
#   make test   - Build and run tests
#   make clean  - Remove build artifacts

CC ?= gcc
CFLAGS = -Wall -Wextra -std=c99 -I../include -O2
LDFLAGS =

# Source files
TEST_SRC = test_pattern.c
PATTERN_SRC = ../src/pattern.c
TARGET = test_pattern

# Default target
all: $(TARGET)

# Build test executable
$(TARGET): $(TEST_SRC) $(PATTERN_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Build and run tests
test: $(TARGET)
	@echo ""
	@./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Run tests with verbose output (for debugging)
verbose: CFLAGS += -DVERBOSE
verbose: clean $(TARGET)
	@./$(TARGET)

# Build with debug symbols
debug: CFLAGS += -g -O0
debug: clean $(TARGET)

# Build with sanitizers (if available)
sanitize: CFLAGS += -fsanitize=address,undefined -g
sanitize: LDFLAGS += -fsanitize=address,undefined
sanitize: clean $(TARGET)
	@./$(TARGET)

.PHONY: all test clean verbose debug sanitize