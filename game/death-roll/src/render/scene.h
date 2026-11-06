// ============================================================================
// Everything 3D: camera, terrain, obstacles, shadow, the ball. The sim runs in world coordinates (s grows without limit); the
// scene draws everything RELATIVE TO THE BALL, so no coordinate the GPU sees is ever large (see terrain_render.h).
// raylib convention: forward is -Z, so the sim's s becomes -z.
// ============================================================================
#ifndef DR_SCENE_H
#define DR_SCENE_H

#include "../sim/game.h"
#include "raylib.h"
#include <stdbool.h>

typedef struct {
    int  obstacles_drawn;
    int  chunks_rebuilt;
    bool world_shader_ok;
    bool hud_shader_ok;
} SceneStats;

void scene_init(void);                                       // after InitWindow
void scene_reset(const Game *g);                             // a new run: rebuild the ground, snap the camera
void scene_update(const Game *g, float dt, uint32_t events, bool portrait);   // camera, spin, shake, ground streaming
void scene_draw3d(Game *g, int sw, int sh);
void scene_begin_2d(void);                                   // switch to the highp 2D shader for the HUD
void scene_end_2d(void);
Color scene_sky(void);
const SceneStats *scene_stats(void);

#endif
