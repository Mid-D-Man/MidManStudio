// ============================================================================
// Shared constants and helpers for the simulation. The sim (src/sim/) has no raylib in it: it is plain C so it can be
// tested headlessly (tests/test_sim.c) and the numbers below are the ones to tune.
//
// Coordinates: x is across the slope (right is +x), s is distance travelled DOWNHILL (always increasing), y is up.
// The renderer maps s to -z (raylib's forward).
// ============================================================================
#ifndef DR_SIM_COMMON_H
#define DR_SIM_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define DR_PI 3.14159265358979323846

// ---- the slope ------------------------------------------------------------------------------------------------------
#define DR_SLOPE        0.33    // base gradient: metres of height lost per metre travelled (about 18 degrees)
#define DR_TRACK_HALF   15.0    // the flat-ish track is |x| <= this; beyond it the ground banks up into walls
#define DR_X_LIMIT      21.0    // the ball can never go beyond |x| = this
#define DR_HILL_AMP     3.0     // height of the rolling hills (m)
#define DR_RAMP_CELL    90.0    // ramps are decided once per 90 m of slope
#define DR_CELL_LEN     24.0    // obstacles are generated per 24 m cell
#define DR_TREE_VARIANTS 6      // different tree shapes/colours (render/meshes.c builds one mesh for each)
#define DR_ROCK_VARIANTS 8      // different rock shapes/colours

// ---- the ball (tuning) ----------------------------------------------------------------------------------------------
#define DR_BALL_R       1.0     // radius (m)
#define DR_GRAVITY      26.0    // m/s^2 (heavier than real: snappier jumps)
#define DR_SLOPE_ACCEL  24.0    // forward acceleration per unit of downhill gradient
#define DR_DRAG_Q       0.0068  // quadratic drag: terminal speed on the base slope is about sqrt(DR_SLOPE*DR_SLOPE_ACCEL/DR_DRAG_Q)
#define DR_STEER_ACCEL  40.0    // sideways acceleration at full steer (m/s^2)
#define DR_LAT_DRAG     4.2     // sideways damping (1/s): top sideways speed is STEER_ACCEL / LAT_DRAG
#define DR_LAT_SLOPE    0.7     // how hard a sideways slope pushes the ball downhill sideways
#define DR_AIR_CONTROL  0.35    // steering strength while airborne
#define DR_START_SPEED  14.0
#define DR_MIN_SPEED    10.0
#define DR_MAX_SPEED    70.0
#define DR_LIFT_EPS     0.02    // how far above the ground the free-fall path must be to count as airborne

static inline double dr_clamp(double v, double lo, double hi) { return v < lo ? lo : (v > hi ? hi : v); }

uint32_t dr_hash3(uint32_t a, uint32_t b, uint32_t c);   // well mixed 32 bit hash of three integers
double   dr_unit(uint32_t h);                            // hash -> [0, 1)

#endif
