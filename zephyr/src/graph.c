// --- screen setup and framebuffer (display via Zephyr display API) ---

#include "graph.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <string.h>

// two framebuffers for double buffering
// placed in SDRAM via linker — see board overlay
static uint16_t fb0[SCREEN_W * SCREEN_H] __attribute__((section(".sdram_bss")));
static uint16_t fb1[SCREEN_W * SCREEN_H] __attribute__((section(".sdram_bss")));

static uint16_t *buffers[2] = { fb0, fb1 };
static int back_idx = 0;
uint16_t *fb_back = NULL;

static const struct device *display_dev;
static int display_x_offset = 0;
static int display_y_offset = 0;

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

    // swap back buffer
    back_idx ^= 1;
    fb_back = buffers[back_idx];
}
