// ============================================================================
// One run: the ball, the generated world, shields, score and death. Pure simulation (no raylib, no input device): the
// caller supplies steer in [-1, 1] each fixed step and reads events back.
// ============================================================================
#ifndef DR_GAME_H
#define DR_GAME_H

#include "ball.h"
#include "world.h"

#define DR_MAX_SHIELDS  3
#define DR_STEP         (1.0 / 120.0)     // the fixed simulation step

typedef struct {
    Ball     ball;
    World    world;
    uint32_t seed;
    double   time;            // seconds since the run began
    double   dist;            // furthest point reached (m)
    int      shield;          // 0..DR_MAX_SHIELDS; a hit with one left slows you instead of ending the run
    double   invuln;          // seconds of grace after a shield hit
    bool     dead;
    double   death_time;
    uint32_t events;          // GE_* bits since the caller last cleared them
} Game;

void game_reset(Game *g, uint32_t seed);
void game_step(Game *g, double dt, double steer);
double game_speed(const Game *g);

#endif
