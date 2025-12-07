/**
 * @file pattern.c
 * @brief Test pattern generation and verification
 */

#include "pattern.h"

void f3v_fill_pattern(uint8_t *buf, uint32_t file_idx, uint32_t block_idx)
{
    /*
     * Pattern formula: (file_index << 24) ^ (block_index << 16) ^ byte_offset
     *
     * This creates a unique 32-bit pattern for each location:
     * - file_idx occupies bits 31-24 (max 256 files = 256GB)
     * - block_idx occupies bits 23-16 (max 256 blocks per file = 256MB, but we use 1024)
     * - byte_offset occupies bits 15-0 (max 65536, we cycle within 1MB block)
     */
    uint32_t base = (file_idx << 24) ^ (block_idx << 16);

    /* Fill buffer with deterministic pattern */
    for (uint32_t i = 0; i < F3V_BLOCK_SIZE; i++)
    {
        /*
         * We take the XOR of base and offset, then extract a single byte.
         * Using different byte positions for different offsets ensures
         * we catch various bit-flip patterns.
         */
        uint32_t val = base ^ i;

        /* Rotate through different byte positions for variety */
        buf[i] = (uint8_t)(val >> ((i & 3) * 8));
    }
}

uint32_t f3v_verify_pattern(const uint8_t *buf, uint32_t file_idx, uint32_t block_idx,
                            uint32_t *first_error_offset)
{
    uint32_t base = (file_idx << 24) ^ (block_idx << 16);
    uint32_t corrupted = 0;
    int found_first = 0;

    for (uint32_t i = 0; i < F3V_BLOCK_SIZE; i++)
    {
        uint32_t val = base ^ i;
        uint8_t expected = (uint8_t)(val >> ((i & 3) * 8));

        if (buf[i] != expected)
        {
            corrupted++;

            if (!found_first && first_error_offset != NULL)
            {
                *first_error_offset = i;
                found_first = 1;
            }
        }
    }

    return corrupted;
}