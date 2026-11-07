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
   src/render/terrain_render.c src/render/meshes.c src/render/grass.c src/render/squash.c src/render/ball_xform.c \
   src/sim/terrain.c tests/test_render.c -lm -o "$OUT/dr_test_render"
"$OUT/dr_test_render"

echo
echo "=== the squash stays out of the simulation ==="
# squash.c / squash.h are pure maths over a few numbers the scene hands them; if one of them ever includes a sim header, the
# cartoon ball could start to feed back into the physics. This keeps that from happening quietly.
if grep -n '#include' src/render/squash.c src/render/squash.h src/render/ball_xform.c src/render/ball_xform.h | grep -E 'sim/|ball\.h|game\.h|world\.h|terrain\.h'; then
    echo "  FAIL: a squash file includes a simulation header"; exit 1
fi
echo "  ok: no simulation header is included by squash.c, squash.h, ball_xform.c or ball_xform.h"
# ... and the physics and the rules never read how an obstacle or the ball LOOKS (the fields, not the English words in comments)
if grep -n -E '(->|\.)(variant|yaw|stretch)\b|squash' src/sim/ball.c src/sim/game.c src/sim/terrain.c; then
    echo "  FAIL: the simulation reads a look-only field"; exit 1
fi
echo "  ok: ball.c, game.c and terrain.c never read an obstacle's variant, yaw or stretch, and never mention squash"
