// --- screen setup and framebuffer ---
// Zephyr build: display via Zephyr display API
// Emscripten build: blit RGB565 → RGBA32 to HTML canvas

#include "graph.h"
#include <string.h>

// ===== Platform-specific framebuffers and state =====

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>

static uint16_t fb0[SCREEN_W * SCREEN_H];
static uint16_t fb1[SCREEN_W * SCREEN_H];

// RGBA32 output buffer — JS reads this for canvas putImageData
static uint8_t canvas_buf[SCREEN_W * SCREEN_H * 4];

EMSCRIPTEN_KEEPALIVE
uint8_t *graph_get_canvas_buf(void) { return canvas_buf; }

#else
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

// two framebuffers placed in SDRAM via linker — see board overlay
static uint16_t fb0[SCREEN_W * SCREEN_H] SDRAM_BSS;
static uint16_t fb1[SCREEN_W * SCREEN_H] SDRAM_BSS;

static const struct device *display_dev;
static int display_x_offset = 0;
static int display_y_offset = 0;

#endif

// ===== Common double-buffer state =====

static uint16_t *buffers[2] = { fb0, fb1 };
static int back_idx = 0;
uint16_t *fb_back = NULL;

// ===== Platform-specific init =====

#ifdef __EMSCRIPTEN__

int graph_init(void)
{
    memset(fb0, 0, sizeof(fb0));
    memset(fb1, 0, sizeof(fb1));
    back_idx = 0;
    fb_back = buffers[back_idx];
    return 0;
}

#else

// initialize display device and framebuffers
int graph_init(void)
{
    display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) {
        return -1;
    }

    display_blanking_off(display_dev);

    // center our 480x272 buffer on the physical display
    struct display_capabilities caps;
    display_get_capabilities(display_dev, &caps);
    if (caps.x_resolution > SCREEN_W) {
        display_x_offset = (caps.x_resolution - SCREEN_W) / 2;
    }
    if (caps.y_resolution > SCREEN_H) {
        display_y_offset = (caps.y_resolution - SCREEN_H) / 2;
    }

    memset(fb0, 0, sizeof(fb0));
    memset(fb1, 0, sizeof(fb1));
    back_idx = 0;
    fb_back = buffers[back_idx];

    return 0;
}

#endif

// ===== Common framebuffer operations =====

// fill entire back buffer with a single color
void fb_clear(uint16_t color)
{
    if (color == 0) {
        memset(fb_back, 0, SCREEN_W * SCREEN_H * sizeof(uint16_t));
    } else {
        for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
            fb_back[i] = color;
        }
    }
}

// ===== Platform-specific swap =====

#ifdef __EMSCRIPTEN__

// expand RGB565 back buffer → RGBA32 canvas buffer, then swap
void fb_swap(void)
{
    const uint16_t *src = fb_back;
    uint8_t *dst = canvas_buf;
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        uint16_t c = src[i];
        dst[0] = (c >> 8) & 0xF8; dst[0] |= dst[0] >> 5;  // R
        dst[1] = (c >> 3) & 0xFC; dst[1] |= dst[1] >> 6;  // G
        dst[2] = (c << 3) & 0xF8; dst[2] |= dst[2] >> 5;  // B
        dst[3] = 255;                                        // A
        dst += 4;
    }

    back_idx ^= 1;
    fb_back = buffers[back_idx];
}

#else

// present back buffer to display, swap buffers
void fb_swap(void)
{
    struct display_buffer_descriptor desc = {
        .buf_size = SCREEN_W * SCREEN_H * 2,
        .width = SCREEN_W,
        .height = SCREEN_H,
        .pitch = SCREEN_W,
    };

    display_write(display_dev, display_x_offset, display_y_offset,
                  &desc, fb_back);

    back_idx ^= 1;
    fb_back = buffers[back_idx];
}

#endif
