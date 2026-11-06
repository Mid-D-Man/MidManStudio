#include "terrain.h"

static uint32_t g_seed = 1;

uint32_t dr_hash3(uint32_t a, uint32_t b, uint32_t c)
{
    uint32_t h = (a * 0x9E3779B1u) ^ ((b + 0x85EBCA6Bu) * 0xC2B2AE35u) ^ ((c + 0x27D4EB2Fu) * 0x165667B1u);
    h ^= h >> 15; h *= 0x2C1B3C6Du;
    h ^= h >> 12; h *= 0x297A2D39u;
    h ^= h >> 15;
    return h;
}

double dr_unit(uint32_t h) { return (double)(h >> 8) * (1.0 / 16777216.0); }

void     terrain_set_seed(uint32_t seed) { g_seed = seed; }
uint32_t terrain_seed(void)              { return g_seed; }

static double smooth01(double t) { t = dr_clamp(t, 0.0, 1.0); return t * t * (3.0 - 2.0 * t); }

// Value noise in [-1, 1]
static double corner(int ix, int iy, uint32_t salt)
{
    return dr_unit(dr_hash3((uint32_t)ix, (uint32_t)iy, salt ^ (g_seed * 0x9E3779B9u))) * 2.0 - 1.0;
}

static double vnoise2(double x, double y, uint32_t salt)
{
    const double fx0 = floor(x), fy0 = floor(y);
    const int ix = (int)fx0, iy = (int)fy0;
    const double ux = smooth01(x - fx0), uy = smooth01(y - fy0);
    const double a = corner(ix, iy, salt),     b = corner(ix + 1, iy, salt);
    const double c = corner(ix, iy + 1, salt), d = corner(ix + 1, iy + 1, salt);
    return (a + (b - a) * ux) + ((c + (d - c) * ux) - (a + (b - a) * ux)) * uy;
}

// ---- ramps ----------------------------------------------------------------------------------------------------------
#define RAMP_CHANCE 0.5

Ramp terrain_ramp(int cell)
{
    Ramp r;
    r.exists = false; r.s0 = 0.0; r.x = 0.0; r.halfw = 0.0; r.len = 0.0; r.height = 0.0;
    if (cell < 1) return r;                                   // the first 90 m are plain slope
    const uint32_t c = (uint32_t)cell;
    if (dr_unit(dr_hash3(c, 0x51u, g_seed)) > RAMP_CHANCE) return r;
    r.exists = true;
    r.s0     = cell * DR_RAMP_CELL + 22.0 + dr_unit(dr_hash3(c, 0x52u, g_seed)) * 45.0;
    r.x      = (dr_unit(dr_hash3(c, 0x53u, g_seed)) - 0.5) * 16.0;
    r.halfw  = 4.2 + dr_unit(dr_hash3(c, 0x54u, g_seed)) * 2.2;
    r.len    = 9.0 * (0.9 + 0.3 * dr_unit(dr_hash3(c, 0x55u, g_seed)));
    r.height = 1.6 + 1.4 * dr_unit(dr_hash3(c, 0x56u, g_seed));
    return r;
}

static double ramp_height(const Ramp *r, double x, double s)
{
    if (!r->exists) return 0.0;
    const double u = (s - r->s0) / r->len;
    if (u < 0.0 || u > 1.0) return 0.0;                       // past u = 1 the ground drops away: the lip
    const double t = 1.0 - fabs(x - r->x) / r->halfw;
    if (t <= 0.0) return 0.0;
    return r->height * pow(u, 1.6) * smooth01(t / 0.4);
}

static double ramp_at(double x, double s)
{
    const int cell = (int)floor(s / DR_RAMP_CELL);
    const Ramp r = terrain_ramp(cell);                        // a ramp never crosses its own cell's end
    return ramp_height(&r, x, s);
}

double terrain_ramp_mask(double x, double s)
{
    const int cell = (int)floor(s / DR_RAMP_CELL);
    const Ramp r = terrain_ramp(cell);
    if (!r.exists || r.height <= 0.0) return 0.0;
    return dr_clamp(ramp_height(&r, x, s) / r.height, 0.0, 1.0);
}

// A ramp keeps a band clear of obstacles: 14 m of approach, the ramp, and 75 m beyond the lip (a fast jump carries about
// 70 m). The band is as wide as the ramp plus 4.5 m each side (plus the obstacle's own radius via `margin`). Only the band
// is kept clear, not the whole track: the rest of the slope keeps its obstacles.
bool terrain_in_ramp_corridor(double x, double s, double margin)
{
    const int c = (int)floor(s / DR_RAMP_CELL);
    for (int k = c - 1; k <= c; k++) {                        // a landing stretch can spill into the next cell
        const Ramp r = terrain_ramp(k);
        if (r.exists && s >= r.s0 - 14.0 && s <= r.s0 + r.len + 75.0 && fabs(x - r.x) < r.halfw + 4.5 + margin) return true;
    }
    return false;
}

// ---- the height field -----------------------------------------------------------------------------------------------
static double bank(double x)
{
    const double ax = fabs(x) - DR_TRACK_HALF;
    return ax > 0.0 ? 0.10 * ax * ax + 0.35 * ax : 0.0;
}

double terrain_height(double x, double s)
{
    const double fade  = smooth01(s / 80.0);                  // plain slope for the first stretch
    const double hills = DR_HILL_AMP * (vnoise2(x * 0.05, s * 0.018, 1u) + 0.35 * vnoise2(x * 0.12, s * 0.05, 2u));
    return -DR_SLOPE * s + hills * fade + ramp_at(x, s) + bank(x);
}

void terrain_gradient(double x, double s, double *dhdx, double *dhds)
{
    const double e = 0.2;
    double gx = (terrain_height(x + e, s) - terrain_height(x - e, s)) / (2.0 * e);
    double gs = (terrain_height(x, s + e) - terrain_height(x, s - e)) / (2.0 * e);
    *dhdx = dr_clamp(gx, -0.9, 0.9);                          // a ramp's lip is a cliff: never feed that into the physics
    *dhds = dr_clamp(gs, -0.9, 0.9);
}

double terrain_lane_center(double s)
{
    return 9.0 * vnoise2(s / 130.0, 0.37, 3u);
}
