// --- sine scroller ---
//
// Tiling horizontal sine-wave text scroller.
// Caller provides text, speed, position via a state struct.

#include "scroller.h"
#include "font.h"
#include "graph.h"
#include <math.h>
#include <string.h>

#define SCROLL_WAVE_SPEED  3.0f    // temporal speed of sine wave
#define SCROLL_WAVE_FREQ   0.02f   // spatial frequency of sine wave
#define SCROLL_WAVE_AMP    12.0f   // vertical amplitude in pixels
#define SCROLL_MAX_TEXT    512

// static buffer for padded text
static char scroller_textbuf[SCROLL_MAX_TEXT];

// create a tiling scroller state: pads text with spaces for seamless wrap
void scroller_create(scroller_t *s, const char *text, float speed, int y)
{
    s->step = FONT_GLYPH_W + FONT_SPACING;

    // pad with a full screen-width of spaces between repeats
    int pad_count = (SCREEN_W + s->step - 1) / s->step;
    int text_len = (int)strlen(text);

    // copy text + padding into static buffer
    if (text_len + pad_count >= SCROLL_MAX_TEXT) {
        text_len = (int)sizeof(scroller_textbuf) - pad_count - 1;
    }
    memcpy(scroller_textbuf, text, text_len);
    memset(scroller_textbuf + text_len, ' ', pad_count);
    scroller_textbuf[text_len + pad_count] = '\0';

    s->text = scroller_textbuf;
    s->text_len = text_len + pad_count;
    s->speed = speed;
    s->y = y;
    s->x = (float)SCREEN_W;
    s->tile_w = s->text_len * s->step;
}

// advance scroller position left by speed * dt
void scroller_update(scroller_t *s, float dt)
{
    s->x -= s->speed * dt;
}

// render tiling sine-wave text across the screen
void scroller_draw(const scroller_t *s, float time_now)
{
    float ox = fmodf(s->x, (float)s->tile_w);
    if (ox > 0.0f) ox -= s->tile_w;

    float x0 = ox;
    while (x0 < SCREEN_W) {
        for (int i = 0; i < s->text_len; i++) {
            float cx = x0 + i * s->step;
            if (cx >= SCREEN_W) break;
            if (cx < -FONT_GLYPH_W) continue;
            int cy = (int)(s->y + sinf(time_now * SCROLL_WAVE_SPEED + cx * SCROLL_WAVE_FREQ) * SCROLL_WAVE_AMP);
            font_draw_char((int)(unsigned char)s->text[i], (int)cx, cy);
        }
        x0 += s->tile_w;
    }
}
