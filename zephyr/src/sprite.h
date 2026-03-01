#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>

typedef struct {
    uint16_t w;
    uint16_t h;
    uint16_t color_key;
    const uint16_t *pixels;
} sprite_t;

// draw sprite scaled to destination rect, skipping color-key pixels
void sprite_blit_scaled(const sprite_t *spr, int dx, int dy, int dw, int dh);

#endif
