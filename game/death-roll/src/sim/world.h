// ============================================================================
// Obstacles. The slope is cut into 24 m cells; each cell's trees and rocks are a pure function of (seed, cell number), so
// they can be generated on demand, thrown away behind the ball and regenerated identically. Two fairness rules:
//   1. A winding "lane" (terrain_lane_center) is always kept clear, wide enough for the ball plus a margin.
//   2. Nothing is placed in a ramp's keep-clear corridor (approach, lip, landing: see terrain_in_ramp_corridor), and
//      nothing in the first 70 m.
// ============================================================================
#ifndef DR_WORLD_H
#define DR_WORLD_H

#include "sim_common.h"

#define DR_MAX_OBS      10
#define DR_CELL_CACHE   40      // cells kept (40 x 24 m = 960 m): far more than the 300 m that is ever drawn

typedef enum { OBS_TREE = 0, OBS_ROCK = 1 } ObsType;

typedef struct {
    double x, s;
    float  r;             // radius of the footprint (m)
    float  h;             // height above the ground (m)
    uint8_t type;
    bool   destroyed;     // smashed through with a shield
} Obstacle;

typedef struct {
    int      index;
    int      count;
    Obstacle obs[DR_MAX_OBS];
} WorldCell;

typedef struct {
    WorldCell cells[DR_CELL_CACHE];
    bool      empty;       // true: generate no obstacles at all (tests, and a clear menu scene)
} World;

void       world_reset(World *w);                       // forget everything generated (call after terrain_set_seed)
WorldCell *world_cell(World *w, int index);             // generated on first use
double     world_difficulty(double s);                  // 0 at the start, 1 from 6000 m on
double     world_safe_halfwidth(double difficulty);     // half width of the always-clear lane

#endif
