// Small generated meshes and the ball's texture. Nothing is loaded from disk except the logo image.
#ifndef DR_MESHES_H
#define DR_MESHES_H

#include "raylib.h"

// Obstacle shapes. Variant v is 0..DR_TREE_VARIANTS-1 / 0..DR_ROCK_VARIANTS-1 (sim_common.h); an out-of-range v gives variant 0.
// Every variant stays inside the same footprint and height as variant 0, so the collision (which only knows r and h) fits all of them.
Mesh      meshes_build_tree_variant(int v);  // unit height 1, base at y = 0, canopy radius <= 0.30; vertex colours (faceted)
Mesh      meshes_build_rock_variant(int v);  // unit radius, y -1..1 around the origin; vertex colours (faceted)
Mesh      meshes_build_tree(void);           // = variant 0
Mesh      meshes_build_rock(void);           // = variant 0
Texture2D meshes_build_ball_texture(void);   // pearl body with the studio logo around it (assets/logo.png)
Texture2D meshes_build_grass_texture(void);  // the tiling detail texture of the ground (grass.c): repeat wrap, mipmapped

#endif
