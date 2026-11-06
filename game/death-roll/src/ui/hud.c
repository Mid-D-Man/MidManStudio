#include "hud.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define NEON   (Color){ 96, 255, 128, 255 }
#define INK    (Color){ 236, 242, 246, 255 }
#define DIM    (Color){ 150, 168, 180, 255 }
#define SHADOW (Color){ 0, 0, 0, 170 }

static void text_at(const char *t, float x, float y, float size, Color c)
{
    const float sp = size * 0.1f, off = size * 0.07f;
    DrawTextEx(GetFontDefault(), t, (Vector2){ x + off, y + off }, size, sp, SHADOW);
    DrawTextEx(GetFontDefault(), t, (Vector2){ x, y }, size, sp, c);
}

static void text_c(const char *t, float cx, float y, float size, Color c)
{
    const Vector2 m = MeasureTextEx(GetFontDefault(), t, size, size * 0.1f);
    text_at(t, cx - m.x * 0.5f, y, size, c);
}

static void text_r(const char *t, float rx, float y, float size, Color c)
{
    const Vector2 m = MeasureTextEx(GetFontDefault(), t, size, size * 0.1f);
    text_at(t, rx - m.x, y, size, c);
}

static void shield_pips(const HudInfo *h, float cx, float y, float u)
{
    if (h->shield <= 0) return;
    const float r = u * 0.022f, gap = u * 0.065f;
    const float x0 = cx - (h->shield - 1) * gap * 0.5f;
    for (int i = 0; i < h->shield; i++) {
        DrawCircle((int)(x0 + i * gap), (int)y, r + 2.0f, SHADOW);
        DrawCircle((int)(x0 + i * gap), (int)y, r, (Color){ 90, 220, 255, 255 });
    }
}

static void draw_debug(const HudInfo *h, float u)
{
    if (!h->debug || !h->debug_text) return;
    const float size = u * 0.032f < 11.0f ? 11.0f : u * 0.032f;
    DrawTextEx(GetFontDefault(), h->debug_text, (Vector2){ u * 0.02f + 1.0f, h->screen_h - u * 0.02f - size * 7.0f + 1.0f }, size, size * 0.1f, SHADOW);
    DrawTextEx(GetFontDefault(), h->debug_text, (Vector2){ u * 0.02f, h->screen_h - u * 0.02f - size * 7.0f }, size, size * 0.1f, (Color){ 255, 214, 90, 255 });
}

void hud_draw(const HudInfo *h)
{
    const float sw = (float)h->screen_w, sh = (float)h->screen_h;
    const float u = sw < sh ? sw : sh;            // the short side drives every size
    const float pad = u * 0.045f;
    char buf[64];
    const bool blink = fmod(h->time, 1.1) < 0.7;

    if (h->mode == UI_RUN) {
        snprintf(buf, sizeof buf, "%d m", h->dist);
        if (h->portrait) {
            text_c(buf, sw * 0.5f, pad, u * 0.11f, INK);
            snprintf(buf, sizeof buf, "BEST %d", h->best);
            text_c(buf, sw * 0.5f, pad + u * 0.135f, u * 0.045f, DIM);
            shield_pips(h, sw * 0.5f, pad + u * 0.21f, u);
        } else {
            text_at(buf, pad, pad, u * 0.10f, INK);
            snprintf(buf, sizeof buf, "BEST %d", h->best);
            text_r(buf, sw - pad, pad, u * 0.05f, DIM);
            shield_pips(h, pad + u * 0.05f, pad + u * 0.145f, u);
        }
    } else if (h->mode == UI_MENU) {
        DrawRectangle(0, 0, h->screen_w, h->screen_h, (Color){ 6, 10, 16, 120 });
        const float ty = h->portrait ? sh * 0.20f : sh * 0.14f;
        text_c("DEATH ROLL", sw * 0.5f, ty, u * (h->portrait ? 0.16f : 0.15f), NEON);
        text_c("MIDMANSTUDIO", sw * 0.5f, ty + u * (h->portrait ? 0.19f : 0.18f), u * 0.045f, DIM);
        if (h->best > 0) {
            snprintf(buf, sizeof buf, "BEST  %d m", h->best);
            text_c(buf, sw * 0.5f, sh * (h->portrait ? 0.50f : 0.56f), u * 0.06f, INK);
        }
        text_c(h->steer_hint, sw * 0.5f, sh * (h->portrait ? 0.66f : 0.68f), u * 0.04f, INK);
        text_c("ONE HIT AND YOU ARE DONE", sw * 0.5f, sh * (h->portrait ? 0.71f : 0.75f), u * 0.032f, DIM);
        if (blink) text_c(h->start_hint, sw * 0.5f, sh * (h->portrait ? 0.82f : 0.86f), u * 0.055f, NEON);
    } else {
        DrawRectangle(0, 0, h->screen_w, h->screen_h, (Color){ 0, 0, 0, 120 });
        text_c("CRASHED", sw * 0.5f, sh * (h->portrait ? 0.22f : 0.16f), u * 0.15f, (Color){ 255, 90, 90, 255 });
        snprintf(buf, sizeof buf, "%d m", h->dist);
        text_c(buf, sw * 0.5f, sh * (h->portrait ? 0.38f : 0.38f), u * 0.12f, INK);
        if (h->new_best) text_c("NEW BEST!", sw * 0.5f, sh * (h->portrait ? 0.52f : 0.56f), u * 0.07f, NEON);
        else { snprintf(buf, sizeof buf, "BEST  %d m", h->best); text_c(buf, sw * 0.5f, sh * (h->portrait ? 0.52f : 0.56f), u * 0.055f, DIM); }
        if (h->dead_time > 0.6 && blink) text_c(h->start_hint, sw * 0.5f, sh * (h->portrait ? 0.78f : 0.82f), u * 0.05f, NEON);
    }
    draw_debug(h, u);
}
