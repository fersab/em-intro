#!/usr/bin/env bash
# build.sh — build em-intro targets
# Usage: ./scripts/build.sh [--wasm] [--logo] [--zephyr] [--all] [--clean]
#        No args defaults to --wasm
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ZDIR="$ROOT/zephyr"

# ---------- helpers ----------

die() { echo "error: $*" >&2; exit 1; }
has() { command -v "$1" &>/dev/null; }

activate_emsdk() {
    if has emcc; then return 0; fi
    local emsdk_dir="${EMSDK:-$ROOT/emsdk}"
    if [[ -f "$emsdk_dir/emsdk_env.sh" ]]; then
        # shellcheck disable=SC1091
        source "$emsdk_dir/emsdk_env.sh" || die "Failed to source emsdk_env.sh"
        return 0
    fi
    die "emcc not found. Run ./scripts/setup.sh first."
}

# ---------- build targets ----------

build_wasm() {
    echo "==> Building WASM"
    activate_emsdk
    make -C "$ZDIR" -f Makefile.web || die "WASM build failed. Check emcc with: emcc --version"
    if [[ ! -f "$ZDIR/web/demo.wasm" ]]; then
        die "Build completed but demo.wasm not found"
    fi
    echo ""
}

build_logo() {
    echo "==> Generating logo_data.h"
    if ! python3 -c "import PIL" 2>/dev/null; then
        die "Pillow not installed. Run: python3 -m pip install Pillow"
    fi
    python3 "$ZDIR/scripts/png2rgb565.py" "$ZDIR/logo.png" > "$ZDIR/src/logo_data.h"
    echo "    wrote $ZDIR/src/logo_data.h"
    echo ""
}

build_zephyr() {
    echo "==> Building Zephyr firmware"
    if ! has west; then
        die "west not found. Install Zephyr SDK: https://docs.zephyrproject.org/latest/develop/getting_started/"
    fi
    west build -b stm32h747i_disco/stm32h747xx/m7 "$ZDIR" || die "Zephyr build failed"
    echo ""
}

do_clean() {
    echo "==> Cleaning build artifacts"
    make -C "$ZDIR" -f Makefile.web clean
    if [[ -d "$ROOT/build" ]]; then
        echo "    removing build/ (Zephyr)"
        rm -rf "$ROOT/build"
    fi
    echo "    done"
}

# ---------- parse args ----------

DO_WASM=0
DO_LOGO=0
DO_ZEPHYR=0
DO_CLEAN=0

if [[ $# -eq 0 ]]; then
    DO_WASM=1
fi

while [[ $# -gt 0 ]]; do
    case "$1" in
        --wasm)   DO_WASM=1 ;;
        --logo)   DO_LOGO=1 ;;
        --zephyr) DO_ZEPHYR=1 ;;
        --all)    DO_WASM=1; DO_LOGO=1; DO_ZEPHYR=1 ;;
        --clean)  DO_CLEAN=1 ;;
        -h|--help)
            echo "Usage: $0 [--wasm] [--logo] [--zephyr] [--all] [--clean]"
            echo "  --wasm     Build C to WebAssembly via Emscripten (default)"
            echo "  --logo     Regenerate zephyr/src/logo_data.h from logo.png"
            echo "  --zephyr   Build Zephyr firmware (requires west + Zephyr SDK)"
            echo "  --all      Build all targets"
            echo "  --clean    Remove build artifacts"
            exit 0
            ;;
        *) die "Unknown option: $1 (try --help)" ;;
    esac
    shift
done

# ---------- run ----------

if [[ $DO_CLEAN -eq 1 ]]; then do_clean; fi
if [[ $DO_LOGO -eq 1 ]];  then build_logo; fi
if [[ $DO_WASM -eq 1 ]];  then build_wasm; fi
if [[ $DO_ZEPHYR -eq 1 ]]; then build_zephyr; fi

echo "Build finished."
