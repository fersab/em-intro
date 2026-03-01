# EM Intro

A retro demoscene intro built three ways from one design:

- **JS6** -- vanilla JavaScript in imperative C style. No frameworks, no classes, no
  build step. Just flat arrays, `putpixel`, and `requestAnimationFrame`. Runs in any browser.
- **C-Version (WASM)** -- the exact same C source files that run on bare-metal hardware,
  compiled to WebAssembly with Emscripten. One codebase, two targets: a Zephyr RTOS board
  and a browser tab.
- **C/Zephyr** -- the native build. Same C code runs directly on an STM32H747I-DISCO
  discovery board, rendering to a 480x272 TFT-LCD panel at 60fps under Zephyr RTOS.

Toggle between the JS6 and C-Version (WASM) renderers using the tabs above the screen in
the browser. Both produce the same demo -- stars, fire, checkerboard floor, logo particle
convergence, and a chrome sine scroller -- but from completely different codebases that
share the same architecture.

## Prerequisites

The JS6 version runs in any browser with no build step. To build the C-Version (WASM)
or work with the full toolchain, you need:

**All platforms:**
- **Git** -- to clone the repo and the Emscripten SDK
- **Python 3** -- for the dev server and the `png2rgb565.py` logo converter
- **Bash** -- all scripts use `#!/usr/bin/env bash`

**macOS:**
- **Homebrew** -- used by `scripts/setup.sh` to install missing packages
- Xcode Command Line Tools (`xcode-select --install`) for `git` and `make`

**Linux (Debian/Ubuntu):**
- `sudo apt-get install git python3 python3-pip build-essential`

**Linux (Fedora):**
- `sudo dnf install git python3 python3-pip make`

**Linux (Arch):**
- `sudo pacman -S git python python-pip make`

**Optional (for bare-metal STM32 build only):**
- Zephyr SDK and `west` -- see [C-Version / Zephyr](#c-version-zephyr-bare-metal)

Once prerequisites are in place, `scripts/setup.sh` handles the rest (Emscripten SDK,
Python Pillow library).

## Quick Start

```sh
./scripts/setup.sh          # install dependencies (Emscripten, Pillow)
./scripts/build.sh           # build WASM
./scripts/serve.sh           # start dev server, opens browser
```

Or using make: `make setup && make build && make serve`

## Contents

- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Two Codebases, One Architecture](#two-codebases-one-architecture)
- [What You See](#what-you-see)
- [From putpixel to Demo: A Walkthrough](#from-putpixel-to-demo-a-walkthrough)
  - [1. The Framebuffer and putpixel](#1-the-framebuffer-and-putpixel)
  - [2. The Starfield](#2-the-starfield)
  - [3. The Checkerboard Floor](#3-the-checkerboard-floor)
  - [4. The Fire Effect](#4-the-fire-effect)
  - [5. The Procedural Chrome Font](#5-the-procedural-chrome-font)
  - [6. The Sine Scroller](#6-the-sine-scroller)
  - [7. The Logo Particle Convergence](#7-the-logo-particle-convergence)
  - [8. The Sprite System](#8-the-sprite-system)
- [How the Effects Layer Together](#how-the-effects-layer-together)
- [The C-Version on Zephyr (Bare Metal)](#the-c-version-on-zephyr-bare-metal)
- [Building and Running](#building-and-running)
  - [JS6 (browser)](#js6-browser-no-build-step)
  - [C-Version / WASM (browser)](#c-version-wasm-browser-same-c-source-compiled-with-emscripten)
  - [C-Version / Zephyr (bare metal)](#c-version-zephyr-bare-metal)
  - [Troubleshooting the C build](#troubleshooting-the-c-build)
  - [Local syntax check](#local-syntax-check-no-zephyr-sdk-required)
- [Credits](#credits)

## Two Codebases, One Architecture

The JS6 and C versions are not ports of each other in the usual sense. They were written
side by side with the same structure: same file names, same function names, same rendering
pipeline. `stars.js` mirrors `stars.c`. `fire.js` mirrors `fire.c`. The JS code reads like
C on purpose -- imperative, procedural, no OOP, no closures for state. If you can read one
version, you can read the other.

The C version has a trick: it compiles to two completely different targets from the same
source files. On the STM32 board, it links against the Zephyr RTOS display API and runs on
bare metal. In the browser, Emscripten compiles the same `.c` files to WebAssembly, and a
thin platform bridge in `graph.c` swaps the display backend:

- **Zephyr path:** `fb_swap()` calls `display_write()` to DMA the RGB565 framebuffer to
  the LTDC controller. Framebuffers live in external SDRAM.
- **Emscripten path:** `fb_swap()` expands RGB565 → RGBA32 into a canvas buffer. JavaScript
  glue reads that buffer via `HEAPU8` and paints it to an HTML canvas with `putImageData`.

That is the entire bridge. Every other line of C -- the fire algorithm, the starfield, the
logo particle system, the font renderer, the floor, the scroller -- is identical across
both targets. The `#ifdef __EMSCRIPTEN__` blocks are confined to `graph.c` (display init
and swap) and `main.c` (frame loop and timing). Everything else is pure, portable C11.

## What You See

When the demo starts, the screen is black. Stars drift across the upper sky. Then thousands
of colored particles, scattered in a uniform grid across the entire screen, begin converging
toward the center. They ripple with sine waves as they assemble themselves into the logo.
Once the logo settles, flames erupt along the horizon line and a perspective checkerboard
floor stretches out below it. A chrome-lettered sine scroller begins weaving across the top
of the screen, delivering greetings in the finest oldschool tradition. Every few seconds the
logo gives a quick horizontal shake, just to remind you it is alive.

All of this runs at 480x272 in 16-bit color, targeting 60fps, whether you are looking at
a canvas element in your browser or a physical LCD panel on an STM32 discovery board.

## From putpixel to Demo: A Walkthrough

This section traces the path from the most basic building block -- writing a single pixel --
up to the full layered demo. Every demoscene production starts here.

### 1. The Framebuffer and putpixel

Everything begins with a flat array of 16-bit integers. Each entry is one pixel in RGB565
format: 5 bits red, 6 bits green, 5 bits blue, packed into a `uint16_t`. The screen
resolution is 480x272, the native size of the STM32H747I-DISCO's LCD panel.

The `putpixel` function takes an x, y coordinate and a 16-bit color, does a bounds check,
and writes it into the framebuffer:

```
fb[y * SCREEN_W + x] = color;
```

The `rgb565` helper packs 8-bit RGB channels down to 16 bits. On the JS side, `fb_blit`
expands every RGB565 value back into 32-bit RGBA for the canvas. On the C side, the
framebuffer is written directly to the display controller via Zephyr's display API.

This is the only primitive the entire demo needs. Every star, every fire pixel, every
font glyph, every floor tile -- all drawn through this one function.

### 2. The Starfield

The starfield is one of the oldest demoscene effects. It creates the illusion of depth
with nothing but single-pixel dots moving at different speeds.

200 stars are divided into three layers. Each layer scrolls left at a different speed:
the back layer crawls at 20 pixels/second, the middle at 50, and the front at 90. Faster
stars are brighter (higher RGB values), slower stars are dimmer. When a star scrolls off
the left edge, it wraps around to the right with a new random Y position.

The parallax effect is immediate and convincing. Your brain interprets the speed
differences as depth, and suddenly a flat 2D screen feels like a window into space.
This technique dates back to the earliest 8-bit demos and arcade games.

### 3. The Checkerboard Floor

Below the horizon line (at 75% of the screen height), a red-and-white checkerboard
stretches into the distance. This is a classic mode-7-style perspective floor.

For each scanline below the horizon, the renderer computes a depth value `z` using
inverse interpolation between a near plane and a far plane. Then for each pixel on that
scanline, it calculates the world-space X coordinate by dividing by a focal length. The
integer parts of the X and Z coordinates are summed, and the lowest bit determines
whether the tile is red or white. That is the entire algorithm -- no textures, no matrix
math, just a clever division per scanline and an XOR pattern.

The further a row is from the bottom of the screen, the higher its Z value, so tiles
appear to shrink as they recede toward the horizon. This approach was popularized by
SNES Mode 7 games and Amiga demos in the early 1990s.

### 4. The Fire Effect

Once the logo animation finishes, flames erupt along the horizon. This is the classic
demoscene fire algorithm, a cellular automaton that has been a staple of demo
productions since the early 1990s.

The fire lives in its own buffer (480 wide, 42 rows tall). Each cell holds a heat value
from 0 to 255. The algorithm works in two steps:

**Seed:** The bottom two rows are filled with random values every frame, creating the
fuel source.

**Propagate:** For every other cell, the algorithm averages four neighbors (left, center,
and right from the row below, plus the cell two rows below), then subtracts a small
decay constant. This average-and-decay propagates heat upward while cooling it, naturally
creating the tapered flame shapes.

A 256-entry palette maps heat values to colors: black fades to red, then to yellow, then
to white at maximum intensity. The result is a convincing, organic-looking fire with no
physics simulation at all -- just averaging and a good color ramp.

The fire updates at a fixed 20 Hz tick rate, independent of the rendering framerate, so
the flame speed stays consistent regardless of how fast the machine is.

### 5. The Procedural Chrome Font

The sine scroller needs text, and demoscene text needs to look good. Rather than loading
a font file, this demo generates every glyph procedurally at startup.

Each character is defined as a 5x7 bitmap -- seven rows of 5-bit masks, where each bit
is a pixel. The letter A, for example, is:

```
11111
10001
10001
11111
10001
10001
10001
```

These tiny bitmaps are scaled up 4x (to 20x28 pixels per glyph) and filled with a
multi-stop chrome gradient that runs vertically through each character: steel blue at the
top, a bright highlight band in the middle, warm copper at the bottom. Every other
scanline is dimmed by 18% to create a horizontal groove texture, mimicking the look of
brushed metal.

A bevel effect is added by detecting top and bottom edges of each glyph shape. The
topmost visible scanline of each block is brightened, the bottommost is darkened. This
gives the flat bitmap letters a subtle 3D appearance.

All 64 glyphs (space through underscore) are pre-rendered into a single flat atlas array
at init time. Drawing a character is just a rectangular copy from the atlas to the
framebuffer, skipping zero (transparent) pixels.

### 6. The Sine Scroller

The sine scroller is arguably the most iconic demoscene effect. A line of text scrolls
continuously from right to left, with each character riding a sine wave that gives the
whole message a fluid, undulating motion.

The scroller tiles seamlessly: the text is padded with enough spaces to fill one full
screen width, and the horizontal offset wraps using modulo arithmetic. Characters are
positioned along the X axis at regular intervals, and their Y position is offset by
`sin(time * 3.0 + x * 0.02) * 12.0` -- a sine function of both time and horizontal
position. This creates a wave that travels through the text as it scrolls.

The scroller text can be customized via a URL query parameter (`?text=YOUR+MESSAGE`) in
the browser version.

### 7. The Logo Particle Convergence

This is the showpiece animation. When the demo starts, the logo image is decomposed into
individual colored pixels. Each non-transparent pixel becomes a particle with a start
position and an end position.

The start positions are laid out in a uniform grid that covers the entire screen. The end
positions are the pixel's actual location in the final logo image. Over 4 seconds, each
particle interpolates from its grid position to its logo position using an ease-out curve
(`t * (2 - t)`), so the motion starts fast and decelerates smoothly.

During convergence, a sine wave distortion is applied to all particles. The wave amplitude
ramps up as the particles move, then decays over 2.5 seconds after they settle, giving
the logo a liquid wobble that gradually calms to stillness.

After the animation completes, the logo is drawn as a regular scaled sprite. Every 10
seconds, a brief sine-shake distortion ripples through the logo rows -- a quick horizontal
wobble driven by a sin-envelope pulse -- just to keep things lively.

### 8. The Sprite System

Sprites are simple: a width, height, a flat array of RGB565 pixel data, and a color key
(transparent color, taken from the top-left pixel of the source image). The `sprite_blit_scaled`
function draws a sprite at an arbitrary size using nearest-neighbor scaling, skipping any
pixel that matches the color key.

In the browser version, sprites are loaded from PNG files at runtime via an offscreen
canvas. In the C version, a Python script (`scripts/png2rgb565.py`) pre-converts the PNG
into a C header file containing a `const uint16_t` array that lives in flash.

## How the Effects Layer Together

The main loop renders everything back-to-front in painter's order:

1. Clear the framebuffer to black
2. Draw the starfield (single pixels in the sky area, above the horizon)
3. Draw the fire (only after the logo animation has finished)
4. Draw the checkerboard floor (covers the lower quarter, overlapping the fire slightly)
5. Draw the logo (particles during convergence, sprite after)
6. Draw the sine scroller (chrome text on top of everything)

This layering means the floor naturally overlaps the base of the fire, the logo sits on
top of the floor, and the scroller text floats above it all. No alpha blending, no
z-buffer -- just careful ordering.

## The C-Version on Zephyr (Bare Metal)

The C code that compiles to WASM in the browser is the same code that runs natively on an
**STM32H747I-DISCO** discovery board under Zephyr RTOS. The board has a 480x272 TFT-LCD
panel with an LTDC display controller -- the same native resolution the demo was designed
for. This is the original target: the Emscripten/WASM build is a zero-effort bonus you get
from keeping the platform bridge thin.

### Hardware details

- **MCU:** STM32H747XI dual-core (Cortex-M7 at 480 MHz + Cortex-M4). The demo runs
  entirely on the M7 core.
- **Display:** 480x272 RGB565 TFT-LCD driven by the on-chip LTDC controller via Zephyr's
  display API.
- **Memory:** Framebuffers and particle arrays are placed in external 32 MB SDRAM
  (at 0xD0000000) using a custom linker section (`.sdram_bss`). Internal SRAM would be
  too small for two 480x272 framebuffers plus ~40,000 particle coordinate arrays.
- **FPU:** The M7's single-precision hardware FPU is enabled (`CONFIG_FPU=y`) for all
  the sine, sqrt, and floating-point math the effects need.
- **Double buffering:** Two framebuffers alternate via `fb_swap()`. One is written to by
  the effects while the other is being displayed, avoiding tearing.

### Key differences from JS6

- **No `Math.random()`:** The C version uses a simple LCG (linear congruential generator)
  PRNG for fire seeding and star positions. Deterministic and fast, no OS entropy needed.
- **Logo data in flash:** Instead of loading a PNG at runtime, the logo is pre-converted
  to a C array by `scripts/png2rgb565.py` and compiled directly into the binary as
  `logo_data.h`.
- **Fixed-point-friendly structure:** All timing uses `k_uptime_get()` (millisecond kernel
  timer) converted to float seconds. Delta time is clamped to 100ms to survive debugger
  pauses.
- **SDRAM placement:** The `__attribute__((section(".sdram_bss")))` annotation and a custom
  linker fragment (`sdram.ld`) place large buffers in external SDRAM. The SDRAM region is
  marked non-cacheable via an MPU attribute in the device tree overlay for DMA coherency
  with the LTDC controller.
- **Frame pacing:** The main loop targets 60fps by sleeping for the remainder of each 16ms
  frame using `k_msleep()`.

## Building and Running

### JS6 (browser -- no build step)

No build step required:

```sh
./scripts/serve.sh
```

Or manually: `python3 -m http.server 8000` from the project root.

Open `http://localhost:8000` in a browser. The demo starts in JS6 mode by default. Use
the toggle tabs above the screen to switch between the JS6 and C-Version (WASM)
renderers. An optional MOD music player is included (press play in the UI).

You can customize the scroller text with a query parameter:

```
http://localhost:8000?text=HELLO+WORLD
```

### C-Version / WASM (browser -- same C source, compiled with Emscripten)

The same C source files that run on bare-metal Zephyr are compiled to WebAssembly. No
wrapper code, no separate browser port -- the actual `fire.c`, `stars.c`, `logo.c`, etc.
are fed to `emcc` and the platform bridge in `graph.c` handles the rest.

**Using the scripts (recommended):**

```sh
./scripts/setup.sh       # installs Emscripten SDK into emsdk/ (gitignored)
./scripts/build.sh       # compiles C to WASM
./scripts/serve.sh       # starts dev server, opens browser
```

**Or manually:**

1. Install Emscripten (if not already done):

```sh
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
source ./emsdk_env.sh
```

2. Build with make:

```sh
cd zephyr
make -f Makefile.web
```

This produces `web/demo.js` and `web/demo.wasm`.

3. Run: Serve the project root the same way as JS6. Click the **C-Version (WASM)**
tab above the screen, or navigate directly to `http://localhost:8000?mode=wasm`. The C
code runs as WebAssembly at near-native speed, using `requestAnimationFrame` for 60fps
frame pacing.

A standalone page at `zephyr/web/index.html` is also included for testing the WASM build
in isolation.

### C-Version / Zephyr (bare metal)

Requires a Zephyr SDK installation and the STM32H747I-DISCO board.

1. **Convert the logo to a C header** (if not already done):

```sh
cd zephyr
python3 scripts/png2rgb565.py logo.png > src/logo_data.h
```

2. **Build with west:**

```sh
west build -b stm32h747i_disco/stm32h747xx/m7 zephyr
```

3. **Flash:**

```sh
west flash
```

The demo will start on the board's LCD as soon as it boots.

### Troubleshooting the C build

The rendering code is a straightforward port and should work as-is. The risky parts are
all in the hardware/driver plumbing. Here is what to check when things go wrong.

**Black screen, no crash:**
The most likely cause is the display init path. The code uses
`DEVICE_DT_GET(DT_CHOSEN(zephyr_display))` to find the display device. If the board's
device tree does not set `zephyr,display` in the chosen node, or if the LCD shield is not
configured, this will fail silently. Check `printk` output over the debug UART -- if you
see "Display init failed", that is the problem. You may need to add
`--shield st_b_lcd40_dsi1_mb1166` to the build command, or verify the chosen node in
your board's device tree.

**Garbled or corrupted image:**
This usually means the framebuffer pixel format does not match what the LTDC/DSI driver
expects. The code writes raw RGB565 and passes it to `display_write()`. Some Zephyr
display drivers default to a different format (ARGB8888, BGR565, etc.). Check
`display_get_capabilities()` output and verify `current_pixel_format` is
`PIXEL_FORMAT_RGB_565`. You may need to add `display_set_pixel_format()` in `graph_init`.

**Hard fault or crash at startup:**
Almost certainly an SDRAM issue. The framebuffers and particle arrays are placed in
external SDRAM at 0xD0000000 via the `.sdram_bss` linker section. If the FMC/SDRAM
controller is not initialized before the application code runs, writes to those arrays
will fault. Verify that `CONFIG_MEMC=y` is set and that the `&sdram2` device tree node is
properly configured with correct FMC timing for your board revision. Also check that the
`memset` calls in `graph_init()` do not happen before the MEMC driver has run -- they
should be safe since `main()` runs after `POST_KERNEL` init, but a custom init level
override could break this.

**Tearing or stale frames:**
The SDRAM region must be non-cacheable for the LTDC DMA to read correct data. The board
overlay sets `zephyr,memory-attr = <( DT_MEM_ARM(ATTR_MPU_RAM_NOCACHE) )>` on the sdram2
node for this reason. If you see partial updates, old frames bleeding through, or
flickering horizontal bands, the MPU may not be applying the non-cacheable attribute
correctly. Verify with a debugger that the MPU region covering 0xD0000000 is configured
as non-cacheable.

**Build succeeds but linker warning about orphan `.sdram_bss` section:**
The linker fragment `sdram.ld` maps `.sdram_bss` into the `SDRAM2` memory region. This
requires that the device tree node has `zephyr,memory-region = "SDRAM2"` set, which
causes Zephyr to generate the corresponding linker MEMORY block. If you see an orphan
section warning, verify the overlay is being applied and the property is present.

**DSI-specific issues:**
The STM32H747I-DISCO uses a MIPI DSI interface between the LTDC and the LCD panel. The
LTDC feeds pixel data into a DSI host bridge, which serializes it to the panel. Zephyr's
DSI support on STM32H7 has historically required specific driver versions and
configuration. If the parallel-RGB path works but DSI does not, check the Zephyr version
and any known issues with the `st,stm32-ltdc` and DSI bridge drivers.

### Local syntax check (no Zephyr SDK required)

If you do not have a Zephyr SDK installed, you can still run a basic compilation test
using your host C compiler. This catches syntax errors, type mismatches, undeclared
identifiers, and redefined macros -- but not linker errors, ARM-specific issues, or
Zephyr API misuse.

Create minimal stub headers for the Zephyr includes:

```sh
mkdir -p /tmp/zstubs/zephyr/drivers
cat > /tmp/zstubs/zephyr/kernel.h << 'STUB'
typedef long long int64_t;
static inline int64_t k_uptime_get(void){return 0;}
static inline void k_msleep(int ms){}
static inline void printk(const char *fmt,...){}
STUB
cat > /tmp/zstubs/zephyr/device.h << 'STUB'
struct device;
static inline int device_is_ready(const struct device *d){return 1;}
STUB
cat > /tmp/zstubs/zephyr/drivers/display.h << 'STUB'
#define DT_CHOSEN(x) 0
#define DEVICE_DT_GET(x) ((const struct device*)0)
struct display_capabilities { int x_resolution; int y_resolution; };
struct display_buffer_descriptor { int buf_size; int width; int height; int pitch; };
static inline void display_blanking_off(const struct device *d){}
static inline void display_get_capabilities(const struct device *d, struct display_capabilities *c){}
static inline int display_write(const struct device *d, int x, int y, const struct display_buffer_descriptor *desc, const void *buf){return 0;}
STUB
```

Then syntax-check all source files. The `-DSYNTAX_CHECK` flag tells `graph.h` to skip
the `__attribute__((section))` annotation, which is ARM/ELF-only and would fail on macOS:

```sh
cd zephyr
for f in src/*.c; do
  echo "--- $f ---"
  cc -fsyntax-only -std=c11 -DSYNTAX_CHECK -I src -I /tmp/zstubs \
    -include stdint.h -include stdbool.h -include math.h -include string.h \
    "$f"
done
```

All files should report zero errors and zero warnings.

## Credits

Built by a human and an AI, working together.

Respect to the demoscene -- past, present, and future.
