#ifndef FONT_H
#define FONT_H

#include <stdint.h>

#define FONT_GW       5
#define FONT_GH       7
#define FONT_SCALE    4
#define FONT_GLYPH_W  (FONT_GW * FONT_SCALE)
#define FONT_GLYPH_H  (FONT_GH * FONT_SCALE)
#define FONT_SPACING  2
#define FONT_FIRST    32
#define FONT_COUNT    64
#define FONT_ATLAS_W  (FONT_COUNT * FONT_GLYPH_W)

// build gradient, define chars, render atlas — call once at startup
void font_init(void);

// blit one character from atlas to framebuffer at (x, y)
void font_draw_char(int code, int x, int y);

#endif
