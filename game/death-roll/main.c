// Death Roll: a big ball with the MidManStudio logo on an endless procedural slope. See README.md.
#include "raylib.h"
#include "src/platform/platform.h"
#include "src/sim/game.h"
#include "src/input/input.h"
#include "src/render/scene.h"
#include "src/ui/hud.h"
#include "src/storage/storage.h"

#include <stdio.h>
#include <time.h>

#if DR_PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

typedef struct {
    Game   game;
    Input  input;
    UiMode mode;
    double accum;        // unspent simulation time (the sim runs on a fixed step)
    double ui_time;
    int    best;
    bool   new_best;
    bool   debug;
    char   debug_text[320];
} App;

static App A;

static uint32_t fresh_seed(void)
{
    return (uint32_t)time(NULL) * 2654435761u ^ (uint32_t)(GetTime() * 1000000.0) ^ 0x9E3779B9u;
}

static void start_run(void)
{
    game_reset(&A.game, fresh_seed());
    scene_reset(&A.game);
    A.mode = UI_RUN;
    A.accum = 0.0;
    A.new_best = false;
}

// Follow the browser's viewport (a phone turning, the address bar sliding away, an iframe resized). raylib has its own resize
// handling; this only nudges it when the two disagree, which is cheap and idempotent. Off the web there is no browser.
static void sync_viewport(void)
{
#if DR_VIEWPORT_FOLLOWS_BROWSER
    const int w = platform_viewport_w(), h = platform_viewport_h();
    if (w >= 200 && h >= 200 && (w != GetScreenWidth() || h != GetScreenHeight())) SetWindowSize(w, h);
#endif
}

static void build_debug_text(int sw, int sh, bool portrait)
{
    const SceneStats *st = scene_stats();
    snprintf(A.debug_text, sizeof A.debug_text,
             "%d fps  %.1f ms\n%s  input:%s%s  %s %dx%d\nshaders world:%s hud:%s\nspeed %.1f  air %.2fs  s %.0f\nobstacles %d  chunks built %d\nseed %u",
             GetFPS(), GetFrameTime() * 1000.0f,
             DR_PLATFORM_NAME, input_device_name(&A.input), A.input.touch_capable ? "(touch device)" : "",
             portrait ? "portrait" : "landscape", sw, sh,
             st->world_shader_ok ? "ok" : "default", st->hud_shader_ok ? "ok" : "default",
             A.game.ball.vs, A.game.ball.air_time, A.game.ball.s,
             st->obstacles_drawn, st->chunks_rebuilt, A.game.seed);
}

static void frame(void)
{
    float dt = GetFrameTime();
    if (dt > 0.05f) dt = 0.05f;                              // a stalled tab must not become a huge step

    sync_viewport();
    const int sw = GetScreenWidth(), sh = GetScreenHeight();
    const bool portrait = sh > sw;
    A.ui_time += dt;

    input_update(&A.input, dt, sw, sh);
    if (IsKeyPressed(KEY_F3)) A.debug = !A.debug;

    if (A.mode == UI_MENU) {
        if (A.input.press) start_run();
    } else {
        A.accum += dt;
        while (A.accum >= DR_STEP) {
            game_step(&A.game, DR_STEP, A.mode == UI_RUN ? A.input.steer : 0.0);
            A.accum -= DR_STEP;
        }
        if (A.mode == UI_RUN && A.game.dead) {
            A.mode = UI_DEAD;
            const int dist = (int)A.game.dist;
            if (dist > A.best) { A.best = dist; A.new_best = true; storage_save_best(A.best); }
        } else if (A.mode == UI_DEAD && A.game.death_time > 0.6 && A.input.press) {
            start_run();
        }
    }

    const uint32_t events = A.game.events;
    A.game.events = 0;
    scene_update(&A.game, dt, events, portrait);

    HudInfo hud = {
        sw, sh, portrait, A.mode,
        (int)A.game.dist, A.best, A.new_best, A.game.shield,
        A.ui_time, A.game.death_time,
        input_steer_hint(&A.input), input_start_hint(&A.input),
        A.debug, A.debug_text
    };
    if (A.debug) build_debug_text(sw, sh, portrait);

    BeginDrawing();
    ClearBackground(scene_sky());
    scene_draw3d(&A.game, sw, sh);
    scene_begin_2d();
    hud_draw(&hud);
    scene_end_2d();
    EndDrawing();
}

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    int w = 960, h = 540;
    if (platform_viewport_w() >= 200 && platform_viewport_h() >= 200) { w = platform_viewport_w(); h = platform_viewport_h(); }
    InitWindow(w, h, "Death Roll");

    input_init(&A.input);
    scene_init();
    A.best = storage_load_best();
    A.debug = platform_debug_requested();

    game_reset(&A.game, fresh_seed());
    scene_reset(&A.game);
    A.mode = UI_MENU;

#if DR_PLATFORM_WEB
    emscripten_set_main_loop(frame, 0, 1);                   // the browser's frame callback; never returns
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) frame();
#endif
    CloseWindow();
    return 0;
}
