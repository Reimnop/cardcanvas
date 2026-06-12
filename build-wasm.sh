#!/usr/bin/env bash
set -euo pipefail

WASI_SDK=${WASI_SDK:-/opt/wasi-sdk}

if [ ! -d "$WASI_SDK" ]; then
  echo "wasi-sdk not found at $WASI_SDK"
  echo "Install it from https://github.com/WebAssembly/wasi-sdk/releases"
  echo "or set the WASI_SDK environment variable to its location"
  exit 1
fi

mkdir -p wasm

"$WASI_SDK/bin/clang" \
  --target=wasm32 \
  -O2 \
  -fno-builtin \
  -nostdlib \
  -Wl,--no-entry \
  -Wl,--export-all \
  -Wl,--strip-all \
  -o wasm/card.wasm \
  native/encoding.c \
  native/glyph_table.c \
  native/canvas.c \
  native/color.c \
  native/walloc.c

echo "Built wasm/card.wasm ($(wc -c < wasm/card.wasm) bytes)"

echo "Generating TypeScript module for the WASM module"
npm run gen-wasm-module
