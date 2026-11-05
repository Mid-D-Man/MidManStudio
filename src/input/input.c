#include "input.h"
#include "../platform/platform.h"
#include "raylib.h"
#include <math.h>
#include <string.h>

void input_init(Input *in)
{
    memset(in, 0, sizeof *in);
    in->last_touch_time = -100.0;
    in->touch_capable = DR_INPUT_TOUCH && platform_touch_capable();
    in->device = in->touch_capable ? DEV_TOUCH : DEV_KEYBOARD;
}

static float approach(float v, float target, float rate, float dt) { return v + (target - v) * (1.0f - expf(-rate * dt)); }

void input_update(Input *in, float dt, int sw, int sh)
{
    const double now = GetTime();
    in->press = false;

    // ---- pointer: a finger, or the mouse's left button ------------------------------------------------------------
    bool have_ptr = false;
    float px = 0.0f;
#if DR_INPUT_TOUCH
    if (GetTouchPointCount() > 0) {
        px = GetTouchPosition(0).x;
        have_ptr = true;
        in->device = DEV_TOUCH;
        in->last_touch_time = now;
    }
#endif
#if DR_INPUT_MOUSE
    if (!have_ptr && (now - in->last_touch_time) > 0.8 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        px = GetMousePosition().x;
        have_ptr = true;
        in->device = DEV_MOUSE;
    }
#endif
    if (have_ptr && !in->prev_ptr_down) in->press = true;
    in->prev_ptr_down = have_ptr;

    float target = 0.0f;
    if (have_ptr) {
        const float range = 0.15f * (float)(sw < sh ? sw : sh);
        if (!in->dragging) { in->dragging = true; in->anchor_x = px; }
        float dx = px - in->anchor_x;
        if (dx >  range) { in->anchor_x = px - range; dx =  range; }          // the neutral point follows the finger
        if (dx < -range) { in->anchor_x = px + range; dx = -range; }
        float t = dx / range;
        const float dead = 0.08f;
        if (fabsf(t) < dead) t = 0.0f; else t = copysignf((fabsf(t) - dead) / (1.0f - dead), t);
        target = t;
    } else {
        in->dragging = false;
    }
    in->ptr_steer = approach(in->ptr_steer, target, 30.0f, dt);

    // ---- keyboard --------------------------------------------------------------------------------------------------
    float k = 0.0f;
#if DR_INPUT_KEYBOARD
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  k -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) k += 1.0f;
    if (k != 0.0f) in->device = DEV_KEYBOARD;
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) { in->press = true; in->device = DEV_KEYBOARD; }
#endif
    in->kb_steer = approach(in->kb_steer, k, k != 0.0f ? 9.0f : 12.0f, dt);

    float s = in->ptr_steer + in->kb_steer;
    in->steer = s < -1.0f ? -1.0f : (s > 1.0f ? 1.0f : s);
}

const char *input_steer_hint(const Input *in)
{
    switch (in->device) {
        case DEV_TOUCH:    return "DRAG LEFT / RIGHT TO STEER";
        case DEV_MOUSE:    return "HOLD AND DRAG, OR USE A / D";
        default:           return "A / D OR ARROW KEYS TO STEER";
    }
}

const char *input_start_hint(const Input *in)
{
    return in->device == DEV_TOUCH ? "TAP TO START" : "PRESS SPACE OR CLICK TO START";
}

const char *input_device_name(const Input *in)
{
    switch (in->device) { case DEV_TOUCH: return "touch"; case DEV_MOUSE: return "mouse"; default: return "keyboard"; }
}
