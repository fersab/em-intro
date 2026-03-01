#!/usr/bin/env python3
"""Convert a PNG image to a C header with RGB565 pixel data."""
import sys
from PIL import Image

def rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <image.png> [scale%]", file=sys.stderr)
        sys.exit(1)

    img = Image.open(sys.argv[1]).convert('RGBA')

    # optional scale percentage (e.g. 40 means 40% of original)
    if len(sys.argv) >= 3:
        scale = int(sys.argv[2]) / 100.0
        new_w = int(img.width * scale)
        new_h = int(img.height * scale)
        img = img.resize((new_w, new_h), Image.LANCZOS)
        print(f"Resized to {new_w}x{new_h} ({sys.argv[2]}%)", file=sys.stderr)

    w, h = img.size
    pixels = list(img.getdata())

    # color key from top-left pixel
    r0, g0, b0, _ = pixels[0]
    color_key = rgb565(r0, g0, b0)

    print("#ifndef LOGO_DATA_H")
    print("#define LOGO_DATA_H")
    print("")
    print("#include <stdint.h>")
    print("")
    print(f"#define LOGO_W {w}")
    print(f"#define LOGO_H {h}")
    print(f"#define LOGO_COLOR_KEY 0x{color_key:04X}")
    print("")
    print(f"static const uint16_t logo_pixels[{w * h}] = {{")

    for i, (r, g, b, a) in enumerate(pixels):
        # transparent or semi-transparent pixels become color key
        val = color_key if a < 128 else rgb565(r, g, b)
        end = "," if i < len(pixels) - 1 else ""
        if i % 16 == 0:
            print("    ", end="")
        print(f"0x{val:04X}{end}", end="")
        if i % 16 == 15 or i == len(pixels) - 1:
            print()

    print("};")
    print("")
    print("#endif")

if __name__ == '__main__':
    main()
