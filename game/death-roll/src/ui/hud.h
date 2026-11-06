// ============================================================================
// Menus and HUD, laid out from the screen's own size and orientation every frame (nothing is hard-coded to 16:9):
//   portrait  - the score is centred at the top, best below it, stacked
//   landscape - the score top-left, best top-right
// Text sizes come from the SHORT side of the screen, so they read the same on a phone held either way.
// ============================================================================
#ifndef DR_HUD_H
#define DR_HUD_H

#include <stdbool.h>

typedef enum { UI_MENU = 0, UI_RUN, UI_DEAD } UiMode;

typedef struct {
    int    screen_w, screen_h;
    bool   portrait;
    UiMode mode;
    int    dist, best;
    bool   new_best;
    int    shield;
    double time;               // seconds, for blinking prompts
    double dead_time;          // seconds since the crash
    const char *steer_hint;
    const char *start_hint;
    bool   debug;
    const char *debug_text;    // several lines, drawn small in a corner when `debug` is on
} HudInfo;

void hud_draw(const HudInfo *h);

#endif
