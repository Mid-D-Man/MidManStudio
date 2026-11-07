// Headless tests for the simulation (src/sim). Built by tests/run_tests.sh with -DDR_TESTS.
// Outside that build this file is an empty translation unit, so mray (which compiles every .c file under the project) can
// never pick up a second main().
#ifdef DR_TESTS

#include "../src/sim/game.h"
#include "../src/sim/terrain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_fail = 0, g_checks = 0;
#define CHECK(cond, ...) do { g_checks++; if (!(cond)) { g_fail++; printf("  FAIL %s:%d  ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static bool finite3(const Ball *b) { return isfinite(b->x) && isfinite(b->y) && isfinite(b->s) && isfinite(b->vx) && isfinite(b->vs) && isfinite(b->vy); }

// ---- 1. generation is a pure function of (seed, cell) -------------------------------------------------------------
static void test_determinism(void)
{
    printf("determinism\n");
    World a, b;
    terrain_set_seed(1234); world_reset(&a);
    for (int k = 0; k < 120; k++) world_cell(&a, k);
    terrain_set_seed(1234); world_reset(&b);
    for (int k = 119; k >= 0; k--) world_cell(&b, k);              // other order, same world
    int same = 1;
    for (int k = 0; k < 120; k++) {
        WorldCell *ca = world_cell(&a, k), *cb = world_cell(&b, k);
        if (ca->count != cb->count) { same = 0; break; }
        for (int i = 0; i < ca->count; i++) if (ca->obs[i].x != cb->obs[i].x || ca->obs[i].s != cb->obs[i].s) same = 0;
    }
    CHECK(same, "same seed gave different obstacles");
    terrain_set_seed(99);
    CHECK(terrain_height(3.0, 500.0) == terrain_height(3.0, 500.0), "terrain not deterministic");
    const double h99 = terrain_height(3.0, 500.0);
    terrain_set_seed(100);
    CHECK(terrain_height(3.0, 500.0) != h99, "different seeds gave the same terrain");
}

// ---- 2. the ground is finite and smooth except at ramp lips ------------------------------------------------------
static void test_terrain(void)
{
    printf("terrain\n");
    int bigjumps = 0, nonlip = 0, ramps = 0;
    for (uint32_t seed = 1; seed <= 6; seed++) {
        terrain_set_seed(seed);
        for (int cell = 0; cell < 60; cell++) { Ramp r = terrain_ramp(cell); if (r.exists) ramps++; }
        for (double s = 0.0; s < 4000.0; s += 0.5) {
            for (double x = -DR_X_LIMIT; x <= DR_X_LIMIT; x += 3.0) {
                const double h0 = terrain_height(x, s), h1 = terrain_height(x, s + 0.5);
                CHECK(isfinite(h0), "non-finite height");
                if (fabs(h1 - h0 + 0.5 * DR_SLOPE) > 1.3) {       // a step far bigger than any hill or ramp run-up
                    bigjumps++;
                    int is_lip = 0;
                    for (int k = (int)(s / DR_RAMP_CELL) - 1; k <= (int)(s / DR_RAMP_CELL); k++) {
                        Ramp r = terrain_ramp(k);
                        if (r.exists && fabs(s + 0.5 - (r.s0 + r.len)) < 0.75) is_lip = 1;
                    }
                    if (!is_lip) nonlip++;
                }
            }
        }
    }
    CHECK(ramps > 100, "too few ramps (%d)", ramps);
    CHECK(bigjumps > 0, "no ramp lips found at all");
    CHECK(nonlip == 0, "%d sudden height steps that are not ramp lips", nonlip);
    terrain_set_seed(7);
    CHECK(fabs(terrain_height(0, 0) - 0.0) < 1e-9, "start is not at height 0 (%f)", terrain_height(0, 0));
    for (double s = 0; s < 70; s += 1.0) { Ramp r = terrain_ramp((int)(s / DR_RAMP_CELL)); CHECK(!r.exists, "ramp in the first 90 m"); }
}

// ---- 3. fairness rules of obstacle generation --------------------------------------------------------------------
static void test_fairness(void)
{
    printf("fairness\n");
    int total = 0, minimum_gap_ok = 1;
    for (uint32_t seed = 1; seed <= 20; seed++) {
        Game *g = (Game *)malloc(sizeof *g); game_reset(g, seed);
        for (int k = 0; k < 700; k++) {                          // ~16.8 km of slope
            WorldCell *c = world_cell(&g->world, k);
            const double d = world_difficulty(k * DR_CELL_LEN);
            for (int i = 0; i < c->count; i++) {
                const Obstacle *o = &c->obs[i]; total++;
                CHECK(o->s >= 70.0, "obstacle inside the clear start (s=%f)", o->s);
                CHECK(!terrain_in_ramp_corridor(o->x, o->s, o->r), "obstacle inside a ramp keep-clear corridor (s=%f)", o->s);
                const double gap = fabs(o->x - terrain_lane_center(o->s)) - o->r;
                if (gap < world_safe_halfwidth(d)) minimum_gap_ok = 0;
                CHECK(fabs(o->x) <= DR_SPAWN_HALF + 1e-9, "obstacle outside the spawn range");
            }
        }
        free(g);
    }
    CHECK(minimum_gap_ok, "an obstacle intrudes on the clear lane");
    CHECK(total > 5000, "suspiciously few obstacles (%d)", total);
    printf("  %d obstacles checked\n", total);
}

// ---- 4. a bot that follows the clear lane never dies; physics stays sane ------------------------------------------
static double lane_bot(const Game *g)
{
    const Ball *b = &g->ball;
    const double target = terrain_lane_center(b->s + 0.45 * b->vs);
    return dr_clamp(0.35 * (target - b->x) - 0.09 * b->vx, -1.0, 1.0);
}

static void test_lane_bot(void)
{
    printf("lane bot (never dies, sane physics)\n");
    Game *g = (Game *)malloc(sizeof *g);
    double worst_speed_min = 1e9, worst_speed_max = 0.0, sum_far = 0.0; long n_far = 0;
    int launches = 0, lands = 0; double max_air = 0.0;
    for (uint32_t seed = 1; seed <= 12; seed++) {
        game_reset(g, seed);
        double last_air = 0.0; bool was_air = false;
        long steps = 0;
        while (g->ball.s < 20000.0 && steps < 120 * 900) {
            g->events = 0;
            game_step(g, DR_STEP, lane_bot(g));
            steps++;
            const Ball *b = &g->ball;
            if (!finite3(b)) { CHECK(0, "NaN/inf in the ball state (seed %u)", seed); break; }
            if (g->dead) { CHECK(0, "lane bot died (seed %u) at s=%.1f", seed, b->s); break; }
            const double ground = terrain_height(b->x, b->s) + DR_BALL_R;
            CHECK(b->y >= ground - 1e-6, "ball below the ground (seed %u, s=%.1f, by %.4f)", seed, b->s, ground - b->y);
            if (b->grounded) CHECK(fabs(b->y - ground) < 1e-6, "grounded ball not on the ground");
            if (b->vs < worst_speed_min) worst_speed_min = b->vs;
            if (b->vs > worst_speed_max) worst_speed_max = b->vs;
            if (b->s > 2000.0) { sum_far += b->vs; n_far++; }
            if (g->events & GE_LAUNCH) launches++;
            if (g->events & GE_LAND) lands++;
            if (!b->grounded) { was_air = true; last_air = b->air_time; }
            else if (was_air) { if (last_air > max_air) max_air = last_air; was_air = false; }
            if (b->air_time > 6.0 && !b->grounded) { CHECK(0, "airborne for over 6 s (seed %u)", seed); break; }
        }
    }
    printf("  speed range %.1f..%.1f m/s, mean beyond 2 km %.1f m/s; %d launches, %d hard landings, longest air %.2f s\n",
           worst_speed_min, worst_speed_max, n_far ? sum_far / n_far : 0.0, launches, lands, max_air);
    CHECK(worst_speed_min >= DR_MIN_SPEED - 1e-9, "speed fell below the minimum");
    CHECK(worst_speed_max <= DR_MAX_SPEED + 1e-9, "speed above the maximum");
    CHECK(n_far > 0 && sum_far / n_far > 26.0 && sum_far / n_far < 62.0, "mean speed out of the intended band");
    free(g);
}

// ---- 4b. the walls are not a safe place ---------------------------------------------------------------------------
// Regression: obstacles used to stop at |x| = 18.5 while the ball can reach |x| = 21, so holding the stick to one side and
// riding the wall survived forever. Now the whole reachable width has obstacles, so a bot that does exactly that must die.
static void test_wall_hugger(void)
{
    printf("wall hugger (the sides are not safe)\n");
    Game *g = (Game *)malloc(sizeof *g);
    int runs = 0, survivors = 0; double sum = 0.0, worst = 0.0;
    for (int side = -1; side <= 1; side += 2) {
        for (uint32_t seed = 1; seed <= 25; seed++) {
            game_reset(g, seed);
            long steps = 0;
            while (!g->dead && g->ball.s < 4000.0 && steps < 120L * 600) { game_step(g, DR_STEP, (double)side); steps++; }
            runs++;
            if (!g->dead) survivors++;
            sum += g->ball.s;
            if (g->ball.s > worst) worst = g->ball.s;
            CHECK(fabs(g->ball.x) > DR_X_LIMIT - 0.01, "the bot was meant to be against the wall (x=%.2f)", g->ball.x);
        }
    }
    printf("  %d runs, %d survived 4 km; mean death at %.0f m, furthest %.0f m\n", runs, survivors, sum / runs, worst);
    CHECK(survivors == 0, "%d of %d wall-hugging runs survived 4 km", survivors, runs);
    CHECK(sum / runs < 800.0, "wall hugging lasts too long on average (%.0f m)", sum / runs);
    free(g);
}

// ---- 5. a bot that deliberately takes ramps gets airtime and lands safely ----------------------------------------
// The bot lines up with each ramp's centre from 60 m out and holds that line until the end of the ramp's keep-clear
// corridor. Crossing the open slope on the way in is risky on purpose (a death there is counted, not failed, and the bot is
// revived); a death INSIDE the corridor is a failure: the run-up, the lip and the landing must always be safe.
static void test_ramps(void)
{
    printf("ramps (launch and land)\n");
    Game *g = (Game *)malloc(sizeof *g);
    int ramps_taken = 0, launched = 0, approach_deaths = 0; double best_air = 0.0, min_air = 1e9;
    for (uint32_t seed = 1; seed <= 8; seed++) {
        game_reset(g, seed);
        while (g->ball.s < 3000.0) {
            const Ball *b = &g->ball;
            const int cell = (int)floor(b->s / DR_RAMP_CELL);
            Ramp r = terrain_ramp(cell);
            int hold = 0;
            for (int k = cell; k <= cell + 1; k++) {
                Ramp q = terrain_ramp(k);
                if (q.exists && b->s > q.s0 - 60.0 && b->s < q.s0 + q.len + 75.0) { r = q; hold = 1; break; }
            }
            double steer = hold ? dr_clamp(0.35 * (r.x - b->x) - 0.09 * b->vx, -1.0, 1.0) : lane_bot(g);
            const bool in_corridor = hold && b->s >= r.s0 - 14.0 && fabs(b->x - r.x) < r.halfw + 4.5;

            g->events = 0;
            game_step(g, DR_STEP, steer);

            if (g->dead) {
                if (in_corridor || (hold && b->s > r.s0 - 14.0)) {
                    printf("  (seed %u died inside a ramp corridor at s=%.1f; ramp s0=%.1f len=%.1f x=%.1f; ball x=%.1f)\n",
                           seed, b->s, r.s0, r.len, r.x, b->x);
                    CHECK(0, "died inside a ramp keep-clear corridor (seed %u)", seed);
                    break;
                }
                approach_deaths++;                                 // crossed open slope to reach the ramp and hit something
                g->dead = false; g->ball.vs = 25.0; g->death_time = 0.0;
            }
            // track one ramp from its start to its end for airtime statistics
            static int tracked = -1000; static bool air_seen = false; static double air_here = 0.0;
            if (hold && r.s0 + r.len < b->s + 400.0) {
                const int id = (int)(r.s0 * 10.0);
                if (id != tracked) {
                    if (tracked != -1000 && air_seen) { launched++; if (air_here > best_air) best_air = air_here; if (air_here < min_air) min_air = air_here; }
                    tracked = id; ramps_taken++; air_seen = false; air_here = 0.0;
                }
                if (!b->grounded && b->s > r.s0 + r.len - 1.0) { air_seen = true; if (b->air_time > air_here) air_here = b->air_time; }
            }
        }
    }
    printf("  %d ramps lined up, %d launched the ball (%d approach deaths tolerated); airtime %.2f..%.2f s\n",
           ramps_taken, launched, approach_deaths, min_air > 1e8 ? 0.0 : min_air, best_air);
    CHECK(ramps_taken >= 12, "bot found too few ramps (%d)", ramps_taken);
    CHECK(launched >= ramps_taken * 5 / 10, "ramps rarely launch the ball (%d of %d)", launched, ramps_taken);
    CHECK(best_air > 0.35 && best_air < 4.5, "best airtime %.2f s is out of range", best_air);
    free(g);
}

// ---- 6. collisions, shields, jumping over --------------------------------------------------------------------------
static Obstacle *plant(Game *g, double x, double s, uint8_t type, float r, float h)
{
    WorldCell *c = world_cell(&g->world, (int)floor(s / DR_CELL_LEN));
    Obstacle *o = &c->obs[c->count++];
    o->x = x; o->s = s; o->r = r; o->h = h; o->type = type; o->destroyed = false;
    return o;
}

// An empty slope with the ball rolling at 30 m/s on a stretch that has no ramp anywhere near it (a ramp would launch the
// ball over the obstacle under test). Returns the ball's s.
static double fresh(Game *g)
{
    game_reset(g, 5);
    g->world.empty = true;
    double base = 200.0;
    for (int cell = 2; cell < 60; cell++) {
        if (!terrain_ramp(cell - 1).exists && !terrain_ramp(cell).exists && !terrain_ramp(cell + 1).exists) { base = cell * DR_RAMP_CELL + 10.0; break; }
    }
    g->ball.s = base; g->ball.x = 0.0; g->ball.vx = 0.0; g->ball.vs = 30.0;
    g->ball.y = terrain_height(0.0, base) + DR_BALL_R; g->ball.grounded = true; g->ball.vy = -9.0;
    g->dist = base;
    return base;
}

static void fly(Game *g, double above)                            // put the ball in the air `above` metres over the ground
{
    g->ball.grounded = false;
    g->ball.y = terrain_height(g->ball.x, g->ball.s) + above;
    g->ball.vy = 0.0;
}

static void test_collisions(void)
{
    printf("collisions\n");
    Game *g = (Game *)malloc(sizeof *g);
    double s;

    s = fresh(g); plant(g, 0.0, s + 30.0, OBS_TREE, 0.7f, 6.0f);
    for (int i = 0; i < 400 && !g->dead; i++) game_step(g, DR_STEP, 0.0);
    CHECK(g->dead, "driving straight into a tree did not end the run");
    CHECK((g->events & GE_DEATH) != 0, "no death event");
    const double s_dead = g->ball.s;
    for (int i = 0; i < 600; i++) game_step(g, DR_STEP, 1.0);
    CHECK(g->ball.vs < 0.5, "the crashed ball did not come to rest (vs=%.2f)", g->ball.vs);
    CHECK(g->ball.s < s_dead + 15.0, "the crashed ball rolled on too far (%.1f m)", g->ball.s - s_dead);

    s = fresh(g); g->shield = 2;
    Obstacle *o = plant(g, 0.0, s + 30.0, OBS_ROCK, 1.0f, 1.0f);
    for (int i = 0; i < 400 && g->shield == 2; i++) game_step(g, DR_STEP, 0.0);
    CHECK(!g->dead, "a shield did not save the run");
    CHECK(g->shield == 1, "shield count %d, wanted 1", g->shield);
    CHECK(o->destroyed, "the obstacle was not smashed");
    CHECK(g->invuln > 0.0, "no grace period after a shield hit");
    CHECK((g->events & GE_SHIELD_HIT) != 0, "no shield-hit event");
    CHECK(g->ball.vs < 30.0, "a shield hit did not slow the ball down (%.2f)", g->ball.vs);

    s = fresh(g); g->shield = 1;                                  // two obstacles in a row: the grace period covers the second
    plant(g, 0.0, s + 30.0, OBS_ROCK, 1.0f, 1.0f); plant(g, 0.2, s + 32.0, OBS_ROCK, 1.0f, 1.0f);
    for (int i = 0; i < 400; i++) game_step(g, DR_STEP, 0.0);
    CHECK(!g->dead && g->shield == 0, "grace period after a shield hit did not protect against the next obstacle");

    s = fresh(g); g->ball.s = s + 26.0; g->ball.y = terrain_height(0.0, g->ball.s) + 1.0;
    plant(g, 0.0, s + 30.0, OBS_ROCK, 1.0f, 1.0f);               // fly high over a low rock
    fly(g, 6.5);
    for (int i = 0; i < 40; i++) game_step(g, DR_STEP, 0.0);
    CHECK(!g->dead, "jumping high over a low rock still hit it");

    s = fresh(g); g->ball.s = s + 26.0; g->ball.y = terrain_height(0.0, g->ball.s) + 1.0;
    plant(g, 0.0, s + 30.0, OBS_TREE, 0.7f, 7.0f);               // the same height is not enough for a tall tree
    fly(g, 5.0);
    for (int i = 0; i < 40 && !g->dead; i++) game_step(g, DR_STEP, 0.0);
    CHECK(g->dead, "flew through a tall tree");

    s = fresh(g); plant(g, 8.0, s + 30.0, OBS_TREE, 0.7f, 6.0f); // well to the side
    for (int i = 0; i < 400; i++) game_step(g, DR_STEP, 0.0);
    CHECK(!g->dead, "hit something that was not in the way");
    free(g);
}

// ---- 7. the steering is responsive ---------------------------------------------------------------------------------
static void test_steering(void)
{
    printf("steering\n");
    Game *g = (Game *)malloc(sizeof *g);
    game_reset(g, 3);
    for (int i = 0; i < 120; i++) game_step(g, DR_STEP, 0.0);
    const double x0 = g->ball.x;
    for (int i = 0; i < 120; i++) game_step(g, DR_STEP, 1.0);
    const double right = g->ball.x - x0;
    CHECK(right > 4.0, "one second of full right steer moved only %.2f m", right);
    const double x1 = g->ball.x;
    for (int i = 0; i < 120; i++) game_step(g, DR_STEP, -1.0);
    CHECK(g->ball.x < x1 - 4.0, "steering back left is too weak");
    for (int i = 0; i < 120 * 6; i++) game_step(g, DR_STEP, 1.0);
    CHECK(g->ball.x <= DR_X_LIMIT + 1e-9, "the ball left the track");
    // nine seconds is well past the clear start, and the wall is no longer a safe place (see test_wall_hugger): only a death
    // before the clear start (nothing can touch the ball before about s = 68) would be a bug here
    CHECK(!g->dead || g->ball.s > 68.0, "died inside the clear start (s=%.1f)", g->ball.s);
    free(g);
}

// ---- 8. the looks of obstacles: valid, repeatable, spread as intended, and not part of the rules --------------------------
static void test_looks(void)
{
    printf("obstacle looks\n");
    static const int tree_mix[DR_TREE_VARIANTS] = { 30, 18, 16, 14, 12, 10 };
    static const int rock_mix[DR_ROCK_VARIANTS] = { 22, 14, 16, 10, 14, 8, 10, 6 };
    int tree_n[DR_TREE_VARIANTS] = { 0 }, rock_n[DR_ROCK_VARIANTS] = { 0 }, trees = 0, rocks = 0;
    World *a = (World *)malloc(sizeof *a), *b = (World *)malloc(sizeof *b);

    for (uint32_t seed = 1; seed <= 8; seed++) {
        terrain_set_seed(seed); world_reset(a);
        for (int k = 0; k < 600; k++) {
            const WorldCell *c = world_cell(a, k);
            for (int i = 0; i < c->count; i++) {
                const Obstacle *o = &c->obs[i];
                CHECK(o->yaw >= 0.0f && o->yaw < 2.0f * (float)DR_PI + 1e-4f, "yaw %f out of range", (double)o->yaw);
                if (o->type == OBS_TREE) {
                    CHECK(o->variant < DR_TREE_VARIANTS, "tree variant %d out of range", o->variant);
                    CHECK(o->stretch == 1.0f, "a tree was stretched (%f)", (double)o->stretch);
                    if (o->variant < DR_TREE_VARIANTS) tree_n[o->variant]++;
                    trees++;
                } else {
                    CHECK(o->variant < DR_ROCK_VARIANTS, "rock variant %d out of range", o->variant);
                    CHECK(o->stretch >= 0.879f && o->stretch <= 1.141f, "rock stretch %f out of 0.88..1.14", (double)o->stretch);
                    if (o->variant < DR_ROCK_VARIANTS) rock_n[o->variant]++;
                    rocks++;
                }
            }
        }
        // generated again from scratch (after the cache is dropped and refilled in a different order) it looks the same
        terrain_set_seed(seed); world_reset(b);
        for (int k = 599; k >= 0; k -= 3) {
            const WorldCell *cb = world_cell(b, k), *ca = world_cell(a, k);
            if (ca->index != k) continue;   // already evicted from the cache of `a`: nothing to compare
            CHECK(cb->count == ca->count, "cell %d count differs on regeneration", k);
            for (int i = 0; i < cb->count && i < ca->count; i++)
                CHECK(cb->obs[i].variant == ca->obs[i].variant && cb->obs[i].yaw == ca->obs[i].yaw && cb->obs[i].stretch == ca->obs[i].stretch &&
                      cb->obs[i].x == ca->obs[i].x && cb->obs[i].r == ca->obs[i].r, "cell %d obstacle %d changed on regeneration", k, i);
        }
    }
    printf("  %d trees, %d rocks\n", trees, rocks);
    for (int v = 0; v < DR_TREE_VARIANTS; v++) {
        const double pct = 100.0 * tree_n[v] / (trees ? trees : 1);
        CHECK(tree_n[v] > 0 && fabs(pct - tree_mix[v]) < 3.0, "tree variant %d is %.1f%% of trees, wanted about %d%%", v, pct, tree_mix[v]);
    }
    for (int v = 0; v < DR_ROCK_VARIANTS; v++) {
        const double pct = 100.0 * rock_n[v] / (rocks ? rocks : 1);
        CHECK(rock_n[v] > 0 && fabs(pct - rock_mix[v]) < 3.0, "rock variant %d is %.1f%% of rocks, wanted about %d%%", v, pct, rock_mix[v]);
    }
    free(a); free(b);
}

int main(void)
{
    test_determinism();
    test_terrain();
    test_fairness();
    test_lane_bot();
    test_wall_hugger();
    test_ramps();
    test_collisions();
    test_steering();
    test_looks();
    printf("\n%d checks, %d failed\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}

#else
typedef int dr_test_sim_unused;   // keeps the file a valid translation unit when it is not a test build
#endif
