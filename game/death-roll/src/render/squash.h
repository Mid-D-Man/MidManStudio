// ============================================================================
// Cartoon squash and stretch for the ball. PURELY VISUAL: it only reads what the simulation did (grounded or not, speed, how
// hard the last landing was, the event bits) and produces a few scale factors for the ball's MESH. Nothing here is ever fed
// back into src/sim/, so the physics, the collisions and the hit radius are exactly what they were (the ball is still a sphere
// of DR_BALL_R as far as the game is concerned).
//
// Two independent effects, multiplied together by the scene:
//   1. an "impact" spring along the ground's normal: a landing snaps it flat (squashed), it then overshoots, wobbles and
//      settles, like jelly; a take-off gives it a stretch pop. Anchored to the contact point so the ball never sinks into the
//      ground or hovers above it while squashed.
//   2. a "motion" stretch along the direction of travel that grows with speed: strongest in the air, a hint on the ground.
// Both keep the volume constant: the other two axes scale by 1 / sqrt(factor).
//
// Plain C, no raylib: tests/test_render.c exercises it headlessly.
// ============================================================================
#ifndef DR_SQUASH_H
#define DR_SQUASH_H

#include <stdbool.h>

typedef struct {
    float e, ev;          // impact spring: e < 0 squashed along the ground normal, e > 0 stretched; ev is its velocity
    float m;              // motion stretch along the travel direction (>= 0), smoothed
    float ground;         // 0..1: how much the ball is standing on the ground (the anchor fades out after take-off)
    float dir[3];         // smoothed travel direction, render space (unit length)
    bool  primed;         // false until the first update has seeded dir
} Squash;

typedef struct {
    bool  grounded;       // the ball is on the ground this frame
    bool  launched;       // it left the ground since the last update
    bool  landed;         // it came down since the last update
    bool  hit;            // a shield absorbed a hit
    bool  died;           // the run just ended
    float speed;          // m/s
    float impact;         // m/s into the ground on the last landing (Ball.last_impact)
    float vel[3];         // velocity in render space (x across, y up, -forward)
} SquashInput;

void  squash_reset(Squash *q);
void  squash_update(Squash *q, float dt, const SquashInput *in);

float squash_along_normal(const Squash *q);   // scale along the ground normal: < 1 flattened, > 1 stretched
float squash_along_motion(const Squash *q);   // scale along the direction of travel (>= 1)
float squash_across(float along);             // the scale on the two other axes that keeps the volume: 1 / sqrt(along)
float squash_anchor(const Squash *q);         // how far (in ball radii, along the normal) to move the centre to keep the contact point

#endif
