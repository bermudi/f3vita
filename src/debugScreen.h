/**
 * @file debugScreen.h
 * @brief Simple debug screen implementation for PS Vita
 * Based on VitaSDK samples
 */

#ifndef DEBUG_SCREEN_H
#define DEBUG_SCREEN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Initialize the debug screen
     * Allocates framebuffer and sets up display
     */
    void psvDebugScreenInit(void);

    /**
     * Clear the screen with specified color
     * @param color 32-bit ABGR color
     */
    void psvDebugScreenClear(uint32_t color);

    /**
     * Set foreground (text) color
     * @param color 32-bit ABGR color
     */
    void psvDebugScreenSetFgColor(uint32_t color);

    /**
     * Set background color
     * @param color 32-bit ABGR color
     */
    void psvDebugScreenSetBgColor(uint32_t color);

    /**
     * Printf to the debug screen
     * @param fmt Format string
     * @return Number of characters printed
     */
    int psvDebugScreenPrintf(const char *fmt, ...);

    /**
     * Set cursor position
     * @param x Column (0-based)
     * @param y Row (0-based)
     */
    void psvDebugScreenSetXY(int x, int y);

    /**
     * Get current X position
     */
    int psvDebugScreenGetX(void);

    /**
     * Get current Y position
     */
    int psvDebugScreenGetY(void);

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_SCREEN_H */