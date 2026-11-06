#include "world.h"
#include "terrain.h"
#include <limits.h>

double world_difficulty(double s) { return dr_clamp(s / 6000.0, 0.0, 1.0); }
double world_safe_halfwidth(double d) { return 3.8 - 1.0 * d; }

void world_reset(World *w)
{
    w->empty = false;
    for (int i = 0; i < DR_CELL_CACHE; i++) { w->cells[i].index = INT_MIN; w->cells[i].count = 0; }
}

static void gen_cell(WorldCell *c, int k, bool empty)
{
    c->index = k;
    c->count = 0;
    if (k < 0 || empty) return;
    const uint32_t seed = terrain_seed();
    const uint32_t ku = (uint32_t)k;
    const double s0 = k * DR_CELL_LEN;
    const double d = world_difficulty(s0);
    const double safe = world_safe_halfwidth(d);

    // 2..3 attempts early, up to 2..7 late, scaled by how much wider the spawn range is than the old +-(track + 3.5) so the
    // density per metre of width stays the same (the fraction is rounded up at random, deterministically per cell)
    const double wide = DR_SPAWN_HALF / (DR_TRACK_HALF + 3.5);
    const int    base = 2 + (int)(dr_unit(dr_hash3(ku, 0x77u, seed)) * (2.0 + 4.0 * d));
    const double want = base * wide;
    int attempts = (int)want;
    if (dr_unit(dr_hash3(ku, 0x78u, seed)) < want - attempts) attempts++;
    for (int i = 0; i < attempts && c->count < DR_MAX_OBS; i++) {
        const uint32_t iu = (uint32_t)i;
        const double ox   = (dr_unit(dr_hash3(ku, 0x100u + iu, seed)) * 2.0 - 1.0) * DR_SPAWN_HALF;
        const double os   = s0 + dr_unit(dr_hash3(ku, 0x200u + iu, seed)) * DR_CELL_LEN;
        const bool   rock = dr_unit(dr_hash3(ku, 0x300u + iu, seed)) < 0.35;
        const double u    = dr_unit(dr_hash3(ku, 0x400u + iu, seed));
        const double v    = dr_unit(dr_hash3(ku, 0x500u + iu, seed));

        const float r = rock ? (float)(0.7 + 0.7 * u)  : (float)(0.55 + 0.25 * u);
        const float h = rock ? (float)(r * (0.9 + 0.3 * v)) : (float)(5.0 + 3.0 * v);

        if (os < 70.0) continue;                                         // a clear start
        if (terrain_in_ramp_corridor(ox, os, r)) continue;               // a clear run-up and landing
        if (fabs(ox - terrain_lane_center(os)) < safe + r + 0.3) continue;   // keep the lane open

        bool crowded = false;
        for (int j = 0; j < c->count; j++) {
            const double dx = ox - c->obs[j].x, ds = os - c->obs[j].s;
            if (dx * dx + ds * ds < 2.2 * 2.2) { crowded = true; break; }
        }
        if (crowded) continue;

        Obstacle *o = &c->obs[c->count++];
        o->x = ox; o->s = os; o->r = r; o->h = h;
        o->type = rock ? OBS_ROCK : OBS_TREE;
        o->destroyed = false;
    }
}

WorldCell *world_cell(World *w, int index)
{
    WorldCell *c = &w->cells[((index % DR_CELL_CACHE) + DR_CELL_CACHE) % DR_CELL_CACHE];
    if (c->index != index) gen_cell(c, index, w->empty);
    return c;
}
