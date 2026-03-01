// --- em-intro demoscene demo — Zephyr / STM32H747I-DISCO ---

#include <zephyr/kernel.h>

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

// initialize all effects and run the main loop
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
