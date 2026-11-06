#!/bin/sh
# Compile every source file twice with -Wundef: once as the desktop build and once as the web build (against a stub
# <emscripten/emscripten.h>), so a typo in a DR_* capability macro, or a mistake inside a web-only branch, shows up here
# without needing emscripten.  Also proves mray's "compile every .c under the project" can link: partial-link all objects.
#   sh tests/platform_check.sh
set -e
cd "$(dirname "$0")/.."
RL="${RAYLIB_SRC:-}"
if [ -z "$RL" ]; then
    if [ -f raylib/src/raylib.h ]; then RL=raylib/src; else RL="$HOME/raylib-test/raylib/src"; fi
fi
OUT="${TMPDIR:-/tmp}/dr_platform_check"
rm -rf "$OUT"; mkdir -p "$OUT/desktop" "$OUT/web"
SRCS="$(find . -name '*.c' -not -path './raylib/*' -not -path './.git/*' | sed 's|^\./||' | sort)"

for cfg in desktop web; do
    if [ "$cfg" = web ]; then DEF="-D__EMSCRIPTEN__ -Itests/stubs -Wno-unused-parameter"; else DEF=""; fi
    echo "--- $cfg ---"
    n=0
    for f in $SRCS; do
        o="$OUT/$cfg/$(echo "$f" | tr '/.' '__').o"
        cc -std=c99 -Wall -Wextra -Wundef $DEF -O1 -isystem "$RL" -I. -Isrc -c "$f" -o "$o"
        n=$((n+1))
    done
    echo "$n files compiled clean"
done

# the project as mray builds it: every .c together must have exactly one main and no duplicate symbols
OBJS=""
for f in $SRCS; do OBJS="$OBJS $OUT/desktop/$(echo "$f" | tr '/.' '__').o"; done
ld -r $OBJS -o "$OUT/all.o"
echo "mray-style link of all $(echo $OBJS | wc -w) objects: no duplicate symbols"
