// --- starfield ---
const NUM_STARS = 200;
const NUM_LAYERS = 3;

const star_x     = new Float32Array(NUM_STARS);
const star_y     = new Float32Array(NUM_STARS);
const star_layer = new Uint8Array(NUM_STARS);

const layer_speed = [20, 50, 90];
const layer_color = new Uint16Array(NUM_LAYERS);

// pre-compute layer colors and randomize star positions
function stars_init() {
  for (let l = 0; l < NUM_LAYERS; l++) {
    let b = (l + 1) * 80;
    if (b > 255) b = 255;
    layer_color[l] = rgb565(b, b, b);
  }
  for (let i = 0; i < NUM_STARS; i++) {
    star_x[i] = Math.random() * SCREEN_W;
    star_y[i] = Math.random() * HORIZON_Y;
    star_layer[i] = i % NUM_LAYERS;
  }
}

// scroll stars left at layer-dependent speeds, wrap around
function stars_update() {
  for (let i = 0; i < NUM_STARS; i++) {
    let layer = star_layer[i];
    star_x[i] = star_x[i] - layer_speed[layer] * dt;
    if (star_x[i] < 0) {
      star_x[i] = star_x[i] + SCREEN_W;
      star_y[i] = Math.random() * HORIZON_Y;
    }
  }
}

// render all stars as single pixels
function stars_draw() {
  for (let i = 0; i < NUM_STARS; i++) {
    putpixel(star_x[i] | 0, star_y[i] | 0, layer_color[star_layer[i]]);
  }
}
