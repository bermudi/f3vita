# f3vita Unit Tests

Desktop-runnable unit tests for the f3vita pattern module.

## Prerequisites

- GCC or Clang C compiler
- Make (optional, for convenience)

## Quick Start

```bash
# Build and run tests
make test

# Or manually
gcc -Wall -Wextra -std=c99 -I../include -o test_pattern test_pattern.c ../src/pattern.c
./test_pattern
```

## Test Coverage

### Pattern Generation (`f3v_fill_pattern`)

| Test | Description |
|------|-------------|
| Deterministic Output | Same inputs produce identical buffers |
| Different File Index | Different file_idx → different patterns |
| Different Block Index | Different block_idx → different patterns |
| Formula Verification | Byte values match expected formula |
| Zero Indices | Edge case: file_idx=0, block_idx=0 |
| Large Indices | Edge case: large index values |

### Pattern Verification (`f3v_verify_pattern`)

| Test | Description |
|------|-------------|
| Perfect Match | Unmodified buffer → 0 corrupted bytes |
| Single Corruption | One corrupted byte detected |
| Multiple Corruption | Count of corrupted bytes correct |
| First Error Offset | Reports earliest corruption position |
| Null Offset Pointer | Handles NULL offset pointer safely |
| Full Corruption | All bytes corrupted → full count |
| First Byte Corruption | Detects corruption at offset 0 |
| Last Byte Corruption | Detects corruption at buffer end |
| Wrong File Index | Mismatched file_idx detected |
| Wrong Block Index | Mismatched block_idx detected |

## Make Targets

```bash
make          # Build test executable
make test     # Build and run tests
make clean    # Remove build artifacts
make debug    # Build with debug symbols
make sanitize # Build with address/undefined sanitizers
```

## Expected Output

```
=== f3vita Pattern Module Tests ===
Buffer size: 1048576 bytes (1 MB)

--- f3v_fill_pattern() Tests ---
Running: test_fill_deterministic... PASS
Running: test_fill_different_file... PASS
Running: test_fill_different_block... PASS
Running: test_fill_formula_verification... PASS
Running: test_fill_zero_indices... PASS
Running: test_fill_large_indices... PASS

--- f3v_verify_pattern() Tests ---
Running: test_verify_perfect_match... PASS
Running: test_verify_single_corruption... PASS
Running: test_verify_multiple_corruption... PASS
Running: test_verify_first_error_offset... PASS
Running: test_verify_null_offset... PASS
Running: test_verify_full_corruption... PASS
Running: test_verify_first_byte_corruption... PASS
Running: test_verify_last_byte_corruption... PASS
Running: test_verify_wrong_file_index... PASS
Running: test_verify_wrong_block_index... PASS

=== Results: 16/16 passed ===
All tests passed!
```

## Notes

- Tests use the full 1MB block size (F3V_BLOCK_SIZE) to match actual f3vita behavior
- Tests are pure C99 with no external dependencies
- The pattern module has no Vita-specific dependencies, so it compiles on any platform
- Static buffers are used to avoid stack overflow with 1MB allocations