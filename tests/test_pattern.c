/**
 * @file test_pattern.c
 * @brief Unit tests for f3vita pattern module
 *
 * Desktop-runnable tests for pattern generation and verification.
 * Compile: gcc -Wall -Wextra -std=c99 -I../include -o test_pattern test_pattern.c ../src/pattern.c
 * Run: ./test_pattern
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Include the pattern module header */
#include "pattern.h"

/*
 * Test Configuration
 * Note: We MUST use F3V_BLOCK_SIZE because the pattern functions
 * always operate on that size. Using static buffers to avoid
 * stack overflow with 1MB allocations.
 */

/*
 * Test Statistics
 */
static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

/*
 * Static buffers for testing (1MB each to match F3V_BLOCK_SIZE)
 */
static uint8_t g_buf1[F3V_BLOCK_SIZE];
static uint8_t g_buf2[F3V_BLOCK_SIZE];

/*
 * Test Assertion Macros
 */
#define TEST_ASSERT(cond, msg)           \
    do                                   \
    {                                    \
        if (!(cond))                     \
        {                                \
            printf("  FAIL: %s\n", msg); \
            g_tests_failed++;            \
            return 0;                    \
        }                                \
    } while (0)

#define TEST_ASSERT_EQ(actual, expected, msg)                 \
    do                                                        \
    {                                                         \
        if ((actual) != (expected))                           \
        {                                                     \
            printf("  FAIL: %s (expected %u, got %u)\n", msg, \
                   (unsigned)(expected), (unsigned)(actual)); \
            g_tests_failed++;                                 \
            return 0;                                         \
        }                                                     \
    } while (0)

#define TEST_ASSERT_NEQ(a, b, msg)                              \
    do                                                          \
    {                                                           \
        if ((a) == (b))                                         \
        {                                                       \
            printf("  FAIL: %s (values should differ)\n", msg); \
            g_tests_failed++;                                   \
            return 0;                                           \
        }                                                       \
    } while (0)

/*
 * Test Runner Macros
 */
#define RUN_TEST(test_func)                    \
    do                                         \
    {                                          \
        printf("Running: %s... ", #test_func); \
        g_tests_run++;                         \
        if (test_func())                       \
        {                                      \
            printf("PASS\n");                  \
            g_tests_passed++;                  \
        }                                      \
    } while (0)

/*
 * Helper Functions
 */

/**
 * Calculate expected byte value at a given offset (mirrors pattern.c logic)
 */
static uint8_t expected_byte(uint32_t file_idx, uint32_t block_idx, uint32_t offset)
{
    uint32_t base = (file_idx << 24) ^ (block_idx << 16);
    uint32_t val = base ^ offset;
    return (uint8_t)(val >> ((offset & 3) * 8));
}

/**
 * Compare two buffers and return 1 if equal
 */
static int buffers_equal(const uint8_t *a, const uint8_t *b, size_t size)
{
    return memcmp(a, b, size) == 0;
}

/*
 * =============================================================================
 * Test Cases for f3v_fill_pattern()
 * =============================================================================
 */

/**
 * FP001: Deterministic Output
 * Same (file_idx, block_idx) always produces identical buffer
 */
static int test_fill_deterministic(void)
{
    /* Fill twice with same parameters */
    f3v_fill_pattern(g_buf1, 1, 0);
    f3v_fill_pattern(g_buf2, 1, 0);

    TEST_ASSERT(buffers_equal(g_buf1, g_buf2, F3V_BLOCK_SIZE),
                "Same parameters should produce identical output");

    /* Try with different parameters */
    f3v_fill_pattern(g_buf1, 5, 10);
    f3v_fill_pattern(g_buf2, 5, 10);

    TEST_ASSERT(buffers_equal(g_buf1, g_buf2, F3V_BLOCK_SIZE),
                "Same parameters should produce identical output (different indices)");

    return 1;
}

/**
 * FP002: Different File Index
 * Different file_idx values produce different patterns
 */
static int test_fill_different_file(void)
{
    f3v_fill_pattern(g_buf1, 1, 0);
    f3v_fill_pattern(g_buf2, 2, 0);

    TEST_ASSERT(!buffers_equal(g_buf1, g_buf2, F3V_BLOCK_SIZE),
                "Different file_idx should produce different patterns");

    return 1;
}

/**
 * FP003: Different Block Index
 * Different block_idx values produce different patterns
 */
static int test_fill_different_block(void)
{
    f3v_fill_pattern(g_buf1, 1, 0);
    f3v_fill_pattern(g_buf2, 1, 1);

    TEST_ASSERT(!buffers_equal(g_buf1, g_buf2, F3V_BLOCK_SIZE),
                "Different block_idx should produce different patterns");

    return 1;
}

/**
 * FP004: Pattern Formula Verification
 * Verify expected values at specific offsets match the formula
 */
static int test_fill_formula_verification(void)
{
    uint32_t file_idx = 3;
    uint32_t block_idx = 7;

    f3v_fill_pattern(g_buf1, file_idx, block_idx);

    /* Check several specific offsets */
    TEST_ASSERT_EQ(g_buf1[0], expected_byte(file_idx, block_idx, 0),
                   "Byte at offset 0 should match formula");

    TEST_ASSERT_EQ(g_buf1[1], expected_byte(file_idx, block_idx, 1),
                   "Byte at offset 1 should match formula");

    TEST_ASSERT_EQ(g_buf1[100], expected_byte(file_idx, block_idx, 100),
                   "Byte at offset 100 should match formula");

    TEST_ASSERT_EQ(g_buf1[F3V_BLOCK_SIZE - 1],
                   expected_byte(file_idx, block_idx, F3V_BLOCK_SIZE - 1),
                   "Byte at last offset should match formula");

    return 1;
}

/**
 * FP005: Edge Case - Zero Indices
 * file_idx=0, block_idx=0 produces valid pattern
 */
static int test_fill_zero_indices(void)
{
    f3v_fill_pattern(g_buf1, 0, 0);

    /* Verify formula works with zeroes */
    TEST_ASSERT_EQ(g_buf1[0], expected_byte(0, 0, 0),
                   "Zero indices should produce valid pattern");

    /* Pattern should still vary across buffer */
    int all_same = 1;
    for (size_t i = 1; i < 1000 && all_same; i++)
    {
        if (g_buf1[i] != g_buf1[0])
            all_same = 0;
    }
    TEST_ASSERT(!all_same, "Buffer should not be uniform with zero indices");

    return 1;
}

/**
 * FP006: Edge Case - Large Indices
 * Large file_idx/block_idx values handled correctly
 */
static int test_fill_large_indices(void)
{
    /* Use values near upper limits */
    f3v_fill_pattern(g_buf1, 255, 65535);

    TEST_ASSERT_EQ(g_buf1[0], expected_byte(255, 65535, 0),
                   "Large indices should produce correct pattern");

    TEST_ASSERT_EQ(g_buf1[F3V_BLOCK_SIZE - 1],
                   expected_byte(255, 65535, F3V_BLOCK_SIZE - 1),
                   "Large indices should work at buffer end");

    return 1;
}

/*
 * =============================================================================
 * Test Cases for f3v_verify_pattern()
 * =============================================================================
 */

/**
 * VP001: Perfect Match
 * Unmodified buffer returns 0 corrupted bytes
 */
static int test_verify_perfect_match(void)
{
    uint32_t first_offset = 0xFFFFFFFF; /* Sentinel value */

    f3v_fill_pattern(g_buf1, 1, 0);
    uint32_t corrupted = f3v_verify_pattern(g_buf1, 1, 0, &first_offset);

    TEST_ASSERT_EQ(corrupted, 0, "Unmodified buffer should have 0 corrupted bytes");

    return 1;
}

/**
 * VP002: Single Byte Corruption
 * One modified byte returns corrupted=1
 */
static int test_verify_single_corruption(void)
{
    uint32_t first_offset = 0;

    f3v_fill_pattern(g_buf1, 1, 0);

    /* Corrupt one byte (flip all bits) */
    g_buf1[100] = ~g_buf1[100];

    uint32_t corrupted = f3v_verify_pattern(g_buf1, 1, 0, &first_offset);

    TEST_ASSERT_EQ(corrupted, 1, "Single byte corruption should report count=1");
    TEST_ASSERT_EQ(first_offset, 100, "First error offset should be 100");

    return 1;
}

/**
 * VP003: Multiple Corruption
 * Multiple modified bytes counted correctly
 */
static int test_verify_multiple_corruption(void)
{
    uint32_t first_offset = 0;

    f3v_fill_pattern(g_buf1, 1, 0);

    /* Corrupt 5 bytes at different locations */
    g_buf1[10] = ~g_buf1[10];
    g_buf1[50] = ~g_buf1[50];
    g_buf1[100] = ~g_buf1[100];
    g_buf1[500] = ~g_buf1[500];
    g_buf1[1000] = ~g_buf1[1000];

    uint32_t corrupted = f3v_verify_pattern(g_buf1, 1, 0, &first_offset);

    TEST_ASSERT_EQ(corrupted, 5, "Should report 5 corrupted bytes");
    TEST_ASSERT_EQ(first_offset, 10, "First error should be at offset 10");

    return 1;
}

/**
 * VP004: First Error Offset
 * first_error_offset points to correct byte
 */
static int test_verify_first_error_offset(void)
{
    uint32_t first_offset = 0xFFFFFFFF;

    f3v_fill_pattern(g_buf1, 2, 3);

    /* Corrupt bytes at 200 and 100 */
    g_buf1[200] = ~g_buf1[200];
    g_buf1[100] = ~g_buf1[100]; /* This is earlier, should be reported */

    uint32_t corrupted = f3v_verify_pattern(g_buf1, 2, 3, &first_offset);

    TEST_ASSERT_EQ(corrupted, 2, "Should report 2 corrupted bytes");
    TEST_ASSERT_EQ(first_offset, 100, "First error offset should be the earliest: 100");

    return 1;
}

/**
 * VP005: Null Offset Pointer
 * NULL first_error_offset handled without crash
 */
static int test_verify_null_offset(void)
{
    f3v_fill_pattern(g_buf1, 1, 0);
    g_buf1[50] = ~g_buf1[50]; /* Introduce corruption */

    /* Should not crash with NULL pointer */
    uint32_t corrupted = f3v_verify_pattern(g_buf1, 1, 0, NULL);

    TEST_ASSERT_EQ(corrupted, 1, "Should still count corruption with NULL offset pointer");

    return 1;
}

/**
 * VP006: Full Corruption
 * Entire buffer corrupted returns correct count
 */
static int test_verify_full_corruption(void)
{
    uint32_t first_offset = 0;

    f3v_fill_pattern(g_buf1, 1, 0);

    /* Corrupt entire buffer by flipping all bits */
    for (size_t i = 0; i < F3V_BLOCK_SIZE; i++)
    {
        g_buf1[i] = ~g_buf1[i];
    }

    uint32_t corrupted = f3v_verify_pattern(g_buf1, 1, 0, &first_offset);

    TEST_ASSERT_EQ(corrupted, F3V_BLOCK_SIZE, "All bytes should be reported corrupted");
    TEST_ASSERT_EQ(first_offset, 0, "First error should be at offset 0");

    return 1;
}

/**
 * VP007: First Byte Corruption
 * Corruption at offset 0 detected
 */
static int test_verify_first_byte_corruption(void)
{
    uint32_t first_offset = 0xFFFFFFFF;

    f3v_fill_pattern(g_buf1, 1, 0);
    g_buf1[0] = ~g_buf1[0]; /* Corrupt first byte */

    uint32_t corrupted = f3v_verify_pattern(g_buf1, 1, 0, &first_offset);

    TEST_ASSERT_EQ(corrupted, 1, "Should detect corruption at offset 0");
    TEST_ASSERT_EQ(first_offset, 0, "First error offset should be 0");

    return 1;
}

/**
 * VP008: Last Byte Corruption
 * Corruption at buffer end detected
 */
static int test_verify_last_byte_corruption(void)
{
    uint32_t first_offset = 0;

    f3v_fill_pattern(g_buf1, 1, 0);
    g_buf1[F3V_BLOCK_SIZE - 1] = ~g_buf1[F3V_BLOCK_SIZE - 1]; /* Corrupt last byte */

    uint32_t corrupted = f3v_verify_pattern(g_buf1, 1, 0, &first_offset);

    TEST_ASSERT_EQ(corrupted, 1, "Should detect corruption at last byte");
    TEST_ASSERT_EQ(first_offset, F3V_BLOCK_SIZE - 1, "First error should be at last offset");

    return 1;
}

/**
 * VP009: Wrong File Index
 * Using wrong file_idx detects bytes as corrupted
 *
 * Note: Due to the byte extraction formula (val >> ((i & 3) * 8)),
 * changing file_idx only affects ~25% of bytes (those where (i & 3) == 3).
 */
static int test_verify_wrong_file_index(void)
{
    uint32_t first_offset = 0;

    /* Fill with file_idx=1, verify with file_idx=2 */
    f3v_fill_pattern(g_buf1, 1, 0);
    uint32_t corrupted = f3v_verify_pattern(g_buf1, 2, 0, &first_offset);

    /* Should detect ~25% of bytes as wrong (every 4th byte where (i & 3) == 3) */
    TEST_ASSERT(corrupted > F3V_BLOCK_SIZE / 8,
                "Wrong file index should cause corruption detection (>12.5%)");

    return 1;
}

/**
 * VP010: Wrong Block Index
 * Using wrong block_idx detects corruption
 *
 * Note: Due to the byte extraction formula (val >> ((i & 3) * 8)),
 * changing block_idx only affects ~25% of bytes (those where (i & 3) == 2).
 */
static int test_verify_wrong_block_index(void)
{
    uint32_t first_offset = 0;

    /* Fill with block_idx=0, verify with block_idx=1 */
    f3v_fill_pattern(g_buf1, 1, 0);
    uint32_t corrupted = f3v_verify_pattern(g_buf1, 1, 1, &first_offset);

    /* Should detect ~25% of bytes as wrong (every 4th byte where (i & 3) == 2) */
    TEST_ASSERT(corrupted > F3V_BLOCK_SIZE / 8,
                "Wrong block index should cause corruption detection (>12.5%)");

    return 1;
}

/*
 * =============================================================================
 * Main Test Runner
 * =============================================================================
 */

int main(void)
{
    printf("\n=== f3vita Pattern Module Tests ===\n");
    printf("Buffer size: %d bytes (1 MB)\n\n", F3V_BLOCK_SIZE);

    /* f3v_fill_pattern() tests */
    printf("--- f3v_fill_pattern() Tests ---\n");
    RUN_TEST(test_fill_deterministic);
    RUN_TEST(test_fill_different_file);
    RUN_TEST(test_fill_different_block);
    RUN_TEST(test_fill_formula_verification);
    RUN_TEST(test_fill_zero_indices);
    RUN_TEST(test_fill_large_indices);

    printf("\n--- f3v_verify_pattern() Tests ---\n");
    RUN_TEST(test_verify_perfect_match);
    RUN_TEST(test_verify_single_corruption);
    RUN_TEST(test_verify_multiple_corruption);
    RUN_TEST(test_verify_first_error_offset);
    RUN_TEST(test_verify_null_offset);
    RUN_TEST(test_verify_full_corruption);
    RUN_TEST(test_verify_first_byte_corruption);
    RUN_TEST(test_verify_last_byte_corruption);
    RUN_TEST(test_verify_wrong_file_index);
    RUN_TEST(test_verify_wrong_block_index);

    /* Summary */
    printf("\n=== Results: %d/%d passed ===\n", g_tests_passed, g_tests_run);

    if (g_tests_failed > 0)
    {
        printf("FAILED: %d test(s)\n", g_tests_failed);
        return 1;
    }

    printf("All tests passed!\n");
    return 0;
}