// --- checkerboard floor ---

#include "floor.h"
#include "graph.h"

#define FLOOR_Z_NEAR  1.0f
#define FLOOR_Z_FAR   6.0f
#define FLOOR_FOCAL   96.0f
#define FLOOR_ROWS    (SCREEN_H - HORIZON_Y)

static const float INV_Z_NEAR = 1.0f / FLOOR_Z_NEAR;
static const float INV_Z_FAR  = 1.0f / FLOOR_Z_FAR;

static uint16_t col_red;
static uint16_t col_white;

// pre-compute tile colors
void floor_init(void)
{
    col_red   = rgb565(180, 40, 40);
    col_white = rgb565(220, 220, 220);
}

// render perspective checkerboard floor from horizon to screen bottom
void floor_draw(void)
{
    for (int y = HORIZON_Y - 4; y < SCREEN_H; y++) {
        float t = (float)(SCREEN_H - 1 - y) / (float)(FLOOR_ROWS - 1);
        float z = 1.0f / (INV_Z_NEAR + (INV_Z_FAR - INV_Z_NEAR) * t);
        int tz = (int)z;
        int fb_row = y * SCREEN_W;

        for (int x = 0; x < SCREEN_W; x++) {
            float wx = (float)(x - SCREEN_W / 2) * z / FLOOR_FOCAL;
            int tx = (wx >= 0.0f) ? (int)wx : ((int)wx - 1);
            int tile = (tx + tz) & 1;
            fb_back[fb_row + x] = tile ? col_red : col_white;
        }
    }
}
