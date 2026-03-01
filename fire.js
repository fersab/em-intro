// --- fire effect ---
const FIRE_H = 42;
const FIRE_TOP_Y = HORIZON_Y - 40;
const FIRE_RATE = 1.0 / 20.0;
let fire_acc = 0.0;
const fire_buf = new Uint8Array(SCREEN_W * FIRE_H);
const fire_pal = new Uint16Array(256);

// build fire palette: black -> red -> yellow -> white (256 entries)
function fire_init() {
  for (let i = 0; i < 64; i++) {
    fire_pal[i] = rgb565(i * 4, 0, 0);
  }
  for (let i = 64; i < 128; i++) {
    fire_pal[i] = rgb565(255, (i - 64) * 4, 0);
  }
  for (let i = 128; i < 192; i++) {
    fire_pal[i] = rgb565(255, 255, (i - 128) * 4);
  }
  for (let i = 192; i < 256; i++) {
    fire_pal[i] = rgb565(255, 255, 255);
  }
}

// one step of fire simulation: seed bottom rows, average and propagate up
function fire_update() {
  // seed bottom two rows
  for (let x = 0; x < SCREEN_W; x++) {
    fire_buf[(FIRE_H - 1) * SCREEN_W + x] = (Math.random() * 255) | 0;
    fire_buf[(FIRE_H - 2) * SCREEN_W + x] = (Math.random() * 255) | 0;
  }

  // average 4 neighbors, propagate upward
  for (let y = 0; y < FIRE_H - 2; y++) {
    for (let x = 0; x < SCREEN_W; x++) {
      let xl = x > 0 ? x - 1 : SCREEN_W - 1;
      let xr = x < SCREEN_W - 1 ? x + 1 : 0;

      let sum = fire_buf[(y + 1) * SCREEN_W + xl]
              + fire_buf[(y + 1) * SCREEN_W + x]
              + fire_buf[(y + 1) * SCREEN_W + xr]
              + fire_buf[(y + 2) * SCREEN_W + x];

      let val = (sum >> 2) - 4;
      if (val < 0) val = 0;
      fire_buf[y * SCREEN_W + x] = val;
    }
  }
}

// render fire buffer to framebuffer using the heat palette
function fire_draw() {
  for (let y = 0; y < FIRE_H - 4; y++) {
    let sy = FIRE_TOP_Y + y;
    for (let x = 0; x < SCREEN_W; x++) {
      let heat = fire_buf[y * SCREEN_W + x];
      if (heat > 0) {
        putpixel(x, sy, fire_pal[heat]);
      }
    }
  }
}
