#!/usr/bin/env bash
# setup.sh — install dev dependencies for em-intro (macOS + Linux)
# Idempotent: safe to re-run, skips already-installed tools.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OS="$(uname -s)"
OK="\033[32m✓\033[0m"
WARN="\033[33m!\033[0m"
FAIL="\033[31m✗\033[0m"

echo "em-intro dev setup ($OS)"
echo "========================"
echo ""

# ---------- helpers ----------

has() { command -v "$1" &>/dev/null; }

install_pkg() {
    local pkg="$1"
    if [[ "$OS" == "Darwin" ]]; then
        if has brew; then
            echo "  Installing $pkg via Homebrew..."
            brew install "$pkg"
        else
            echo -e "  $FAIL Homebrew not found. Install $pkg manually or install Homebrew first."
            return 1
        fi
    elif [[ "$OS" == "Linux" ]]; then
        if has apt-get; then
            echo "  Installing $pkg via apt..."
            sudo apt-get install -y "$pkg"
        elif has dnf; then
            echo "  Installing $pkg via dnf..."
            sudo dnf install -y "$pkg"
        elif has pacman; then
            echo "  Installing $pkg via pacman..."
            sudo pacman -S --noconfirm "$pkg"
        else
            echo -e "  $FAIL No supported package manager found. Install $pkg manually."
            return 1
        fi
    fi
}

# ---------- Python 3 ----------

echo "1. Python 3"
if has python3; then
    echo -e "   $OK python3 $(python3 --version 2>&1 | awk '{print $2}')"
else
    echo -e "   $WARN python3 not found, installing..."
    install_pkg python3
fi

# ---------- Pillow (for png2rgb565.py) ----------

echo "2. Pillow (Python)"
if python3 -c "import PIL" 2>/dev/null; then
    PIL_VER=$(python3 -c "import PIL; print(PIL.__version__)")
    echo -e "   $OK Pillow $PIL_VER"
else
    echo -e "   $WARN Pillow not found, installing..."
    python3 -m pip install --user Pillow
    echo -e "   $OK Pillow installed"
fi

# ---------- Emscripten ----------

echo "3. Emscripten SDK"

EMSDK_DIR="${EMSDK:-$ROOT/emsdk}"

# Check if emcc is already on PATH (system install or previously sourced)
if has emcc; then
    echo -e "   $OK emcc $(emcc --version 2>/dev/null | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+')"
# Check for emsdk locally or at $EMSDK
elif [[ -f "$EMSDK_DIR/emsdk_env.sh" ]]; then
    echo -e "   $OK emsdk found at $EMSDK_DIR (source emsdk/emsdk_env.sh to activate)"
else
    echo -e "   $WARN Emscripten not found, installing to $EMSDK_DIR ..."
    git clone https://github.com/emscripten-core/emsdk.git "$EMSDK_DIR"
    "$EMSDK_DIR/emsdk" install latest
    "$EMSDK_DIR/emsdk" activate latest
    echo -e "   $OK Emscripten installed (gitignored — not committed to repo)"
    echo "   Run: source emsdk/emsdk_env.sh   (to activate in this shell)"
fi

# ---------- Zephyr (optional, just check) ----------

echo "4. Zephyr SDK + west (optional — for bare-metal build only)"
if has west; then
    echo -e "   $OK west $(west --version 2>/dev/null)"
    if [[ -n "${ZEPHYR_BASE:-}" ]]; then
        echo -e "   $OK ZEPHYR_BASE=$ZEPHYR_BASE"
    else
        echo -e "   $WARN ZEPHYR_BASE not set (source zephyr-env.sh in your Zephyr install)"
    fi
else
    echo -e "   $WARN west not found — not needed for browser builds"
    echo "   To build for STM32, see: https://docs.zephyrproject.org/latest/develop/getting_started/"
fi

# ---------- Summary ----------

echo ""
echo "Setup complete. Quick start:"
echo "  ./scripts/build.sh          # build WASM"
echo "  ./scripts/serve.sh          # start dev server"
echo "  open http://localhost:8000   # view demo"
