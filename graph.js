// --- screen setup and framebuffer ---
const canvas = document.getElementById("screen");
canvas.width = SCREEN_W;
canvas.height = SCREEN_H;
canvas.style.width = (SCREEN_W * SCALE) + "px";
canvas.style.height = (SCREEN_H * SCALE) + "px";

const ctx = canvas.getContext("2d");
const imgData = ctx.createImageData(SCREEN_W, SCREEN_H);

// --- framebuffer ---
const fb = new Uint16Array(SCREEN_W * SCREEN_H);

// --- primitives ---

// pack 8-bit RGB into 16-bit RGB565
function rgb565(r, g, b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// write one RGB565 pixel to framebuffer with bounds check
function putpixel(x, y, color) {
  if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return;
  fb[y * SCREEN_W + x] = color;
}

// fill entire framebuffer with a single color
function fb_clear(color) {
  fb.fill(color);
}

// convert RGB565 framebuffer to RGBA and draw to canvas
function fb_blit() {
  let pixels = imgData.data;
  for (let i = 0; i < fb.length; i++) {
    let c = fb[i];
    let r = (c >> 11) & 0x1F;
    let g = (c >> 5) & 0x3F;
    let b = c & 0x1F;
    let off = i * 4;
    pixels[off]     = (r << 3) | (r >> 2);
    pixels[off + 1] = (g << 2) | (g >> 4);
    pixels[off + 2] = (b << 3) | (b >> 2);
    pixels[off + 3] = 255;
  }
  ctx.putImageData(imgData, 0, 0);
}
