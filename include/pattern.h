/**
 * @file pattern.h
 * @brief Test pattern generation and verification
 */

#ifndef F3VITA_PATTERN_H
#define F3VITA_PATTERN_H

#include "types.h"

/**
 * Fill a buffer with the test pattern for a specific block
 *
 * Pattern formula: Each byte = (file_index << 24) ^ (block_index << 16) ^ (byte_offset & 0xFFFF)
 * This ensures each location has a unique deterministic value.
 *
 * @param buf Buffer to fill (must be F3V_BLOCK_SIZE bytes)
 * @param file_idx File index (1-based)
 * @param block_idx Block index within file (0-based)
 */
void f3v_fill_pattern(uint8_t *buf, uint32_t file_idx, uint32_t block_idx);

/**
 * Verify a buffer against the expected pattern
 *
 * @param buf Buffer to verify (must be F3V_BLOCK_SIZE bytes)
 * @param file_idx File index (1-based)
 * @param block_idx Block index within file (0-based)
 * @param first_error_offset Output: offset of first mismatched byte (if any)
 * @return Number of corrupted bytes (0 = perfect match)
 */
uint32_t f3v_verify_pattern(const uint8_t *buf, uint32_t file_idx, uint32_t block_idx,
                            uint32_t *first_error_offset);

#endif /* F3VITA_PATTERN_H */