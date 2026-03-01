# em-intro top-level Makefile
# Convenience wrapper around scripts/*.sh

.PHONY: setup build build-all logo serve clean help

help:
	@echo "make setup      — install dev dependencies (Emscripten, Pillow, etc.)"
	@echo "make build      — build WASM (default)"
	@echo "make build-all  — build WASM + logo + Zephyr"
	@echo "make logo       — regenerate logo_data.h from logo.png"
	@echo "make serve      — start local dev server on port 8000"
	@echo "make clean      — remove build artifacts"

setup:
	./scripts/setup.sh

build:
	./scripts/build.sh --wasm

build-all:
	./scripts/build.sh --all

logo:
	./scripts/build.sh --logo

serve:
	./scripts/serve.sh

clean:
	./scripts/build.sh --clean
