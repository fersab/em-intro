// --- fire effect ---

#include "fire.h"
#include "graph.h"
#include <string.h>

#define FIRE_H_SIZE  42
#define FIRE_TOP_Y   (HORIZON_Y - 40)

static uint8_t fire_buf[SCREEN_W * FIRE_H_SIZE];
static uint16_t fire_pal[256];

// simple LCG PRNG for fire seeding
static uint32_t fire_rng = 12345;

static inline uint8_t fire_rand_byte(void)
{
    fire_rng = fire_rng * 1664525u + 1013904223u;
    return (uint8_t)(fire_rng >> 16);
}

// build fire palette: black -> red -> yellow -> white (256 entries)
void fire_init(void)
{
    for (int i = 0; i < 64; i++)
        fire_pal[i] = rgb565(i * 4, 0, 0);
    for (int i = 64; i < 128; i++)
        fire_pal[i] = rgb565(255, (i - 64) * 4, 0);
    for (int i = 128; i < 192; i++)
        fire_pal[i] = rgb565(255, 255, (i - 128) * 4);
    for (int i = 192; i < 256; i++)
        fire_pal[i] = rgb565(255, 255, 255);

    memset(fire_buf, 0, sizeof(fire_buf));
}

// one step of fire simulation: seed bottom rows, average and propagate up
void fire_update(void)
{
    // seed bottom two rows
    for (int x = 0; x < SCREEN_W; x++) {
        fire_buf[(FIRE_H_SIZE - 1) * SCREEN_W + x] = fire_rand_byte();
        fire_buf[(FIRE_H_SIZE - 2) * SCREEN_W + x] = fire_rand_byte();
    }

    // average 4 neighbors, propagate upward
    for (int y = 0; y < FIRE_H_SIZE - 2; y++) {
        for (int x = 0; x < SCREEN_W; x++) {
            int xl = (x > 0) ? x - 1 : SCREEN_W - 1;
            int xr = (x < SCREEN_W - 1) ? x + 1 : 0;

            int sum = fire_buf[(y + 1) * SCREEN_W + xl]
                    + fire_buf[(y + 1) * SCREEN_W + x]
                    + fire_buf[(y + 1) * SCREEN_W + xr]
                    + fire_buf[(y + 2) * SCREEN_W + x];

            int val = (sum >> 2) - 4;
            if (val < 0) val = 0;
            fire_buf[y * SCREEN_W + x] = (uint8_t)val;
        }
    }
}

// render fire buffer to framebuffer using the heat palette
void fire_draw(void)
{
    for (int y = 0; y < FIRE_H_SIZE - 4; y++) {
        int sy = FIRE_TOP_Y + y;
        if ((unsigned)sy >= SCREEN_H) continue;
        int fb_row = sy * SCREEN_W;
        int fire_row = y * SCREEN_W;
        for (int x = 0; x < SCREEN_W; x++) {
            uint8_t heat = fire_buf[fire_row + x];
            if (heat > 0) {
                fb_back[fb_row + x] = fire_pal[heat];
            }
        }
    }
}
