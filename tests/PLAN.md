# Pattern Module Unit Tests - Implementation Plan

## Overview

This document outlines the plan for creating desktop-runnable unit tests for the f3vita pattern module (`pattern.c` / `pattern.h`).

## Approach

**Simple Test Harness** - A self-contained C file with custom assertion macros:
- Zero external dependencies
- Compiles with standard gcc/clang
- Clear pass/fail output
- Easy to extend

## File Structure

```
f3vita/
├── tests/
│   ├── PLAN.md              # This document
│   ├── test_pattern.c       # Test harness and test cases
│   ├── Makefile             # Build configuration
│   └── README.md            # Usage instructions
├── src/
│   └── pattern.c            # Source under test
└── include/
    ├── pattern.h            # API under test
    └── types.h              # Type definitions
```

## Test Cases

### 1. f3v_fill_pattern() Tests

| Test ID | Name | Description |
|---------|------|-------------|
| FP001 | Deterministic Output | Same (file_idx, block_idx) always produces identical buffer |
| FP002 | Different File Index | Different file_idx values produce different patterns |
| FP003 | Different Block Index | Different block_idx values produce different patterns |
| FP004 | Pattern Formula Verification | Verify expected values at specific offsets match formula |
| FP005 | Edge Case - Zero Indices | file_idx=0, block_idx=0 produces valid pattern |
| FP006 | Edge Case - Large Indices | Large file_idx/block_idx values handled correctly |

### 2. f3v_verify_pattern() Tests

| Test ID | Name | Description |
|---------|------|-------------|
| VP001 | Perfect Match | Unmodified buffer returns 0 corrupted bytes |
| VP002 | Single Byte Corruption | One modified byte returns corrupted=1 |
| VP003 | Multiple Corruption | Multiple modified bytes counted correctly |
| VP004 | First Error Offset | first_error_offset points to correct byte |
| VP005 | Null Offset Pointer | NULL first_error_offset handled without crash |
| VP006 | Full Corruption | Entire buffer corrupted returns correct count |
| VP007 | First Byte Corruption | Corruption at offset 0 detected |
| VP008 | Last Byte Corruption | Corruption at buffer end detected |

## Test Harness Design

```c
// Simple assertion macros
#define TEST_ASSERT(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s - %s\n", __func__, msg); \
        g_tests_failed++; \
        return; \
    } \
} while(0)

#define TEST_ASSERT_EQ(a, b, msg) TEST_ASSERT((a) == (b), msg)

// Test registration pattern
typedef void (*test_func)(void);

typedef struct {
    const char *name;
    test_func func;
} TestCase;

// Test runner
int run_all_tests(TestCase *tests, int count);
```

## Build Configuration

### Makefile

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I../include
SRC = test_pattern.c ../src/pattern.c
TARGET = test_pattern

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all test clean
```

### Dependencies

The pattern module only requires:
- `<stdint.h>` - Standard integer types
- `<stddef.h>` - size_t definition

No Vita-specific headers needed for testing.

## Pattern Formula Reference

From [`pattern.c:18`](../src/pattern.c:18):

```c
// Base pattern calculation
uint32_t base = (file_idx << 24) ^ (block_idx << 16);

// For each byte in the buffer
uint32_t val = base ^ i;  // i = byte offset
buf[i] = (uint8_t)(val >> ((i & 3) * 8));
```

The formula ensures:
- Each location has a unique expected value
- Address aliasing (fake capacity) is detected
- Bit-flip corruption is detected

## Usage Instructions

```bash
# Navigate to tests directory
cd f3vita/tests

# Build tests
make

# Run tests
make test

# Or directly
./test_pattern
```

### Expected Output

```
=== f3vita Pattern Module Tests ===

Running: test_fill_deterministic... PASS
Running: test_fill_different_file... PASS
Running: test_fill_different_block... PASS
Running: test_fill_formula_verification... PASS
Running: test_verify_perfect_match... PASS
Running: test_verify_single_corruption... PASS
Running: test_verify_multiple_corruption... PASS
Running: test_verify_first_error_offset... PASS
Running: test_verify_null_offset... PASS

=== Results: 9/9 passed ===
```

## Implementation Checklist

- [x] Create `tests/` directory structure
- [x] Implement test harness macros and runner
- [x] Implement f3v_fill_pattern() test cases
- [x] Implement f3v_verify_pattern() test cases
- [x] Create Makefile
- [x] Create README.md with usage instructions
- [x] Run and verify all tests pass
- [x] Document any edge cases discovered

## Notes

- Tests use the full F3V_BLOCK_SIZE (1MB) to match actual pattern module behavior
- Static buffers are used to avoid stack overflow with large allocations
- All 16 tests pass successfully
- Edge case discovered: Wrong file_idx/block_idx only affects ~25% of bytes due to byte extraction formula