// ============================================================================
// Shaders. On the web raylib's own default vertex shader is mediump (16 bit on a phone GPU), which tears small geometry
// apart (see docs/phone-rendering.md in the Tower of Babylon repo). So everything here draws with highp shaders of its own:
//   hud   - a copy of raylib's default shader with the precision raised; used for 2D and for the small immediate-mode bits.
//   world - the same plus distance fog, vertex colours, texture and an optional light (used by the ball only).
// If either fails to compile the game still runs (raylib's default shader, no fog) and the debug overlay says so.
// ============================================================================
#ifndef DR_SHADERS_H
#define DR_SHADERS_H

#include "raylib.h"
#include <stdbool.h>

typedef struct {
    Shader hud;
    Shader world;
    bool   hud_ok;
    bool   world_ok;
    int    loc_fog_color;
    int    loc_fog_range;
    int    loc_light_dir;
    int    loc_use_light;
} Shaders;

void shaders_init(Shaders *s);
void shaders_use_light(const Shaders *s, bool on);   // lights the next DrawMesh by its normals (the ball); off for baked colours

#endif
