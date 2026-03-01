// --- procedural demoscene chrome font ---
//
// Generates chunky angular bitmap glyphs at init time.
// Each character is defined on a 5x7 cell grid, then scaled up
// and filled with a chrome gradient (steel-blue -> bright -> copper).
// All glyphs are pre-rendered into a flat atlas for fast blitting.

// --- font constants ---
const FONT_GW = 5;
const FONT_GH = 7;
const FONT_SCALE = 4;
const FONT_GLYPH_W = FONT_GW * FONT_SCALE;
const FONT_GLYPH_H = FONT_GH * FONT_SCALE;
const FONT_SPACING = 2;
const FONT_FIRST = 32;
const FONT_COUNT = 64;
const FONT_ATLAS_W = FONT_COUNT * FONT_GLYPH_W;

// --- font state ---
const font_grad = new Uint16Array(FONT_GLYPH_H);
const font_map = new Uint8Array(FONT_COUNT * FONT_GH);
let font_atlas = null;

// --- gradient helpers ---

// brighten (amt > 0) or darken (amt < 0) an RGB565 color by amt units
function font_adjust(c, amt) {
  let r = (((c >> 11) & 0x1F) << 3) + amt;
  let g = (((c >> 5) & 0x3F) << 2) + amt;
  let b = ((c & 0x1F) << 3) + amt;
  if (r < 0) r = 0; if (r > 255) r = 255;
  if (g < 0) g = 0; if (g > 255) g = 255;
  if (b < 0) b = 0; if (b > 255) b = 255;
  return rgb565(r, g, b);
}

// --- gradient build ---

// build chrome gradient lookup table for each glyph scanline
function font_build_gradient() {
  // chrome: steel-blue top -> bright highlight -> warm copper bottom
  let stop_t = [0.00, 0.20, 0.38, 0.45, 0.55, 0.75, 1.00];
  let stop_r = [  40,   80,  170,  230,  200,  160,   80];
  let stop_g = [  50,  100,  180,  225,  170,  120,   55];
  let stop_b = [ 110,  170,  210,  220,  100,   60,   30];

  for (let y = 0; y < FONT_GLYPH_H; y++) {
    let t = y / (FONT_GLYPH_H - 1);

    // find enclosing stops
    let si = 0;
    for (let s = 1; s < 7; s++) {
      if (stop_t[s] >= t) { si = s - 1; break; }
    }

    let lt = (t - stop_t[si]) / (stop_t[si + 1] - stop_t[si]);
    let r = stop_r[si] + (stop_r[si + 1] - stop_r[si]) * lt;
    let g = stop_g[si] + (stop_g[si + 1] - stop_g[si]) * lt;
    let b = stop_b[si] + (stop_b[si + 1] - stop_b[si]) * lt;

    // scanline groove: dim every other row for textured chrome look
    if (y & 1) {
      r = r * 0.82;
      g = g * 0.82;
      b = b * 0.82;
    }

    font_grad[y] = rgb565(r | 0, g | 0, b | 0);
  }
}

// --- glyph definitions ---

// store a 5x7 bitmap glyph: 7 rows of 5-bit bitmasks, MSB = left
function font_set(ch, b0, b1, b2, b3, b4, b5, b6) {
  let idx = (ch - FONT_FIRST) * FONT_GH;
  font_map[idx]     = b0;
  font_map[idx + 1] = b1;
  font_map[idx + 2] = b2;
  font_map[idx + 3] = b3;
  font_map[idx + 4] = b4;
  font_map[idx + 5] = b5;
  font_map[idx + 6] = b6;
}

// define all character bitmaps: punctuation, digits 0-9, A-Z
function font_define_chars() {
  // angular / blocky demoscene style
  // 5 bits per row, MSB = left column

  // punctuation / symbols
  font_set(32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00); // space
  font_set(33, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04); // !
  font_set(34, 0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00); // "
  font_set(35, 0x0A, 0x1F, 0x0A, 0x0A, 0x1F, 0x0A, 0x00); // #
  font_set(39, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00); // '
  font_set(40, 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02); // (
  font_set(41, 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08); // )
  font_set(42, 0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00); // *
  font_set(43, 0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00); // +
  font_set(44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08); // ,
  font_set(45, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00); // -
  font_set(46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04); // .
  font_set(47, 0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10); // /

  // digits - angular/boxy style
  font_set(48, 0x1F, 0x11, 0x13, 0x15, 0x19, 0x11, 0x1F); // 0
  font_set(49, 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x1F); // 1
  font_set(50, 0x1F, 0x01, 0x01, 0x1F, 0x10, 0x10, 0x1F); // 2
  font_set(51, 0x1F, 0x01, 0x01, 0x1F, 0x01, 0x01, 0x1F); // 3
  font_set(52, 0x11, 0x11, 0x11, 0x1F, 0x01, 0x01, 0x01); // 4
  font_set(53, 0x1F, 0x10, 0x10, 0x1F, 0x01, 0x01, 0x1F); // 5
  font_set(54, 0x1F, 0x10, 0x10, 0x1F, 0x11, 0x11, 0x1F); // 6
  font_set(55, 0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08); // 7
  font_set(56, 0x1F, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x1F); // 8
  font_set(57, 0x1F, 0x11, 0x11, 0x1F, 0x01, 0x01, 0x1F); // 9

  // misc
  font_set(58, 0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00); // :
  font_set(61, 0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00); // =
  font_set(63, 0x1F, 0x11, 0x01, 0x06, 0x04, 0x00, 0x04); // ?

  // A-Z - angular blocky demoscene style (flat tops/bottoms)
  font_set(65, 0x1F, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11); // A
  font_set(66, 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E); // B
  font_set(67, 0x1F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F); // C
  font_set(68, 0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E); // D
  font_set(69, 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F); // E
  font_set(70, 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10); // F
  font_set(71, 0x1F, 0x10, 0x10, 0x17, 0x11, 0x11, 0x1F); // G
  font_set(72, 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11); // H
  font_set(73, 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F); // I
  font_set(74, 0x1F, 0x01, 0x01, 0x01, 0x01, 0x11, 0x1F); // J
  font_set(75, 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11); // K
  font_set(76, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F); // L
  font_set(77, 0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11); // M
  font_set(78, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11); // N
  font_set(79, 0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F); // O
  font_set(80, 0x1F, 0x11, 0x11, 0x1F, 0x10, 0x10, 0x10); // P
  font_set(81, 0x1F, 0x11, 0x11, 0x11, 0x15, 0x12, 0x1F); // Q
  font_set(82, 0x1F, 0x11, 0x11, 0x1F, 0x14, 0x12, 0x11); // R
  font_set(83, 0x1F, 0x10, 0x10, 0x1F, 0x01, 0x01, 0x1F); // S
  font_set(84, 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04); // T
  font_set(85, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F); // U
  font_set(86, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04); // V
  font_set(87, 0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11); // W
  font_set(88, 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11); // X
  font_set(89, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04); // Y
  font_set(90, 0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F); // Z

  // underscore
  font_set(95, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F); // _
}

// --- atlas rendering ---

// rasterize all glyphs into a flat atlas with chrome gradient and bevel
function font_render_atlas() {
  font_atlas = new Uint16Array(FONT_ATLAS_W * FONT_GLYPH_H);

  for (let ci = 0; ci < FONT_COUNT; ci++) {
    let base = ci * FONT_GH;
    let ox = ci * FONT_GLYPH_W;

    for (let gy = 0; gy < FONT_GH; gy++) {
      let row = font_map[base + gy];
      if (row === 0) continue;

      // rows above/below for bevel detection
      let row_above = gy > 0 ? font_map[base + gy - 1] : 0;
      let row_below = gy < FONT_GH - 1 ? font_map[base + gy + 1] : 0;

      for (let gx = 0; gx < FONT_GW; gx++) {
        let bit = 1 << (FONT_GW - 1 - gx);
        if (!(row & bit)) continue;

        let is_top = !(row_above & bit);
        let is_bot = !(row_below & bit);

        let px_x = ox + gx * FONT_SCALE;
        for (let py = 0; py < FONT_SCALE; py++) {
          let ay = gy * FONT_SCALE + py;
          let color = font_grad[ay];

          // bevel: bright top edge, dark bottom edge
          if (is_top && py === 0) {
            color = font_adjust(color, 50);
          }
          if (is_bot && py === FONT_SCALE - 1) {
            color = font_adjust(color, -40);
          }

          for (let px = 0; px < FONT_SCALE; px++) {
            font_atlas[ay * FONT_ATLAS_W + px_x + px] = color;
          }
        }
      }
    }
  }
}

// --- public API ---

// build gradient, define chars, render atlas — call once at startup
function font_init() {
  font_build_gradient();
  font_define_chars();
  font_render_atlas();
}

// blit one character from atlas to framebuffer at (x, y)
function font_draw_char(code, x, y) {
  // map lowercase to uppercase
  if (code >= 97 && code <= 122) code = code - 32;
  if (code < FONT_FIRST || code >= FONT_FIRST + FONT_COUNT) return;

  let ox = (code - FONT_FIRST) * FONT_GLYPH_W;

  for (let gy = 0; gy < FONT_GLYPH_H; gy++) {
    let sy = y + gy;
    if (sy < 0 || sy >= SCREEN_H) continue;
    for (let gx = 0; gx < FONT_GLYPH_W; gx++) {
      let c = font_atlas[gy * FONT_ATLAS_W + ox + gx];
      if (c === 0) continue;
      let sx = x + gx;
      if (sx < 0 || sx >= SCREEN_W) continue;
      fb[sy * SCREEN_W + sx] = c;
    }
  }
}
