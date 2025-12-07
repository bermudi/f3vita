/**
 * @file ui.c
 * @brief UI display and input handling using debug screen
 */

#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <psp2/rtc.h>
#include <psp2/kernel/threadmgr.h>
#include <stdio.h>
#include <string.h>

#include "ui.h"

/* Debug screen from VitaSDK samples */
#include <debugScreen.h>

/* Screen dimensions (debug screen uses fixed size) */
#define SCREEN_WIDTH 60
#define SCREEN_HEIGHT 34

/* Last button state for edge detection */
static uint32_t g_last_buttons = 0;

/* Init debug screen */
void f3v_ui_init(void)
{
    psvDebugScreenInit();

    /* Initialize controller */
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
}

void f3v_ui_clear(void)
{
    psvDebugScreenClear(0); /* Black background */
}

void f3v_ui_header(const char *title)
{
    psvDebugScreenSetFgColor(0xFF00FFFF); /* Cyan */
    psvDebugScreenPrintf("\n  %s\n", title);
    psvDebugScreenPrintf("  ");
    for (int i = 0; i < 56; i++)
        psvDebugScreenPrintf("-");
    psvDebugScreenPrintf("\n\n");
    psvDebugScreenSetFgColor(0xFFFFFFFF); /* White */
}

void f3v_ui_menu(const StorageDevice *devices, int count, int selected)
{
    psvDebugScreenPrintf("  Select storage device:\n\n");

    for (int i = 0; i < count; i++)
    {
        char free_str[32], total_str[32];
        f3v_format_bytes(devices[i].free_bytes, free_str, sizeof(free_str));
        f3v_format_bytes(devices[i].total_bytes, total_str, sizeof(total_str));

        if (i == selected)
        {
            psvDebugScreenSetFgColor(0xFF00FF00); /* Green for selected */
            psvDebugScreenPrintf("  > ");
        }
        else
        {
            psvDebugScreenSetFgColor(0xFFFFFFFF); /* White */
            psvDebugScreenPrintf("    ");
        }

        psvDebugScreenPrintf("%s (%s)\n", devices[i].path, devices[i].name);
        psvDebugScreenPrintf("      Free: %s / %s\n\n", free_str, total_str);
    }

    psvDebugScreenSetFgColor(0xFFFFFFFF);
}

void f3v_ui_progress(const char *phase, uint64_t current_mb, uint64_t total_mb,
                     uint64_t errors, uint32_t elapsed_secs)
{
    char time_str[32];
    f3v_format_duration(elapsed_secs, time_str, sizeof(time_str));

    /* Phase display */
    psvDebugScreenSetFgColor(0xFF00FFFF); /* Cyan */
    psvDebugScreenPrintf("  Phase: %s\n\n", phase);
    psvDebugScreenSetFgColor(0xFFFFFFFF);

    /* Progress bar */
    uint32_t percent = 0;
    if (total_mb > 0)
    {
        percent = (uint32_t)((current_mb * 100) / total_mb);
        if (percent > 100)
            percent = 100;
    }

    psvDebugScreenPrintf("  Progress: [");

    int bar_width = 40;
    int filled = (bar_width * percent) / 100;

    psvDebugScreenSetFgColor(0xFF00FF00); /* Green */
    for (int i = 0; i < filled; i++)
        psvDebugScreenPrintf("=");
    psvDebugScreenSetFgColor(0xFF888888); /* Gray */
    for (int i = filled; i < bar_width; i++)
        psvDebugScreenPrintf("-");
    psvDebugScreenSetFgColor(0xFFFFFFFF);

    psvDebugScreenPrintf("] %3u%%\n\n", percent);

    /* Statistics */
    psvDebugScreenPrintf("  Processed: %llu MB / %llu MB\n", current_mb, total_mb);
    psvDebugScreenPrintf("  Elapsed:   %s\n", time_str);

    if (strcmp(phase, "VERIFY") == 0)
    {
        if (errors > 0)
        {
            psvDebugScreenSetFgColor(0xFF0000FF); /* Red */
        }
        else
        {
            psvDebugScreenSetFgColor(0xFF00FF00); /* Green */
        }
        psvDebugScreenPrintf("  Errors:    %llu bytes\n", errors);
        psvDebugScreenSetFgColor(0xFFFFFFFF);
    }

    /* Speed calculation */
    if (elapsed_secs > 0)
    {
        uint64_t speed_mbps = current_mb / elapsed_secs;
        psvDebugScreenPrintf("  Speed:     %llu MB/s\n", speed_mbps);
    }

    psvDebugScreenPrintf("\n");
}

void f3v_ui_results(const TestContext *ctx, TestResult result)
{
    char bytes_str[32], corrupt_str[32], time_str[32];
    uint32_t total_secs = (uint32_t)((ctx->end_time - ctx->start_time) / 1000000);

    f3v_format_bytes(ctx->bytes_written, bytes_str, sizeof(bytes_str));
    f3v_format_bytes(ctx->bytes_corrupted, corrupt_str, sizeof(corrupt_str));
    f3v_format_duration(total_secs, time_str, sizeof(time_str));

    /* Result status */
    switch (result)
    {
    case RESULT_PASS:
        psvDebugScreenSetFgColor(0xFF00FF00); /* Green */
        psvDebugScreenPrintf("  Status: PASS\n\n");
        break;
    case RESULT_FAIL:
        psvDebugScreenSetFgColor(0xFF0000FF); /* Red */
        psvDebugScreenPrintf("  Status: FAIL\n\n");
        break;
    case RESULT_CANCELLED:
        psvDebugScreenSetFgColor(0xFF00FFFF); /* Yellow */
        psvDebugScreenPrintf("  Status: CANCELLED\n\n");
        break;
    default:
        psvDebugScreenPrintf("  Status: UNKNOWN\n\n");
        break;
    }
    psvDebugScreenSetFgColor(0xFFFFFFFF);

    /* Statistics */
    psvDebugScreenPrintf("  Data Written:  %s (%u files)\n", bytes_str, ctx->files_written);
    psvDebugScreenPrintf("  Data Verified: %llu MB\n", ctx->bytes_verified / (1024 * 1024));
    psvDebugScreenPrintf("  Total Time:    %s\n\n", time_str);

    if (ctx->bytes_corrupted > 0)
    {
        psvDebugScreenSetFgColor(0xFF0000FF); /* Red */
        psvDebugScreenPrintf("  Corrupted:     %s\n", corrupt_str);
        psvDebugScreenSetFgColor(0xFFFFFFFF);

        if (ctx->has_first_error)
        {
            psvDebugScreenPrintf("  First Error:   File %03u, Block %u, Offset %u\n",
                                 ctx->first_error_file, ctx->first_error_block,
                                 ctx->first_error_offset);
        }
    }
    else if (result == RESULT_PASS)
    {
        psvDebugScreenSetFgColor(0xFF00FF00); /* Green */
        psvDebugScreenPrintf("  No corruption detected!\n");
        psvDebugScreenSetFgColor(0xFFFFFFFF);
    }

    psvDebugScreenPrintf("\n");
}

void f3v_ui_prompt(const char *message)
{
    psvDebugScreenSetFgColor(0xFF888888); /* Gray */
    psvDebugScreenPrintf("\n  %s\n", message);
    psvDebugScreenSetFgColor(0xFFFFFFFF);
}

void f3v_ui_error(const char *message)
{
    psvDebugScreenSetFgColor(0xFF0000FF); /* Red */
    psvDebugScreenPrintf("\n  ERROR: %s\n", message);
    psvDebugScreenSetFgColor(0xFFFFFFFF);
}

uint32_t f3v_ui_read_buttons(void)
{
    SceCtrlData pad;
    sceCtrlPeekBufferPositive(0, &pad, 1);

    /* Convert SCE button bits to our format and detect edges */
    uint32_t current = 0;

    if (pad.buttons & SCE_CTRL_CROSS)
        current |= F3V_BTN_CROSS;
    if (pad.buttons & SCE_CTRL_CIRCLE)
        current |= F3V_BTN_CIRCLE;
    if (pad.buttons & SCE_CTRL_UP)
        current |= F3V_BTN_UP;
    if (pad.buttons & SCE_CTRL_DOWN)
        current |= F3V_BTN_DOWN;
    if (pad.buttons & SCE_CTRL_LEFT)
        current |= F3V_BTN_LEFT;
    if (pad.buttons & SCE_CTRL_RIGHT)
        current |= F3V_BTN_RIGHT;
    if (pad.buttons & SCE_CTRL_START)
        current |= F3V_BTN_START;

    /* Return newly pressed buttons (edge detection) */
    uint32_t pressed = current & ~g_last_buttons;
    g_last_buttons = current;

    return pressed;
}

uint32_t f3v_ui_wait_button(uint32_t mask)
{
    uint32_t btn;

    /* Wait for release first */
    do
    {
        SceCtrlData pad;
        sceCtrlPeekBufferPositive(0, &pad, 1);
        sceKernelDelayThread(50000); /* 50ms */
    } while (g_last_buttons != 0);

    /* Wait for press */
    do
    {
        btn = f3v_ui_read_buttons();
        sceKernelDelayThread(50000); /* 50ms */
    } while ((btn & mask) == 0);

    return btn & mask;
}

void f3v_ui_swap(void)
{
    sceDisplayWaitVblankStart();
}

uint64_t f3v_get_time_usec(void)
{
    SceRtcTick tick;
    sceRtcGetCurrentTick(&tick);
    return tick.tick;
}

char *f3v_format_bytes(uint64_t bytes, char *buf, size_t buf_size)
{
    if (bytes >= (1024ULL * 1024 * 1024))
    {
        snprintf(buf, buf_size, "%.2f GB", (double)bytes / (1024.0 * 1024.0 * 1024.0));
    }
    else if (bytes >= (1024 * 1024))
    {
        snprintf(buf, buf_size, "%.2f MB", (double)bytes / (1024.0 * 1024.0));
    }
    else if (bytes >= 1024)
    {
        snprintf(buf, buf_size, "%.2f KB", (double)bytes / 1024.0);
    }
    else
    {
        snprintf(buf, buf_size, "%llu B", bytes);
    }
    return buf;
}

char *f3v_format_duration(uint32_t seconds, char *buf, size_t buf_size)
{
    uint32_t hours = seconds / 3600;
    uint32_t mins = (seconds % 3600) / 60;
    uint32_t secs = seconds % 60;

    if (hours > 0)
    {
        snprintf(buf, buf_size, "%u:%02u:%02u", hours, mins, secs);
    }
    else
    {
        snprintf(buf, buf_size, "%u:%02u", mins, secs);
    }
    return buf;
}