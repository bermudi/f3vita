/**
 * @file storage.h
 * @brief Storage enumeration and file I/O operations
 */

#ifndef F3VITA_STORAGE_H
#define F3VITA_STORAGE_H

#include "types.h"

/**
 * Enumerate available storage devices
 * @param devices Array to fill with device info
 * @param max_devices Maximum number of devices to detect
 * @return Number of devices found
 */
int f3v_enumerate_storage(StorageDevice *devices, int max_devices);

/**
 * Get storage info (free/total space) for a device
 * @param device Device to query (path must be set)
 * @return 0 on success, negative on error
 */
int f3v_get_storage_info(StorageDevice *device);

/**
 * Create the test directory on target storage
 * @param ctx Test context (target.path must be set)
 * @return 0 on success, negative on error
 */
int f3v_create_test_dir(TestContext *ctx);

/**
 * Generate test filename for given index
 * @param ctx Test context
 * @param index File index (1-based)
 * @param buf Output buffer
 * @param buf_size Buffer size
 * @return Pointer to buf
 */
char *f3v_get_test_filename(TestContext *ctx, uint32_t index, char *buf, size_t buf_size);

/**
 * Open test file for writing
 * @param path Full path to file
 * @return File descriptor or negative on error
 */
int f3v_open_write(const char *path);

/**
 * Open test file for reading
 * @param path Full path to file
 * @return File descriptor or negative on error
 */
int f3v_open_read(const char *path);

/**
 * Write block to file
 * @param fd File descriptor
 * @param buf Buffer to write
 * @param size Number of bytes to write
 * @return Bytes written or negative on error
 */
int f3v_write_block(int fd, const void *buf, size_t size);

/**
 * Read block from file
 * @param fd File descriptor
 * @param buf Buffer to read into
 * @param size Number of bytes to read
 * @return Bytes read or negative on error
 */
int f3v_read_block(int fd, void *buf, size_t size);

/**
 * Close file
 * @param fd File descriptor
 * @return 0 on success, negative on error
 */
int f3v_close(int fd);

/**
 * Delete all test files
 * @param ctx Test context
 * @return Number of files deleted
 */
int f3v_cleanup_files(TestContext *ctx);

/**
 * Check if there's enough space to write (at least 1 block)
 * @param ctx Test context
 * @return 1 if space available, 0 if full
 */
int f3v_has_space(TestContext *ctx);

#endif /* F3VITA_STORAGE_H */