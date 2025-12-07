/**
 * @file main.c
 * @brief f3vita - Storage Verification Tool for PS Vita
 *
 * Entry point and state machine for the application.
 */

#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/ctrl.h>
#include <string.h>
#include <stdio.h>

#include "types.h"
#include "storage.h"
#include "pattern.h"
#include "ui.h"

/* Global state */
static AppState g_state = STATE_MENU;
static TestContext g_ctx;
static StorageDevice g_devices[F3V_MAX_DEVICES];
static int g_device_count = 0;
static int g_selected_device = 0;

/* Work buffer for I/O (1 MB) - static to avoid stack issues */
static uint8_t g_buffer[F3V_BLOCK_SIZE] __attribute__((aligned(64)));

/* Forward declarations */
static void state_menu(void);
static void state_write(void);
static void state_verify(void);
static void state_results(void);
static void state_cleanup(void);

/**
 * Application entry point
 */
int main(void)
{
    /* Initialize UI (debug screen) */
    f3v_ui_init();

    /* Enumerate storage devices */
    g_device_count = f3v_enumerate_storage(g_devices, F3V_MAX_DEVICES);

    /* Clear context */
    memset(&g_ctx, 0, sizeof(g_ctx));

    /* Main loop */
    while (g_state != STATE_EXIT)
    {
        /* Prevent screen dimming and auto-sleep */
        sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DEFAULT);

        /* Clear screen each frame */
        f3v_ui_clear();

        /* Process current state */
        switch (g_state)
        {
        case STATE_MENU:
            state_menu();
            break;
        case STATE_WRITE:
            state_write();
            break;
        case STATE_VERIFY:
            state_verify();
            break;
        case STATE_RESULTS:
            state_results();
            break;
        case STATE_CLEANUP:
            state_cleanup();
            break;
        default:
            g_state = STATE_EXIT;
            break;
        }

        /* Swap buffers */
        f3v_ui_swap();
    }

    /* Clean exit */
    sceKernelExitProcess(0);
    return 0;
}

/**
 * Storage selection menu state
 */
static void state_menu(void)
{
    /* Draw header and menu */
    f3v_ui_header("f3vita - Storage Verification");

    if (g_device_count == 0)
    {
        f3v_ui_error("No writable storage found!");
        f3v_ui_prompt("Press O to exit");

        uint32_t btn = f3v_ui_read_buttons();
        if (btn & F3V_BTN_CIRCLE)
        {
            g_state = STATE_EXIT;
        }
        return;
    }

    f3v_ui_menu(g_devices, g_device_count, g_selected_device);
    f3v_ui_prompt("D-Pad: Select | X: Start Test | O: Exit");

    /* Handle input */
    uint32_t btn = f3v_ui_read_buttons();

    if (btn & F3V_BTN_UP)
    {
        if (g_selected_device > 0)
        {
            g_selected_device--;
        }
    }
    if (btn & F3V_BTN_DOWN)
    {
        if (g_selected_device < g_device_count - 1)
        {
            g_selected_device++;
        }
    }
    if (btn & F3V_BTN_CROSS)
    {
        /* Start test on selected device */
        memset(&g_ctx, 0, sizeof(g_ctx));
        g_ctx.target = g_devices[g_selected_device];

        /* Create test directory */
        if (f3v_create_test_dir(&g_ctx) < 0)
        {
            f3v_ui_error("Failed to create test directory!");
            f3v_ui_wait_button(F3V_BTN_ANY);
            return;
        }

        /* Initialize test */
        g_ctx.total_expected = g_ctx.target.free_bytes;
        g_ctx.start_time = f3v_get_time_usec();
        g_ctx.phase_start_time = g_ctx.start_time;
        g_ctx.files_written = 0;
        g_ctx.bytes_written = 0;

        g_state = STATE_WRITE;
    }
    if (btn & F3V_BTN_CIRCLE)
    {
        g_state = STATE_EXIT;
    }
}

/**
 * Write phase state - write test patterns to storage
 */
static void state_write(void)
{
    uint32_t elapsed = (uint32_t)((f3v_get_time_usec() - g_ctx.phase_start_time) / 1000000);

    /* Draw progress */
    f3v_ui_header("f3vita - Writing");
    f3v_ui_progress("WRITE",
                    g_ctx.bytes_written / (1024 * 1024),
                    g_ctx.total_expected / (1024 * 1024),
                    0, elapsed);
    f3v_ui_prompt("Press O to cancel");

    /* Check for cancel */
    uint32_t btn = f3v_ui_read_buttons();
    if (btn & F3V_BTN_CIRCLE)
    {
        g_ctx.cancelled = 1;
        g_state = STATE_RESULTS;
        return;
    }

    /* Check if we have space */
    if (!f3v_has_space(&g_ctx))
    {
        /* Disk full - transition to verify */
        g_ctx.phase_start_time = f3v_get_time_usec();
        g_ctx.current_file = 1;
        g_ctx.current_block = 0;
        g_ctx.bytes_verified = 0;
        g_state = STATE_VERIFY;
        return;
    }

    /* Calculate current file and block */
    uint32_t file_idx = (uint32_t)(g_ctx.bytes_written / F3V_FILE_SIZE) + 1;
    uint32_t block_idx = (uint32_t)((g_ctx.bytes_written % F3V_FILE_SIZE) / F3V_BLOCK_SIZE);

    /* Open new file if needed */
    static int fd = -1;
    static uint32_t current_file_idx = 0;

    if (file_idx != current_file_idx)
    {
        /* Close previous file if open */
        if (fd >= 0)
        {
            f3v_close(fd);
        }

        /* Open new file */
        char filename[128];
        f3v_get_test_filename(&g_ctx, file_idx, filename, sizeof(filename));
        fd = f3v_open_write(filename);

        if (fd < 0)
        {
            /* Write error - transition to verify */
            g_ctx.phase_start_time = f3v_get_time_usec();
            g_ctx.current_file = 1;
            g_ctx.current_block = 0;
            g_ctx.bytes_verified = 0;
            g_state = STATE_VERIFY;
            return;
        }

        current_file_idx = file_idx;
        g_ctx.files_written = file_idx;
    }

    /* Generate pattern for this block */
    f3v_fill_pattern(g_buffer, file_idx, block_idx);

    /* Write block */
    int written = f3v_write_block(fd, g_buffer, F3V_BLOCK_SIZE);

    if (written <= 0)
    {
        /* Write error or disk full */
        f3v_close(fd);
        fd = -1;
        current_file_idx = 0;

        g_ctx.phase_start_time = f3v_get_time_usec();
        g_ctx.current_file = 1;
        g_ctx.current_block = 0;
        g_ctx.bytes_verified = 0;
        g_state = STATE_VERIFY;
        return;
    }

    g_ctx.bytes_written += written;
}

/**
 * Verify phase state - read back and verify test patterns
 */
static void state_verify(void)
{
    uint32_t elapsed = (uint32_t)((f3v_get_time_usec() - g_ctx.phase_start_time) / 1000000);

    /* Draw progress */
    f3v_ui_header("f3vita - Verifying");
    f3v_ui_progress("VERIFY",
                    g_ctx.bytes_verified / (1024 * 1024),
                    g_ctx.bytes_written / (1024 * 1024),
                    g_ctx.bytes_corrupted, elapsed);
    f3v_ui_prompt("Press O to cancel");

    /* Check for cancel */
    uint32_t btn = f3v_ui_read_buttons();
    if (btn & F3V_BTN_CIRCLE)
    {
        g_ctx.cancelled = 1;
        g_state = STATE_RESULTS;
        return;
    }

    /* Check if verification complete */
    if (g_ctx.bytes_verified >= g_ctx.bytes_written)
    {
        g_ctx.end_time = f3v_get_time_usec();
        g_state = STATE_RESULTS;
        return;
    }

    /* Calculate current file and block */
    uint32_t file_idx = (uint32_t)(g_ctx.bytes_verified / F3V_FILE_SIZE) + 1;
    uint32_t block_idx = (uint32_t)((g_ctx.bytes_verified % F3V_FILE_SIZE) / F3V_BLOCK_SIZE);

    /* Open file if needed */
    static int fd = -1;
    static uint32_t current_file_idx = 0;

    if (file_idx != current_file_idx)
    {
        /* Close previous file if open */
        if (fd >= 0)
        {
            f3v_close(fd);
        }

        /* Open file for reading */
        char filename[128];
        f3v_get_test_filename(&g_ctx, file_idx, filename, sizeof(filename));
        fd = f3v_open_read(filename);

        if (fd < 0)
        {
            /* Read error - count entire remaining file as corrupted */
            uint64_t remaining = g_ctx.bytes_written - g_ctx.bytes_verified;
            g_ctx.bytes_corrupted += remaining;
            g_ctx.bytes_verified = g_ctx.bytes_written;

            if (!g_ctx.has_first_error)
            {
                g_ctx.has_first_error = 1;
                g_ctx.first_error_file = file_idx;
                g_ctx.first_error_block = block_idx;
                g_ctx.first_error_offset = 0;
            }

            g_ctx.end_time = f3v_get_time_usec();
            g_state = STATE_RESULTS;
            return;
        }

        current_file_idx = file_idx;
    }

    /* Read block */
    int bytes_read = f3v_read_block(fd, g_buffer, F3V_BLOCK_SIZE);

    if (bytes_read <= 0)
    {
        /* Read error - count as corrupted */
        g_ctx.bytes_corrupted += F3V_BLOCK_SIZE;
        g_ctx.bytes_verified += F3V_BLOCK_SIZE;

        if (!g_ctx.has_first_error)
        {
            g_ctx.has_first_error = 1;
            g_ctx.first_error_file = file_idx;
            g_ctx.first_error_block = block_idx;
            g_ctx.first_error_offset = 0;
        }
        return;
    }

    /* Verify pattern */
    uint32_t first_offset = 0;
    uint32_t corrupted = f3v_verify_pattern(g_buffer, file_idx, block_idx, &first_offset);

    if (corrupted > 0)
    {
        g_ctx.bytes_corrupted += corrupted;

        if (!g_ctx.has_first_error)
        {
            g_ctx.has_first_error = 1;
            g_ctx.first_error_file = file_idx;
            g_ctx.first_error_block = block_idx;
            g_ctx.first_error_offset = first_offset;
        }
    }

    g_ctx.bytes_verified += bytes_read;
}

/**
 * Results state - display test results
 */
static void state_results(void)
{
    TestResult result;

    if (g_ctx.cancelled)
    {
        result = RESULT_CANCELLED;
    }
    else if (g_ctx.bytes_corrupted > 0)
    {
        result = RESULT_FAIL;
    }
    else
    {
        result = RESULT_PASS;
    }

    f3v_ui_header("f3vita - Results");
    f3v_ui_results(&g_ctx, result);
    f3v_ui_prompt("X: Clean up files | O: Keep files & exit");

    uint32_t btn = f3v_ui_read_buttons();

    if (btn & F3V_BTN_CROSS)
    {
        g_ctx.cleanup_requested = 1;
        g_state = STATE_CLEANUP;
    }
    if (btn & F3V_BTN_CIRCLE)
    {
        g_ctx.cleanup_requested = 0;
        g_state = STATE_EXIT;
    }
}

/**
 * Cleanup state - delete test files
 */
static void state_cleanup(void)
{
    f3v_ui_header("f3vita - Cleaning Up");
    f3v_ui_prompt("Deleting test files...");
    f3v_ui_swap(); /* Show message immediately */

    int deleted = f3v_cleanup_files(&g_ctx);

    f3v_ui_clear();
    f3v_ui_header("f3vita - Cleanup Complete");

    char msg[64];
    if (deleted > 0)
    {
        snprintf(msg, sizeof(msg), "Deleted %d test file(s)", deleted);
    }
    else
    {
        snprintf(msg, sizeof(msg), "No files to delete");
    }
    f3v_ui_prompt(msg);

    f3v_ui_wait_button(F3V_BTN_ANY);
    g_state = STATE_EXIT;
}