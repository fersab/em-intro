// --- sprite blitting ---

#include "sprite.h"
#include "graph.h"

// draw sprite scaled to destination rect, skipping color-key pixels
void sprite_blit_scaled(const sprite_t *spr, int dx, int dy, int dw, int dh)
{
    for (int y = 0; y < dh; y++) {
        int sy = dy + y;
        if ((unsigned)sy >= SCREEN_H) continue;
        int src_y = y * spr->h / dh;
        int fb_row = sy * SCREEN_W;
        for (int x = 0; x < dw; x++) {
            int sx = dx + x;
            if ((unsigned)sx >= SCREEN_W) continue;
            int src_x = x * spr->w / dw;
            uint16_t c = spr->pixels[src_y * spr->w + src_x];
            if (c != spr->color_key) {
                fb_back[fb_row + sx] = c;
            }
        }
    }
}
