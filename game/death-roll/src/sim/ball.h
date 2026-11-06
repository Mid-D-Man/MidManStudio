// ============================================================================
// The ball: a point that follows the analytic ground, and leaves it when the ground falls away faster than free fall.
// No physics engine, no collision mesh. Fixed timestep (see game.c).
// ============================================================================
#ifndef DR_BALL_H
#define DR_BALL_H

#include "sim_common.h"

#define GE_LAUNCH       1u      // left the ground
#define GE_LAND         2u      // came back down hard (Game.last_impact says how hard)
#define GE_SHIELD_HIT   4u
#define GE_DEATH        8u

typedef struct {
    double x, y, s;             // position; y is the CENTRE of the ball
    double vx, vs, vy;          // sideways, forward (downhill), vertical
    bool   grounded;
    double air_time;
    double last_impact;         // how hard the last landing was (m/s into the ground)
} Ball;

void ball_init(Ball *b);
// steer in [-1, 1]; difficulty in [0, 1]; brake >= 0 slows the ball (used after a crash)
void ball_step(Ball *b, double dt, double steer, double difficulty, double brake, uint32_t *events);

#endif
