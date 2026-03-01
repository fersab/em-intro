#ifndef SCROLLER_H
#define SCROLLER_H

#include <stdbool.h>

typedef struct {
    const char *text;
    int text_len;
    float speed;
    int y;
    float x;
    int step;
    int tile_w;
} scroller_t;

// create a tiling scroller state: pads text with spaces for seamless wrap
void scroller_create(scroller_t *s, const char *text, float speed, int y);

// advance scroller position left by speed * dt
void scroller_update(scroller_t *s, float dt);

// render tiling sine-wave text across the screen
void scroller_draw(const scroller_t *s, float time_now);

#endif
