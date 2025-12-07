/**
 * @file ui.h
 * @brief UI display and input handling using debug screen
 */

#ifndef F3VITA_UI_H
#define F3VITA_UI_H

#include "types.h"

/* Button masks */
#define F3V_BTN_CROSS (1 << 0)  /* X / Confirm */
#define F3V_BTN_CIRCLE (1 << 1) /* O / Cancel */
#define F3V_BTN_UP (1 << 2)
#define F3V_BTN_DOWN (1 << 3)
#define F3V_BTN_LEFT (1 << 4)
#define F3V_BTN_RIGHT (1 << 5)
#define F3V_BTN_START (1 << 6)
#define F3V_BTN_ANY (0xFF)

/**
 * Initialize the debug screen
 */
void f3v_ui_init(void);

/**
 * Clear the screen
 */
void f3v_ui_clear(void);

/**
 * Draw the app header
 * @param title Title to display
 */
void f3v_ui_header(const char *title);

/**
 * Draw storage selection menu
 * @param devices Array of storage devices
 * @param count Number of devices
 * @param selected Currently selected index
 */
void f3v_ui_menu(const StorageDevice *devices, int count, int selected);

/**
 * Draw progress display
 * @param phase "WRITE" or "VERIFY"
 * @param current_mb Current MB processed
 * @param total_mb Total MB to process
 * @param errors Number of errors found (verify only)
 * @param elapsed_secs Elapsed time in seconds
 */
void f3v_ui_progress(const char *phase, uint64_t current_mb, uint64_t total_mb,
                     uint64_t errors, uint32_t elapsed_secs);

/**
 * Draw results screen
 * @param ctx Test context with results
 * @param result Test result (PASS/FAIL/CANCELLED)
 */
void f3v_ui_results(const TestContext *ctx, TestResult result);

/**
 * Draw a confirmation prompt
 * @param message Prompt message
 */
void f3v_ui_prompt(const char *message);

/**
 * Draw error message
 * @param message Error message
 */
void f3v_ui_error(const char *message);

/**
 * Read current button state (with debounce)
 * @return Bitmask of currently pressed buttons
 */
uint32_t f3v_ui_read_buttons(void);

/**
 * Wait for specific button(s) to be pressed
 * @param mask Button mask to wait for
 * @return Buttons that were pressed
 */
uint32_t f3v_ui_wait_button(uint32_t mask);

/**
 * Swap display buffers (end frame)
 */
void f3v_ui_swap(void);

/**
 * Get current time in microseconds
 * @return Microseconds since epoch
 */
uint64_t f3v_get_time_usec(void);

/**
 * Format bytes as human-readable (e.g., "1.5 GB")
 * @param bytes Byte count
 * @param buf Output buffer
 * @param buf_size Buffer size
 * @return Pointer to buf
 */
char *f3v_format_bytes(uint64_t bytes, char *buf, size_t buf_size);

/**
 * Format duration in seconds as HH:MM:SS
 * @param seconds Duration in seconds
 * @param buf Output buffer
 * @param buf_size Buffer size
 * @return Pointer to buf
 */
char *f3v_format_duration(uint32_t seconds, char *buf, size_t buf_size);

#endif /* F3VITA_UI_H */