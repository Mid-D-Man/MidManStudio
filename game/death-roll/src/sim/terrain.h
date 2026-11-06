// ============================================================================
// The slope as a function: height(x, s) is computed, never stored, so the world is endless and costs nothing to keep.
// Base gradient + rolling hills (value noise) + ramps (kickers, one per ~90 m cell, sometimes) + banked walls at the sides.
// ============================================================================
#ifndef DR_TERRAIN_H
#define DR_TERRAIN_H

#include "sim_common.h"

typedef struct {
    bool   exists;
    double s0;        // where the ramp starts (m of travel)
    double x;         // centre across the slope
    double halfw;     // half width
    double len;       // run-up length; the ground then drops away at the lip
    double height;    // height at the lip
} Ramp;

void   terrain_set_seed(uint32_t seed);
uint32_t terrain_seed(void);

double terrain_height(double x, double s);
void   terrain_gradient(double x, double s, double *dhdx, double *dhds);   // clamped to +-0.9
Ramp   terrain_ramp(int cell);                       // the ramp (if any) of ramp cell `cell`
double terrain_ramp_mask(double x, double s);        // 0..1: how much of the ground here is a ramp (for colouring)
bool   terrain_in_ramp_corridor(double x, double s, double margin);   // inside a ramp's keep-clear corridor (approach, lip, landing)
double terrain_lane_center(double s);                // the always-clear path that obstacle generation leaves open

#endif
