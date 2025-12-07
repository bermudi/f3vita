/**
 * @file types.h
 * @brief Common types and constants for f3vita
 */

#ifndef F3VITA_TYPES_H
#define F3VITA_TYPES_H

#include <stdint.h>
#include <stddef.h>

/* Constants */
#define F3V_BLOCK_SIZE      (1024 * 1024)       /* 1 MB blocks */
#define F3V_FILE_SIZE       (1024 * 1024 * 1024) /* 1 GB per file */
#define F3V_BLOCKS_PER_FILE (F3V_FILE_SIZE / F3V_BLOCK_SIZE)
#define F3V_MAX_DEVICES     8
#define F3V_TEST_DIR        "data/f3vita"
#define F3V_FILE_PREFIX     "f3vita_"
#define F3V_FILE_EXT        ".dat"

/* Application states */
typedef enum {
    STATE_MENU,     /* Storage selection */
    STATE_WRITE,    /* Writing test files */
    STATE_VERIFY,   /* Reading and verifying */
    STATE_RESULTS,  /* Showing summary */
    STATE_CLEANUP,  /* Deleting files */
    STATE_EXIT      /* Clean exit */
} AppState;

/* Storage device info */
typedef struct {
    char path[16];          /* "ux0:", "uma0:", etc. */
    char name[32];          /* Human-readable name */
    uint64_t total_bytes;
    uint64_t free_bytes;
    int writable;           /* 1 if writable, 0 otherwise */
} StorageDevice;

/* Test context tracking all state */
typedef struct {
    /* Target storage */
    StorageDevice target;
    char test_dir[64];      /* Full path to test directory */
    
    /* Write phase tracking */
    uint32_t files_written;
    uint32_t current_file_blocks;
    uint64_t bytes_written;
    uint64_t total_expected;
    
    /* Verify phase tracking */
    uint32_t current_file;
    uint32_t current_block;
    uint64_t bytes_verified;
    uint64_t bytes_corrupted;
    
    /* First error location */
    int has_first_error;
    uint32_t first_error_file;
    uint32_t first_error_block;
    uint32_t first_error_offset;
    
    /* Timing (microseconds since epoch) */
    uint64_t start_time;
    uint64_t phase_start_time;
    uint64_t end_time;
    
    /* User preferences */
    int cleanup_requested;
    int cancelled;
} TestContext;

/* Test result */
typedef enum {
    RESULT_UNKNOWN,
    RESULT_PASS,
    RESULT_FAIL,
    RESULT_CANCELLED
} TestResult;

#endif /* F3VITA_TYPES_H */