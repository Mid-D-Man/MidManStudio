#include "ball.h"
#include "terrain.h"

void ball_init(Ball *b)
{
    b->x = 0.0; b->s = 0.0;
    b->y = terrain_height(0.0, 0.0) + DR_BALL_R;
    b->vx = 0.0; b->vs = DR_START_SPEED; b->vy = 0.0;
    b->grounded = true; b->air_time = 0.0; b->last_impact = 0.0;
    double gx, gs;
    terrain_gradient(0.0, 0.0, &gx, &gs);
    b->vy = b->vs * gs;
}

void ball_step(Ball *b, double dt, double steer, double difficulty, double brake, uint32_t *events)
{
    double gx, gs;
    terrain_gradient(b->x, b->s, &gx, &gs);

    const double dragq = DR_DRAG_Q * (1.0 - 0.5 * difficulty);
    double as, ax;
    if (b->grounded) {
        as = brake > 0.0 ? -brake * b->vs                    // crashed: only braking, so it comes to rest even downhill
                         : -DR_SLOPE_ACCEL * gs - dragq * b->vs * b->vs;
        ax = steer * DR_STEER_ACCEL - DR_LAT_SLOPE * DR_SLOPE_ACCEL * gx - DR_LAT_DRAG * b->vx - brake * b->vx;
    } else {
        as = -0.03 * b->vs - brake * b->vs;
        ax = steer * DR_STEER_ACCEL * DR_AIR_CONTROL - 1.2 * b->vx - brake * b->vx;
    }
    b->vs += as * dt;
    b->vx += ax * dt;
    if (brake <= 0.0 && b->vs < DR_MIN_SPEED) b->vs = DR_MIN_SPEED;
    if (b->vs > DR_MAX_SPEED) b->vs = DR_MAX_SPEED;
    if (b->vs < 0.0) b->vs = 0.0;

    double nx = b->x + b->vx * dt;
    const double ns = b->s + b->vs * dt;
    if (nx > DR_X_LIMIT)  { nx =  DR_X_LIMIT; if (b->vx > 0.0) b->vx = 0.0; }
    if (nx < -DR_X_LIMIT) { nx = -DR_X_LIMIT; if (b->vx < 0.0) b->vx = 0.0; }

    const double g_new = terrain_height(nx, ns) + DR_BALL_R;   // where the ball would sit on the ground at the new spot

    if (b->grounded) {
        const double y_free = b->y + b->vy * dt - 0.5 * DR_GRAVITY * dt * dt;     // where free fall would take it
        if (y_free > g_new + DR_LIFT_EPS) {
            b->grounded = false;                                // the ground fell away faster than gravity: airborne
            b->air_time = 0.0;
            b->y = y_free;
            b->vy -= DR_GRAVITY * dt;
            if (events) *events |= GE_LAUNCH;
        } else {
            const double follow = (g_new - b->y) / dt;
            b->vy = dr_clamp(follow, -60.0, 60.0);
            b->y = g_new;
        }
    } else {
        b->vy -= DR_GRAVITY * dt;
        b->y += b->vy * dt;
        b->air_time += dt;
        if (b->y <= g_new) {                                    // landed
            double lgx, lgs;
            terrain_gradient(nx, ns, &lgx, &lgs);
            const double vy_ground = b->vs * lgs + b->vx * lgx;
            const double impact = vy_ground - b->vy;            // how fast it was going into the ground
            b->y = g_new;
            b->vy = dr_clamp(vy_ground, -60.0, 60.0);
            b->grounded = true;
            b->last_impact = impact > 0.0 ? impact : 0.0;
            if (b->last_impact > 6.0) {
                b->vs *= 1.0 - dr_clamp(b->last_impact * 0.004, 0.0, 0.10);
                if (events) *events |= GE_LAND;
            }
        }
    }
    b->x = nx;
    b->s = ns;
}
