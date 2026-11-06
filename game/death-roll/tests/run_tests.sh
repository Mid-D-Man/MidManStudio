#!/bin/sh
# Headless tests. Need only a C compiler: no raylib library, no GL, no emscripten.   sh tests/run_tests.sh
#   1. the simulation (terrain, ball physics, obstacle generation, rules), with bots playing thousands of metres
#   2. the render-side geometry (terrain chunk meshes, generated meshes), with raylib's GPU calls stubbed
# RAYLIB_SRC points at raylib's headers; default: raylib/src if it has them, else ~/raylib-test/raylib/src.
set -e
cd "$(dirname "$0")/.."
RL="${RAYLIB_SRC:-}"
if [ -z "$RL" ]; then
    if [ -f raylib/src/raylib.h ]; then RL=raylib/src; else RL="$HOME/raylib-test/raylib/src"; fi
fi
OUT="${TMPDIR:-/tmp}"

echo "=== simulation ==="
cc -std=c99 -Wall -Wextra -Wundef -O2 -DDR_TESTS -Isrc/sim \
   src/sim/terrain.c src/sim/ball.c src/sim/world.c src/sim/game.c tests/test_sim.c -lm -o "$OUT/dr_test_sim"
"$OUT/dr_test_sim"

echo
echo "=== render geometry ==="
cc -std=c99 -Wall -Wextra -Wno-unused-parameter -O2 -DDR_RENDER_TESTS -isystem "$RL" -Isrc \
   src/render/terrain_render.c src/render/meshes.c src/sim/terrain.c tests/test_render.c -lm -o "$OUT/dr_test_render"
"$OUT/dr_test_render"
