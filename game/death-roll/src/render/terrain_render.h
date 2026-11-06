// ============================================================================
// The slope as meshes. The ground is cut into 32 m chunks; a fixed set of eleven (one behind the ball, nine ahead) is kept,
// each a flat-shaded triangle mesh with the lighting baked into vertex colours. As the ball moves on, the chunk that fell
// behind is rebuilt for the one coming up and re-uploaded in place (UpdateMeshBuffer). A chunk's vertices are relative to
// its own start, and the whole thing is drawn relative to the ball, so no coordinate the GPU sees is ever large.
// ============================================================================
#ifndef DR_TERRAIN_RENDER_H
#define DR_TERRAIN_RENDER_H

#include "raylib.h"

#define TR_SLOTS 11

typedef struct { int index; Mesh mesh; } TrSlot;

typedef struct {
    TrSlot slots[TR_SLOTS];
    int    rebuilt_total;       // chunks rebuilt since start (debug)
} TerrainRender;

void terrain_render_init(TerrainRender *t);
void terrain_render_invalidate(TerrainRender *t);                        // a new seed: everything must be rebuilt
void terrain_render_update(TerrainRender *t, double ball_s, int max_rebuild);   // max_rebuild <= 0: no limit
void terrain_render_draw(const TerrainRender *t, Material mat, double ball_y, double ball_s);

#endif
