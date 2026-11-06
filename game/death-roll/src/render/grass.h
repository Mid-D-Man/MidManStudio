// ============================================================================
// Procedural grass for the ground. No raylib in here, so the tests can check it without a GL context.
//   grass_texel  one pixel of a small tileable detail texture (blade streaks along the slope, flecks, a few dry blades).
//   grass_patch  the smooth, seed-dependent patch colour at a ground position (lighter and darker lawn, no lighting).
// The ground mesh multiplies the two: patch colour * lighting in the vertex colours, the texture on top of it. Both depend
// only on the WORLD position (the texture through repeating coordinates), so chunk edges meet exactly and no seam shows.
// ============================================================================
#ifndef DR_GRASS_H
#define DR_GRASS_H

#define GRASS_TILE_PX   128       // power of two: WebGL 1 only repeats and mipmaps those
#define GRASS_TILE_M    8.0       // metres of ground per tile; divides the 32 m chunk length, so chunk starts are tile edges
#define GRASS_TEX_GAIN  1.17f     // the texture averages about 0.86, grass_patch scales by this so the lawn keeps its brightness

// Any integer coordinates; repeats every GRASS_TILE_PX in both directions.
void grass_texel(int px, int py, unsigned char rgb[3]);

// Colour (0..255, floats) of the bare lawn at ground position (x across, s downhill). Changes with terrain_seed().
void grass_patch(double x, double s, float rgb[3]);

#endif
