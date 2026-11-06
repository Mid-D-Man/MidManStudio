#include "grass.h"
#include "../sim/sim_common.h"
#include "../sim/terrain.h"
#include <math.h>
#include <stdint.h>

static double smooth01(double t) { return t * t * (3.0 - 2.0 * t); }

static double lattice(int ix, int iy, uint32_t salt) { return dr_unit(dr_hash3((uint32_t)ix, (uint32_t)iy, salt)) * 2.0 - 1.0; }

// Value noise in [-1, 1] that repeats every pu x pv cells: the lattice indices wrap, so the tile has no seam.
static double tiled_noise(double u, double v, int pu, int pv, uint32_t salt)
{
    const double fu = floor(u), fv = floor(v);
    const double tu = smooth01(u - fu), tv = smooth01(v - fv);
    const int u0 = (((int)fu % pu) + pu) % pu, v0 = (((int)fv % pv) + pv) % pv;
    const int u1 = (u0 + 1) % pu,              v1 = (v0 + 1) % pv;
    const double a = lattice(u0, v0, salt), b = lattice(u1, v0, salt);
    const double c = lattice(u0, v1, salt), d = lattice(u1, v1, salt);
    const double top = a + (b - a) * tu, bot = c + (d - c) * tu;
    return top + (bot - top) * tv;
}

// Plain (non repeating) value noise in [-1, 1] for the patches.
static double open_noise(double x, double y, uint32_t salt)
{
    const double fx = floor(x), fy = floor(y);
    const double tx = smooth01(x - fx), ty = smooth01(y - fy);
    const int ix = (int)fx, iy = (int)fy;
    const double a = lattice(ix, iy, salt),     b = lattice(ix + 1, iy, salt);
    const double c = lattice(ix, iy + 1, salt), d = lattice(ix + 1, iy + 1, salt);
    const double top = a + (b - a) * tx, bot = c + (d - c) * tx;
    return top + (bot - top) * ty;
}

static unsigned char to_byte(double v) { v = v < 0.0 ? 0.0 : (v > 1.0 ? 1.0 : v); return (unsigned char)(v * 255.0 + 0.5); }

void grass_texel(int px, int py, unsigned char rgb[3])
{
    const int N = GRASS_TILE_PX;
    const int x = ((px % N) + N) % N, y = ((py % N) + N) % N;
    const double fx = (x + 0.5) / N, fy = (y + 0.5) / N;          // 0..1 across the tile; y runs along the slope

    const double patch = tiled_noise(fx * 3.0, fy * 3.0, 3, 3, 0x11u);                // a few soft light and dark areas per tile
    const double blade = 0.62 * tiled_noise(fx * 44.0, fy * 7.0, 44, 7, 0x12u)        // streaks: narrow across, long along the slope
                       + 0.38 * tiled_noise(fx * 88.0, fy * 15.0, 88, 15, 0x13u);
    const double fleck = dr_unit(dr_hash3((uint32_t)x, (uint32_t)y, 0x14u)) * 2.0 - 1.0;
    const double dry   = dr_unit(dr_hash3((uint32_t)x, (uint32_t)y, 0x15u)) < 0.010 ? 1.0 : 0.0;   // the odd pale blade

    const double lum = 0.86 + 0.045 * patch + 0.075 * blade + 0.030 * fleck + 0.09 * dry;
    rgb[0] = to_byte(lum * (1.00 + 0.07 * blade + 0.10 * dry));
    rgb[1] = to_byte(lum);
    rgb[2] = to_byte(lum * (0.93 - 0.05 * blade - 0.20 * dry));
}

static double lerpd(double a, double b, double t) { return a + (b - a) * t; }

void grass_patch(double x, double s, float rgb[3])
{
    const uint32_t salt = terrain_seed() * 0x9E3779B9u;           // every run gets its own pattern of patches
    const double n1 = open_noise(x / 26.0, s / 34.0, 0x21u ^ salt);
    const double n2 = open_noise(x / 8.5,  s / 10.5, 0x22u ^ salt);
    const double n3 = open_noise(x / 3.0,  s / 3.6,  0x23u ^ salt);
    double t = 0.5 + 0.34 * n1 + 0.20 * n2 + 0.06 * n3;
    t = t < 0.0 ? 0.0 : (t > 1.0 ? 1.0 : t);

    // dark, mid and light lawn
    static const double D[3] = { 36.0, 78.0, 32.0 }, M[3] = { 60.0, 112.0, 40.0 }, L[3] = { 98.0, 148.0, 54.0 };
    for (int k = 0; k < 3; k++) {
        const double c = t < 0.5 ? lerpd(D[k], M[k], t * 2.0) : lerpd(M[k], L[k], (t - 0.5) * 2.0);
        rgb[k] = (float)(c * GRASS_TEX_GAIN);
    }
}
