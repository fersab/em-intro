// --- em-intro demoscene demo ---

// --- resolution (change these!) ---
const SCREEN_W = 480;
const SCREEN_H = 272;
const SCALE = 1.5;
const HORIZON_Y = (SCREEN_H * 0.75) | 0;

// --- timing ---
let time_now = 0;
let time_last = 0;
let dt = 0;

// --- scene config ---
const SCROLL_TEXT = new URLSearchParams(window.location.search).get("text")
  || "GREETINGS FROM THE CODERS OF TOMORROW ... THIS INTRO WAS CRAFTED BY A HUMAN AND AN AI WORKING TOGETHER ... NO PIXELS WERE HARMED IN THE MAKING OF THIS DEMO ... RESPECT TO ALL OLDSCHOOL SCENERS OUT THERE ...";
const SCROLL_SPEED = 50.0;
const SCROLL_Y = 24;
let scroller = null;

// --- main loop ---

// advance all effects by one frame
function update() {
  if (logo_is_done()) {
    fire_acc = fire_acc + dt;
    while (fire_acc >= FIRE_RATE) {
      fire_update();
      fire_acc = fire_acc - FIRE_RATE;
    }
  }
  stars_update();
  logo_update();
  if (scroller) scroller_update(scroller);
}

// render all effects to framebuffer in back-to-front order
function draw() {
  fb_clear(0);
  stars_draw();
  if (logo_is_done()) {
    fire_draw();
  }
  floor_draw();
  logo_draw();
  if (scroller) scroller_draw(scroller);
}

// per-frame callback: compute dt, update, draw, present
function main_loop(timestamp) {
  time_now = timestamp / 1000.0;
  dt = time_now - time_last;
  time_last = time_now;

  // create scroller once logo animation is done (fire starts)
  if (!scroller && logo_is_done()) {
    scroller = scroller_create(SCROLL_TEXT, SCROLL_SPEED, SCROLL_Y);
  }

  update();
  draw();
  fb_blit();

  requestAnimationFrame(main_loop);
}

// initialize all effects and start the main loop
function main_start() {
  font_init();
  fire_init();
  stars_init();
  logo_init();

  requestAnimationFrame(function(timestamp) {
    time_now = timestamp / 1000.0;
    time_last = time_now;
    dt = 0;
    main_loop(timestamp);
  });
}
