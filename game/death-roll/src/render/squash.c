#include "squash.h"
#include <math.h>

// ---- tuning -----------------------------------------------------------------------------------------------------------
#define SQ_OMEGA        18.0f     // spring frequency (rad/s): one wobble takes about a third of a second
#define SQ_ZETA          0.30f    // damping ratio: below 1 so it overshoots and jiggles before settling
#define SQ_E_MIN        (-0.42f)  // the flattest the ball ever gets (58 % of its height)
#define SQ_E_MAX         0.30f
#define SQ_EV_MAX       12.0f
#define SQ_M_MAX         0.32f
#define SQ_MAX_DT        0.05f    // a long frame (tab switch) must not blow the spring up
#define SQ_SUBSTEP       (1.0f / 240.0f)

static float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

void squash_reset(Squash *q)
{
    q->e = q->ev = q->m = 0.0f;
    q->ground = 1.0f;
    q->dir[0] = 0.0f; q->dir[1] = 0.0f; q->dir[2] = -1.0f;
    q->primed = false;
}

void squash_update(Squash *q, float dt, const SquashInput *in)
{
    if (!(dt > 0.0f)) return;
    if (dt > SQ_MAX_DT) dt = SQ_MAX_DT;
    const float speed = (in->speed > 0.0f && isfinite(in->speed)) ? in->speed : 0.0f;

    // ---- events: these set the spring off ----
    if (in->launched) {
        q->ev += clampf(2.4f + 0.05f * speed, 2.8f, 5.0f);             // a stretch pop as it leaves the ground
    }
    if (in->landed) {
        const float impact = (in->impact > 0.0f && isfinite(in->impact)) ? in->impact : 0.0f;
        const float depth = clampf(0.02f + 0.0125f * impact, 0.0f, 0.40f);
        if (depth > 0.03f) { q->e = -depth; q->ev = 0.0f; }             // snap flat, then let the spring bounce it back
        q->ground = 1.0f;
    }
    if (in->hit)  { q->e = -0.34f; q->ev = 0.0f; }
    if (in->died) { q->e = SQ_E_MIN; q->ev = 0.0f; }

    // ---- the impact spring (semi-implicit Euler in small steps, so it is stable and frame-rate independent) ----
    const float k = SQ_OMEGA * SQ_OMEGA, c = 2.0f * SQ_ZETA * SQ_OMEGA;
    int n = (int)ceilf(dt / SQ_SUBSTEP);
    if (n < 1) n = 1;
    const float h = dt / (float)n;
    for (int i = 0; i < n; i++) {
        q->ev += (-k * q->e - c * q->ev) * h;
        q->ev = clampf(q->ev, -SQ_EV_MAX, SQ_EV_MAX);
        q->e += q->ev * h;
        q->e = clampf(q->e, SQ_E_MIN, SQ_E_MAX);
    }

    // ---- the motion stretch: speed-based, strongest in the air ----
    const float target = in->grounded ? clampf(0.0012f * speed, 0.0f, 0.05f) : clampf(0.0058f * speed, 0.0f, 0.28f);
    q->m += (target - q->m) * (1.0f - expf(-12.0f * dt));
    q->m = clampf(q->m, 0.0f, SQ_M_MAX);

    // ---- the travel direction, smoothed so the stretch axis does not snap when the velocity does (at a landing) ----
    {
        const float *v = in->vel;
        const float len = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        if (len > 0.5f && isfinite(len)) {
            const float t[3] = { v[0] / len, v[1] / len, v[2] / len };
            if (!q->primed) { q->dir[0] = t[0]; q->dir[1] = t[1]; q->dir[2] = t[2]; q->primed = true; }
            else {
                const float a = 1.0f - expf(-16.0f * dt);
                for (int i = 0; i < 3; i++) q->dir[i] += (t[i] - q->dir[i]) * a;
                const float dl = sqrtf(q->dir[0] * q->dir[0] + q->dir[1] * q->dir[1] + q->dir[2] * q->dir[2]);
                if (dl > 1e-4f) { q->dir[0] /= dl; q->dir[1] /= dl; q->dir[2] /= dl; }
                else { q->dir[0] = t[0]; q->dir[1] = t[1]; q->dir[2] = t[2]; }
            }
        }
    }

    // ---- the ground anchor: full while standing on it, fading out over about an eighth of a second after take-off ----
    if (in->grounded) q->ground = 1.0f;
    else { q->ground -= 8.0f * dt; if (q->ground < 0.0f) q->ground = 0.0f; }
}

float squash_along_normal(const Squash *q) { return 1.0f + q->e; }
float squash_along_motion(const Squash *q) { return 1.0f + q->m; }
float squash_across(float along) { return 1.0f / sqrtf(along > 0.05f ? along : 0.05f); }
float squash_anchor(const Squash *q) { return q->e * q->ground; }
