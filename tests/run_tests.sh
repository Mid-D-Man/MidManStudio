#!/bin/sh
# Headless tests. Need only a C compiler: no raylib library, no GL, no emscripten.   sh tests/run_tests.sh
#   1. the simulation (terrain, ball physics, obstacle generation, rules), with bots playing thousands of metres
#   2. the render-side geometry (terrain chunk meshes, generated meshes), with raylib's GPU calls stubbed
# RAYLIB_SRC points at raylib's headers; the default is the copy mray puts in every project.
set -e
cd "$(dirname "$0")/.."
RL="${RAYLIB_SRC:-raylib/src}"
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
