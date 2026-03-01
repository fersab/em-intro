#!/usr/bin/env bash
# serve.sh — start a local dev server for em-intro
# Usage: ./scripts/serve.sh [--port N]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PORT=8000

while [[ $# -gt 0 ]]; do
    case "$1" in
        --port) PORT="$2"; shift ;;
        -h|--help)
            echo "Usage: $0 [--port N]"
            echo "  Starts a local HTTP server (default port 8000)"
            exit 0
            ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
    shift
done

URL="http://localhost:$PORT"
echo "Serving em-intro at $URL"
echo "  JS6 mode:           $URL"
echo "  C-Version (WASM):   $URL?mode=wasm"
echo ""

# Open browser in background
OS="$(uname -s)"
if [[ "$OS" == "Darwin" ]]; then
    open "$URL" 2>/dev/null &
elif [[ "$OS" == "Linux" ]] && command -v xdg-open &>/dev/null; then
    xdg-open "$URL" 2>/dev/null &
fi

cd "$ROOT"
python3 -m http.server "$PORT"
