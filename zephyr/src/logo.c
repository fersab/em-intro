// --- logo particle animation ---

#include "logo.h"
#include "graph.h"
#include "sprite.h"
#include "logo_data.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define LOGO_ANIM_TIME      4.0f
#define LOGO_WAVE_TIME      2.5f
#define LOGO_WAVE_MAX       15.0f
#define LOGO_TOTAL_TIME     (LOGO_ANIM_TIME + LOGO_WAVE_TIME)

// periodic shake
#define LOGO_SHAKE_INTERVAL 10.0f
#define LOGO_SHAKE_DURATION 0.8f
#define LOGO_SHAKE_AMP      6.0f
#define LOGO_SHAKE_FREQ     0.15f   // spatial frequency of shake sine
#define LOGO_SHAKE_SPEED    8.0f    // temporal speed of shake sine

// display scale
#define LOGO_SCALE          0.5f    // fraction of screen width
#define LOGO_Y_OFFSET       20      // pixels below vertical center

// convergence wave tuning
#define LOGO_CONV_SPEED     4.0f    // temporal speed of convergence wave
#define LOGO_CONV_FREQ      0.05f   // spatial frequency of convergence wave
#define LOGO_CONV_DITHER    0.001f  // per-particle phase offset

static sprite_t logo_sprite;
static bool logo_ready = false;
static float logo_time = 0.0f;

// display dimensions
static int logo_dw = 0;
static int logo_dh = 0;
static int logo_dx = 0;
static int logo_dy = 0;

// particle arrays in SDRAM (struct-of-arrays)
#define MAX_LOGO_PARTICLES 40000

static float logo_sx[MAX_LOGO_PARTICLES] __attribute__((section(".sdram_bss")));
static float logo_sy[MAX_LOGO_PARTICLES] __attribute__((section(".sdram_bss")));
static float logo_ex[MAX_LOGO_PARTICLES] __attribute__((section(".sdram_bss")));
static float logo_ey[MAX_LOGO_PARTICLES] __attribute__((section(".sdram_bss")));
static uint16_t logo_color[MAX_LOGO_PARTICLES] __attribute__((section(".sdram_bss")));

static int logo_num = 0;

// scale logo, extract non-transparent pixels into particle arrays
static void logo_build_particles(void)
{
    logo_dw = (int)(SCREEN_W * LOGO_SCALE);
    logo_dh = logo_dw * logo_sprite.h / logo_sprite.w;
    logo_dx = (SCREEN_W - logo_dw) / 2;
    logo_dy = (SCREEN_H - logo_dh) / 2 + LOGO_Y_OFFSET;

    // count non-transparent pixels at display resolution
    int count = 0;
    for (int y = 0; y < logo_dh; y++) {
        int src_y = y * logo_sprite.h / logo_dh;
        for (int x = 0; x < logo_dw; x++) {
            int src_x = x * logo_sprite.w / logo_dw;
            if (logo_sprite.pixels[src_y * logo_sprite.w + src_x] != logo_sprite.color_key) {
                count++;
            }
        }
    }

    if (count > MAX_LOGO_PARTICLES) count = MAX_LOGO_PARTICLES;
    logo_num = count;

    // fill end positions and colors
    int idx = 0;
    for (int y = 0; y < logo_dh && idx < logo_num; y++) {
        int src_y = y * logo_sprite.h / logo_dh;
        for (int x = 0; x < logo_dw && idx < logo_num; x++) {
            int src_x = x * logo_sprite.w / logo_dw;
            uint16_t c = logo_sprite.pixels[src_y * logo_sprite.w + src_x];
            if (c != logo_sprite.color_key) {
                logo_ex[idx] = (float)(logo_dx + x);
                logo_ey[idx] = (float)(logo_dy + y);
                logo_color[idx] = c;
                idx++;
            }
        }
    }

    // generate evenly spaced start positions in a grid
    int cols = (int)ceilf(sqrtf((float)count * SCREEN_W / SCREEN_H));
    int rows = (int)ceilf((float)count / cols);
    float step_x = (float)SCREEN_W / cols;
    float step_y = (float)SCREEN_H / rows;

    for (int i = 0; i < count; i++) {
        int col = i % cols;
        int row = i / cols;
        logo_sx[i] = step_x * (col + 0.5f);
        logo_sy[i] = step_y * (row + 0.5f);
    }

    logo_ready = true;
}

// sine wave amplitude: ramps up during convergence, decays after settling
static float logo_wave_amp(void)
{
    if (logo_time < LOGO_ANIM_TIME) {
        float t = logo_time / LOGO_ANIM_TIME;
        return LOGO_WAVE_MAX * t;
    }
    if (logo_time < LOGO_TOTAL_TIME) {
        float t = (logo_time - LOGO_ANIM_TIME) / LOGO_WAVE_TIME;
        return LOGO_WAVE_MAX * (1.0f - t);
    }
    return 0.0f;
}

// periodic shake: sin-envelope pulse every LOGO_SHAKE_INTERVAL seconds
static float logo_shake_amp(void)
{
    float elapsed = logo_time - LOGO_TOTAL_TIME;
    if (elapsed < LOGO_SHAKE_INTERVAL) return 0.0f;
    float cycle = fmodf(elapsed, LOGO_SHAKE_INTERVAL);
    if (cycle >= LOGO_SHAKE_DURATION) return 0.0f;
    float t = cycle / LOGO_SHAKE_DURATION;
    float env = sinf(t * (float)M_PI);
    return LOGO_SHAKE_AMP * env;
}

// load logo from flash data and build particle arrays
void logo_init(void)
{
    logo_sprite.w = LOGO_W;
    logo_sprite.h = LOGO_H;
    logo_sprite.color_key = LOGO_COLOR_KEY;
    logo_sprite.pixels = logo_pixels;
    logo_build_particles();
}

// true when convergence + wave animation has finished
bool logo_is_done(void)
{
    return logo_ready && logo_time >= LOGO_TOTAL_TIME;
}

// advance logo animation timer
void logo_update(float dt)
{
    if (!logo_ready) return;
    logo_time += dt;
}

// render logo: particles during convergence, sprite + shake after
void logo_draw(void)
{
    if (!logo_ready) return;

    // animation fully done — static or periodic shake
    if (logo_time >= LOGO_TOTAL_TIME) {
        float shake = logo_shake_amp();
        if (shake < 0.01f) {
            sprite_blit_scaled(&logo_sprite, logo_dx, logo_dy, logo_dw, logo_dh);
        } else {
            // per-row sine distortion
            for (int y = 0; y < logo_dh; y++) {
                int sy = logo_dy + y;
                if ((unsigned)sy >= SCREEN_H) continue;
                int src_y = y * logo_sprite.h / logo_dh;
                int ox = (int)(shake * sinf(sy * LOGO_SHAKE_FREQ + logo_time * LOGO_SHAKE_SPEED));
                int fb_row = sy * SCREEN_W;
                for (int x = 0; x < logo_dw; x++) {
                    int sx = logo_dx + x + ox;
                    if ((unsigned)sx >= SCREEN_W) continue;
                    int src_x = x * logo_sprite.w / logo_dw;
                    uint16_t c = logo_sprite.pixels[src_y * logo_sprite.w + src_x];
                    if (c != logo_sprite.color_key) {
                        fb_back[fb_row + sx] = c;
                    }
                }
            }
        }
        return;
    }

    // convergence + wave
    float t = logo_time / LOGO_ANIM_TIME;
    if (t > 1.0f) t = 1.0f;
    float e = t * (2.0f - t);
    float amp = logo_wave_amp();
    float phase = logo_time * LOGO_CONV_SPEED;

    for (int i = 0; i < logo_num; i++) {
        float x = logo_sx[i] + (logo_ex[i] - logo_sx[i]) * e;
        float y = logo_sy[i] + (logo_ey[i] - logo_sy[i]) * e;
        float wave = amp * sinf(y * LOGO_CONV_FREQ + phase + i * LOGO_CONV_DITHER);
        putpixel((int)(x + wave), (int)y, logo_color[i]);
    }
}
