// --- logo particle animation ---
let logo = null;
let logo_ready = false;
let logo_time = 0.0;
const LOGO_ANIM_TIME = 4.0;
const LOGO_WAVE_TIME = 2.5;
const LOGO_WAVE_MAX = 15.0;
const LOGO_TOTAL_TIME = LOGO_ANIM_TIME + LOGO_WAVE_TIME;

// periodic shake
const LOGO_SHAKE_INTERVAL = 10.0;
const LOGO_SHAKE_DURATION = 0.8;
const LOGO_SHAKE_AMP = 6.0;

// display dimensions
let logo_dw = 0;
let logo_dh = 0;
let logo_dx = 0;
let logo_dy = 0;

// particle arrays (struct-of-arrays)
let logo_num = 0;
let logo_sx = null;
let logo_sy = null;
let logo_ex = null;
let logo_ey = null;
let logo_color = null;

// load logo image and kick off particle setup
function logo_init() {
  sprite_load("logo.png", function(spr) {
    logo = spr;
    logo_build_particles();
  });
}

// scale logo, extract non-transparent pixels into particle arrays
function logo_build_particles() {
  logo_dw = (SCREEN_W * 0.5) | 0;
  logo_dh = (logo_dw * logo.h / logo.w) | 0;
  logo_dx = ((SCREEN_W - logo_dw) / 2) | 0;
  logo_dy = ((SCREEN_H - logo_dh) / 2 + 20) | 0;

  // count non-transparent pixels at display resolution
  let count = 0;
  for (let y = 0; y < logo_dh; y++) {
    let src_y = (y * logo.h / logo_dh) | 0;
    for (let x = 0; x < logo_dw; x++) {
      let src_x = (x * logo.w / logo_dw) | 0;
      if (logo.pixels[src_y * logo.w + src_x] !== logo.color_key) count++;
    }
  }

  logo_num = count;
  logo_sx = new Float32Array(count);
  logo_sy = new Float32Array(count);
  logo_ex = new Float32Array(count);
  logo_ey = new Float32Array(count);
  logo_color = new Uint16Array(count);

  // fill end positions and colors
  let idx = 0;
  for (let y = 0; y < logo_dh; y++) {
    let src_y = (y * logo.h / logo_dh) | 0;
    for (let x = 0; x < logo_dw; x++) {
      let src_x = (x * logo.w / logo_dw) | 0;
      let c = logo.pixels[src_y * logo.w + src_x];
      if (c !== logo.color_key) {
        logo_ex[idx] = logo_dx + x;
        logo_ey[idx] = logo_dy + y;
        logo_color[idx] = c;
        idx++;
      }
    }
  }

  // generate evenly spaced start positions in a grid
  let cols = Math.ceil(Math.sqrt(count * SCREEN_W / SCREEN_H));
  let rows = Math.ceil(count / cols);
  let step_x = SCREEN_W / cols;
  let step_y = SCREEN_H / rows;

  for (let i = 0; i < count; i++) {
    let col = i % cols;
    let row = (i / cols) | 0;
    logo_sx[i] = step_x * (col + 0.5);
    logo_sy[i] = step_y * (row + 0.5);
  }

  logo_ready = true;
}

// sine wave amplitude: ramps up during convergence, decays after settling
function logo_wave_amp() {
  if (logo_time < LOGO_ANIM_TIME) {
    // ramp up during convergence
    let t = logo_time / LOGO_ANIM_TIME;
    return LOGO_WAVE_MAX * t;
  }
  if (logo_time < LOGO_TOTAL_TIME) {
    // decay after settling
    let t = (logo_time - LOGO_ANIM_TIME) / LOGO_WAVE_TIME;
    return LOGO_WAVE_MAX * (1.0 - t);
  }
  return 0.0;
}

// true when convergence + wave animation has finished
function logo_is_done() {
  return logo_ready && logo_time >= LOGO_TOTAL_TIME;
}

// advance logo animation timer
function logo_update() {
  if (!logo_ready) return;
  logo_time = logo_time + dt;
}

// periodic shake: sin-envelope pulse every LOGO_SHAKE_INTERVAL seconds
function logo_shake_amp() {
  let elapsed = logo_time - LOGO_TOTAL_TIME;
  if (elapsed < LOGO_SHAKE_INTERVAL) return 0.0;
  let cycle = elapsed % LOGO_SHAKE_INTERVAL;
  if (cycle >= LOGO_SHAKE_DURATION) return 0.0;
  // quick ramp up then decay
  let t = cycle / LOGO_SHAKE_DURATION;
  let env = Math.sin(t * Math.PI);
  return LOGO_SHAKE_AMP * env;
}

// render logo: particles during convergence, sprite + shake after
function logo_draw() {
  if (!logo_ready) return;

  // animation fully done — static or periodic shake
  if (logo_time >= LOGO_TOTAL_TIME) {
    let shake = logo_shake_amp();
    if (shake < 0.01) {
      sprite_blit_scaled(logo, logo_dx, logo_dy, logo_dw, logo_dh);
    } else {
      // per-row sine distortion
      for (let y = 0; y < logo_dh; y++) {
        let sy = logo_dy + y;
        if (sy < 0 || sy >= SCREEN_H) continue;
        let src_y = (y * logo.h / logo_dh) | 0;
        let ox = (shake * Math.sin(sy * 0.15 + logo_time * 8.0)) | 0;
        for (let x = 0; x < logo_dw; x++) {
          let sx = logo_dx + x + ox;
          if (sx < 0 || sx >= SCREEN_W) continue;
          let src_x = (x * logo.w / logo_dw) | 0;
          let c = logo.pixels[src_y * logo.w + src_x];
          if (c !== logo.color_key) {
            fb[sy * SCREEN_W + sx] = c;
          }
        }
      }
    }
    return;
  }

  // convergence + wave
  let t = logo_time / LOGO_ANIM_TIME;
  if (t > 1.0) t = 1.0;
  let e = t * (2.0 - t);
  let amp = logo_wave_amp();
  let phase = logo_time * 4.0;

  for (let i = 0; i < logo_num; i++) {
    let x = logo_sx[i] + (logo_ex[i] - logo_sx[i]) * e;
    let y = logo_sy[i] + (logo_ey[i] - logo_sy[i]) * e;
    let wave = amp * Math.sin(y * 0.05 + phase + i * 0.001);
    putpixel((x + wave) | 0, y | 0, logo_color[i]);
  }
}
