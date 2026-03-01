// --- checkerboard floor ---
const FLOOR_Z_NEAR = 1.0;
const FLOOR_Z_FAR = 6.0;
const FLOOR_FOCAL = 96.0;
const FLOOR_ROWS = SCREEN_H - HORIZON_Y;
const INV_Z_NEAR = 1.0 / FLOOR_Z_NEAR;
const INV_Z_FAR  = 1.0 / FLOOR_Z_FAR;

const COL_RED   = rgb565(180, 40, 40);
const COL_WHITE = rgb565(220, 220, 220);

// render perspective checkerboard floor from horizon to screen bottom
function floor_draw() {
  for (let y = HORIZON_Y - 4; y < SCREEN_H; y++) {
    let t = (SCREEN_H - 1 - y) / (FLOOR_ROWS - 1);
    let z = 1.0 / (INV_Z_NEAR + (INV_Z_FAR - INV_Z_NEAR) * t);
    let tz = (z | 0);

    for (let x = 0; x < SCREEN_W; x++) {
      let wx = (x - SCREEN_W / 2) * z / FLOOR_FOCAL;
      let tx = wx >= 0 ? (wx | 0) : ((wx | 0) - 1);

      let tile = (tx + tz) & 1;
      putpixel(x, y, tile ? COL_RED : COL_WHITE);
    }
  }
}
