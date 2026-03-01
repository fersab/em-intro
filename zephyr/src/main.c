// --- em-intro demoscene demo ---
// Zephyr build: STM32H747I-DISCO bare metal
// Emscripten build: browser via WebAssembly

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#else
#include <zephyr/kernel.h>
#endif

#include "graph.h"
#include "font.h"
#include "fire.h"
#include "floor.h"
#include "stars.h"
#include "logo.h"
#include "scroller.h"

// --- scene config ---
static const char SCROLL_TEXT[] =
    "GREETINGS FROM THE CODERS OF TOMORROW ... "
    "THIS INTRO WAS CRAFTED BY A HUMAN AND AN AI WORKING TOGETHER ... "
    "NO PIXELS WERE HARMED IN THE MAKING OF THIS DEMO ... "
    "RESPECT TO ALL OLDSCHOOL SCENERS OUT THERE ...";

#define SCROLL_SPEED  50.0f
#define SCROLL_Y      24

// --- timing ---
static float time_now = 0.0f;
static float dt = 0.0f;
static float fire_acc = 0.0f;

// --- scroller state ---
static scroller_t scroller;
static bool scroller_active = false;

// advance all effects by one frame
static void update(void)
{
    if (logo_is_done()) {
        fire_acc += dt;
        while (fire_acc >= FIRE_RATE) {
            fire_update();
            fire_acc -= FIRE_RATE;
        }
    }
    stars_update(dt);
    logo_update(dt);
    if (scroller_active) {
        scroller_update(&scroller, dt);
    }
}

// render all effects to framebuffer in back-to-front order
static void draw(void)
{
    fb_clear(0);
    stars_draw();
    if (logo_is_done()) {
        fire_draw();
    }
    floor_draw();
    logo_draw();
    if (scroller_active) {
        scroller_draw(&scroller, time_now);
    }
}

// === Emscripten main loop ===
#ifdef __EMSCRIPTEN__

// JS-side function to blit the RGBA buffer to canvas
EM_JS(void, js_blit_canvas, (const uint8_t *buf, int w, int h), {
    var canvas = document.getElementById('screen')
              || document.getElementById('c');
    if (!canvas) { console.error('no canvas found'); return; }
    var ctx = canvas.getContext('2d');
    var img = ctx.createImageData(w, h);
    img.data.set(HEAPU8.subarray(buf, buf + w * h * 4));
    ctx.putImageData(img, 0, 0);
});

static double last_time = 0.0;

static void em_frame(void)
{
    double now = emscripten_get_now() / 1000.0;
    dt = (float)(now - last_time);
    last_time = now;

    // clamp dt to avoid huge jumps
    if (dt > 0.1f) dt = 0.1f;

    time_now += dt;

    // create scroller once logo animation is done
    if (!scroller_active && logo_is_done()) {
        scroller_create(&scroller, SCROLL_TEXT, SCROLL_SPEED, SCROLL_Y);
        scroller_active = true;
    }

    update();
    draw();
    fb_swap();

    // blit RGBA buffer to canvas
    js_blit_canvas(graph_get_canvas_buf(), SCREEN_W, SCREEN_H);
}

int main(void)
{
    graph_init();
    font_init();
    fire_init();
    floor_init();
    stars_init();
    logo_init();

    last_time = emscripten_get_now() / 1000.0;

    // 0 = use requestAnimationFrame, 1 = simulate infinite loop
    emscripten_set_main_loop(em_frame, 0, 1);

    return 0;
}

#else
// === Zephyr main loop ===

int main(void)
{
    // initialize display
    int ret = graph_init();
    if (ret < 0) {
        printk("Display init failed: %d\n", ret);
        return ret;
    }

    // initialize all effects
    font_init();
    fire_init();
    floor_init();
    stars_init();
    logo_init();

    printk("em-intro: all effects initialized, entering main loop\n");

    // main loop
    int64_t last_ms = k_uptime_get();

    while (1) {
        int64_t now_ms = k_uptime_get();
        dt = (float)(now_ms - last_ms) / 1000.0f;
        last_ms = now_ms;

        // clamp dt to avoid huge jumps on first frame or debugger pause
        if (dt > 0.1f) dt = 0.1f;

        time_now += dt;

        // create scroller once logo animation is done (fire starts)
        if (!scroller_active && logo_is_done()) {
            scroller_create(&scroller, SCROLL_TEXT, SCROLL_SPEED, SCROLL_Y);
            scroller_active = true;
        }

        update();
        draw();
        fb_swap();

        // target ~60fps (16ms per frame)
        int64_t elapsed = k_uptime_get() - now_ms;
        if (elapsed < 16) {
            k_msleep(16 - (int)elapsed);
        }
    }

    return 0;
}

#endif
