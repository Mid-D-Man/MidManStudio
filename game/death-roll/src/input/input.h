// ============================================================================
// Input, gated by device. One steering value in [-1, 1] comes out, from whichever of these the player is using:
//   touch    : a floating drag stick. Where the finger lands is the neutral point; dragging 15% of the short screen side
//              gives full steer; if the finger goes further the neutral point follows it, so coming back is quick.
//   mouse    : the same drag stick with the left button (so a desktop can play the phone way).
//   keyboard : A / D or the arrow keys, eased in and out.
// Gating rules (see README):
//   * touch is read only where the platform has a screen to touch (DR_INPUT_TOUCH);
//   * browsers also send emulated mouse events after a touch: the mouse is ignored for 0.8 s after any touch;
//   * `device` is whichever was used last, and only decides the hints and prompts shown;
//   * the drag range scales with the SHORT screen side, so it is the same distance under the thumb in portrait and landscape.
// ============================================================================
#ifndef DR_INPUT_H
#define DR_INPUT_H

#include <stdbool.h>

typedef enum { DEV_TOUCH = 0, DEV_KEYBOARD, DEV_MOUSE } InputDevice;

typedef struct {
    InputDevice device;
    bool  touch_capable;
    float steer;              // the result, [-1, 1]
    bool  press;              // a start / restart press happened this frame (a finger landing, a click, space, enter)
    bool  dragging;
    float anchor_x;
    float ptr_steer, kb_steer;
    bool  prev_ptr_down;
    double last_touch_time;
} Input;

void        input_init(Input *in);
void        input_update(Input *in, float dt, int screen_w, int screen_h);
const char *input_steer_hint(const Input *in);
const char *input_start_hint(const Input *in);
const char *input_device_name(const Input *in);

#endif
