// --- sprite loading and blitting ---

// allocate a new sprite with given dimensions and zero color key
function sprite_create(w, h) {
  let spr = {};
  spr.w = w;
  spr.h = h;
  spr.color_key = 0;
  spr.pixels = new Uint16Array(w * h);
  return spr;
}

// draw sprite scaled to destination rect, skipping color-key pixels
function sprite_blit_scaled(spr, dx, dy, dw, dh) {
  for (let y = 0; y < dh; y++) {
    let sy = dy + y;
    if (sy < 0 || sy >= SCREEN_H) continue;
    let src_y = (y * spr.h / dh) | 0;
    for (let x = 0; x < dw; x++) {
      let sx = dx + x;
      if (sx < 0 || sx >= SCREEN_W) continue;
      let src_x = (x * spr.w / dw) | 0;
      let c = spr.pixels[src_y * spr.w + src_x];
      if (c !== spr.color_key) {
        fb[sy * SCREEN_W + sx] = c;
      }
    }
  }
}

// load image from URL, convert to RGB565 sprite (browser only)
function sprite_load(src, callback) {
  let img = new Image();
  img.onload = function() {
    let tmp = document.createElement("canvas");
    tmp.width = img.width;
    tmp.height = img.height;
    let tmpctx = tmp.getContext("2d");
    tmpctx.drawImage(img, 0, 0);
    let data = tmpctx.getImageData(0, 0, img.width, img.height).data;

    let spr = sprite_create(img.width, img.height);
    // use top-left pixel as color key
    spr.color_key = rgb565(data[0], data[1], data[2]);
    for (let i = 0; i < img.width * img.height; i++) {
      let r = data[i * 4];
      let g = data[i * 4 + 1];
      let b = data[i * 4 + 2];
      spr.pixels[i] = rgb565(r, g, b);
    }
    callback(spr);
  };
  img.src = src;
}
