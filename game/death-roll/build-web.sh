#!/bin/sh
# Build Death Roll for the web and put the result where the site serves it: static/play/death-roll/
# This is the same emcc command mray.py runs (so it builds wherever mray does), without needing a mray project.
#
#   sh game/death-roll/build-web.sh
#   RAYLIB=/some/other/raylib sh game/death-roll/build-web.sh      # default: ~/raylib-test/raylib
#
# Then commit static/play/death-roll/ : the site serves it at /play/death-roll/
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
RAYLIB="${RAYLIB:-$HOME/raylib-test/raylib}"
RL="$RAYLIB/src"
OUT="$HERE/../../static/play/death-roll"

command -v emcc >/dev/null 2>&1 || { echo "emcc not found: install or activate emscripten first."; exit 1; }
[ -f "$RL/libraylib.a" ] || { echo "No $RL/libraylib.a: point RAYLIB at raylib's web build (the folder that contains src/)."; exit 1; }

mkdir -p "$OUT"
cd "$HERE"

# every .c file except raylib's own and the tests (the tests are separate programs with their own main)
SRCS=$(find . -name '*.c' -not -path './raylib/*' -not -path './tests/*' | sort)
echo "Compiling: $SRCS"

rm -f "$OUT/index.html" "$OUT/index.js" "$OUT/index.wasm" "$OUT/index.data"
emcc $SRCS "$RL/libraylib.a" -o "$OUT/index.html" \
    -Wall -Wno-missing-braces -O2 -std=c99 -I"$RL" -I./src -I. \
    -s USE_GLFW=3 -s ASYNCIFY=1 -s ALLOW_MEMORY_GROWTH=1 -s WASM=1 \
    --shell-file raylib/src/shell.html --preload-file assets

echo
echo "Built into static/play/death-roll/:"
ls -l "$OUT"
echo
echo "Try it:  cd static/play/death-roll && python3 -m http.server 8080   (add ?debug to the URL for the overlay)"
