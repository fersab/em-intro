#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>

// place large buffers in external SDRAM on Zephyr, regular memory elsewhere
#if defined(__EMSCRIPTEN__) || defined(SYNTAX_CHECK)
#define SDRAM_BSS
#else
#define SDRAM_BSS __attribute__((section(".sdram_bss")))
#endif

// --- resolution (change these!) ---
#define SCREEN_W  480
#define SCREEN_H  272
#define HORIZON_Y (SCREEN_H * 3 / 4)

// current back buffer — effects write pixels here
extern uint16_t *fb_back;

// initialize display device and framebuffers
int graph_init(void);

// pack 8-bit RGB into 16-bit RGB565
static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// write one RGB565 pixel to back buffer with bounds check
static inline void putpixel(int x, int y, uint16_t color)
{
    if ((unsigned)x < SCREEN_W && (unsigned)y < SCREEN_H) {
        fb_back[y * SCREEN_W + x] = color;
    }
}

// fill entire back buffer with a single color
void fb_clear(uint16_t color);

// present back buffer to display, swap buffers
void fb_swap(void);

#ifdef __EMSCRIPTEN__
// get pointer to RGBA32 canvas buffer (Emscripten only)
uint8_t *graph_get_canvas_buf(void);
#endif

#endif
