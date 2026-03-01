// --- sine scroller ---
//
// Tiling horizontal sine-wave text scroller.
// Caller provides text, speed, position via a state struct.

// create a tiling scroller state: pads text with spaces for seamless wrap
function scroller_create(text, speed, y) {
  let s = {};
  s.step = FONT_GLYPH_W + FONT_SPACING;
  // pad with a full screen-width of spaces between repeats
  let pad_count = Math.ceil(SCREEN_W / s.step);
  let pad = "";
  for (let i = 0; i < pad_count; i++) pad = pad + " ";
  s.text = text + pad;
  s.speed = speed;
  s.y = y;
  s.x = SCREEN_W;
  s.tile_w = s.text.length * s.step;
  return s;
}

// advance scroller position left by speed * dt
function scroller_update(s) {
  s.x = s.x - s.speed * dt;
}

// render tiling sine-wave text across the screen
function scroller_draw(s) {
  let ox = s.x % s.tile_w;
  if (ox > 0) ox = ox - s.tile_w;

  let x0 = ox;
  while (x0 < SCREEN_W) {
    for (let i = 0; i < s.text.length; i++) {
      let cx = x0 + i * s.step;
      if (cx >= SCREEN_W) break;
      if (cx < -FONT_GLYPH_W) continue;
      let cy = (s.y + Math.sin(time_now * 3.0 + cx * 0.02) * 12.0) | 0;
      font_draw_char(s.text.charCodeAt(i), cx | 0, cy);
    }
    x0 = x0 + s.tile_w;
  }
}
