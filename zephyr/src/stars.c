// --- starfield ---

#include "stars.h"
#include "graph.h"

#define NUM_STARS   200
#define NUM_LAYERS  3

static float star_x[NUM_STARS];
static float star_y[NUM_STARS];
static uint8_t star_layer[NUM_STARS];

static const float layer_speed[NUM_LAYERS] = { 20.0f, 50.0f, 90.0f };
static uint16_t layer_color[NUM_LAYERS];

// simple LCG PRNG for star positions
static uint32_t star_rng = 54321;

static float star_randf(void)
{
    star_rng = star_rng * 1664525u + 1013904223u;
    return (float)(star_rng >> 16) / 65536.0f;
}

// pre-compute layer colors and randomize star positions
void stars_init(void)
{
    for (int l = 0; l < NUM_LAYERS; l++) {
        int b = (l + 1) * 80;
        if (b > 255) b = 255;
        layer_color[l] = rgb565(b, b, b);
    }
    for (int i = 0; i < NUM_STARS; i++) {
        star_x[i] = star_randf() * SCREEN_W;
        star_y[i] = star_randf() * HORIZON_Y;
        star_layer[i] = i % NUM_LAYERS;
    }
}

// scroll stars left at layer-dependent speeds, wrap around
void stars_update(float dt)
{
    for (int i = 0; i < NUM_STARS; i++) {
        int layer = star_layer[i];
        star_x[i] -= layer_speed[layer] * dt;
        if (star_x[i] < 0.0f) {
            star_x[i] += SCREEN_W;
            star_y[i] = star_randf() * HORIZON_Y;
        }
    }
}

// render all stars as single pixels
void stars_draw(void)
{
    for (int i = 0; i < NUM_STARS; i++) {
        putpixel((int)star_x[i], (int)star_y[i], layer_color[star_layer[i]]);
    }
}
