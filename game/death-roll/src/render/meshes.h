// Small generated meshes and the ball's texture. Nothing is loaded from disk except the logo image.
#ifndef DR_MESHES_H
#define DR_MESHES_H

#include "raylib.h"

Mesh      meshes_build_tree(void);      // unit height 1, base at y = 0, canopy base radius 0.30; vertex colours (faceted)
Mesh      meshes_build_rock(void);      // unit radius, centred on the origin; vertex colours (faceted)
Texture2D meshes_build_ball_texture(void);   // pearl body with the studio logo around it (assets/logo.png)

#endif
