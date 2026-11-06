#include "game.h"
#include "terrain.h"
#include <string.h>

void game_reset(Game *g, uint32_t seed)
{
    memset(g, 0, sizeof *g);
    g->seed = seed;
    terrain_set_seed(seed);
    world_reset(&g->world);
    ball_init(&g->ball);
}

double game_speed(const Game *g) { return g->ball.vs; }

static void collide(Game *g)
{
    Ball *b = &g->ball;
    const int k0 = (int)floor((b->s - 4.0) / DR_CELL_LEN);
    const int k1 = (int)floor((b->s + 4.0) / DR_CELL_LEN);
    for (int k = k0; k <= k1; k++) {
        WorldCell *c = world_cell(&g->world, k);
        for (int i = 0; i < c->count; i++) {
            Obstacle *o = &c->obs[i];
            if (o->destroyed) continue;
            const double reach = DR_BALL_R * 0.9 + o->r * (o->type == OBS_ROCK ? 0.92 : 0.85);   // a little forgiving
            const double dx = b->x - o->x, ds = b->s - o->s;
            if (dx * dx + ds * ds >= reach * reach) continue;
            const double top = terrain_height(o->x, o->s) + o->h;
            if (b->y - DR_BALL_R >= top - 0.15) continue;       // jumped over it
            if (g->invuln > 0.0) continue;

            if (g->shield > 0) {
                g->shield--;
                o->destroyed = true;
                b->vs *= 0.6;
                g->invuln = 0.7;
                g->events |= GE_SHIELD_HIT;
            } else {
                g->dead = true;
                g->death_time = 0.0;
                g->events |= GE_DEATH;
                return;
            }
        }
    }
}

void game_step(Game *g, double dt, double steer)
{
    if (g->dead) {                                              // roll to a stop
        g->death_time += dt;
        ball_step(&g->ball, dt, 0.0, 0.0, 5.0, &g->events);
        return;
    }
    const double d = world_difficulty(g->ball.s);
    ball_step(&g->ball, dt, dr_clamp(steer, -1.0, 1.0), d, 0.0, &g->events);
    g->time += dt;
    if (g->invuln > 0.0) g->invuln -= dt;
    if (g->ball.s > g->dist) g->dist = g->ball.s;
    collide(g);
}
