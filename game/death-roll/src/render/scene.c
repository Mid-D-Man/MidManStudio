#include "scene.h"
#include "shaders.h"
#include "meshes.h"
#include "terrain_render.h"
#include "../sim/terrain.h"
#include "../sim/world.h"
#include "../platform/platform.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>

typedef struct {
    Shaders   sh;
    Material  mat_world, mat_ball, mat_ground;
    Mesh      ball, tree, rock;
    Texture2D ball_tex, grass_tex;
    TerrainRender terrain;
    Quaternion spin;
    float shake, cam_dy, look_dy, back, ahead, t;
    SceneStats stats;
} Scene;

static Scene S;

Color scene_sky(void) { return (Color){ 22, 34, 50, 255 }; }      // = the shader's fog colour (0.086, 0.133, 0.196)
const SceneStats *scene_stats(void) { return &S.stats; }

void scene_begin_2d(void) { if (S.sh.hud_ok) BeginShaderMode(S.sh.hud); }
void scene_end_2d(void)   { if (S.sh.hud_ok) EndShaderMode(); }

void scene_init(void)
{
    shaders_init(&S.sh);
    S.stats.world_shader_ok = S.sh.world_ok;
    S.stats.hud_shader_ok = S.sh.hud_ok;

    S.ball_tex = meshes_build_ball_texture();
    S.grass_tex = meshes_build_grass_texture();
    S.ball = GenMeshSphere((float)DR_BALL_R, 24, 32);
    S.tree = meshes_build_tree();
    S.rock = meshes_build_rock();

    S.mat_world = LoadMaterialDefault();
    S.mat_ball  = LoadMaterialDefault();
    S.mat_ground = LoadMaterialDefault();
    if (S.sh.world_ok) { S.mat_world.shader = S.sh.world; S.mat_ball.shader = S.sh.world; S.mat_ground.shader = S.sh.world; }
    SetMaterialTexture(&S.mat_ball, MATERIAL_MAP_DIFFUSE, S.ball_tex);
    SetMaterialTexture(&S.mat_ground, MATERIAL_MAP_DIFFUSE, S.grass_tex);

    terrain_render_init(&S.terrain);
    S.spin = QuaternionIdentity();
}

// ---- camera numbers depend on orientation: a portrait phone sees a narrow slice, so the camera sits further back/higher
static void view_params(bool portrait, float *back, float *up, float *ahead)
{
    *back  = portrait ? 10.5f : 8.5f;
    *up    = portrait ? 5.4f  : 4.0f;
    *ahead = portrait ? 18.0f : 13.0f;
}

void scene_reset(const Game *g)
{
    float up;
    view_params(false, &S.back, &up, &S.ahead);
    S.cam_dy = up + (float)DR_SLOPE * S.back;
    S.look_dy = 0.0f;
    S.shake = 0.0f;
    S.spin = QuaternionIdentity();
    terrain_render_invalidate(&S.terrain);
    terrain_render_update(&S.terrain, g->ball.s, 0);
    S.stats.chunks_rebuilt = S.terrain.rebuilt_total;
}

void scene_update(const Game *g, float dt, uint32_t events, bool portrait)
{
    const Ball *b = &g->ball;
    S.t += dt;

    if (events & GE_LAND)       S.shake = fmaxf(S.shake, fminf(0.6f, (float)b->last_impact * 0.03f));
    if (events & GE_SHIELD_HIT) S.shake = fmaxf(S.shake, 0.7f);
    if (events & GE_DEATH)      S.shake = 1.0f;
    S.shake *= expf(-5.0f * dt);

    float up;
    view_params(portrait, &S.back, &up, &S.ahead);
    const float want_cam  = up + (float)DR_SLOPE * S.back;                       // the camera sits as high above the slope as the ball
    const float want_look = (float)(terrain_height(b->x, b->s + S.ahead) - b->y) + 0.6f;   // and looks at the ground ahead
    const float k = 1.0f - expf(-7.0f * dt);
    S.cam_dy  += (want_cam - S.cam_dy) * k;
    S.look_dy += (want_look - S.look_dy) * k;

    // rolling: the axis is up x velocity (render space: forward is -z), the rate is speed / radius
    const float vx = (float)b->vx, vs = (float)b->vs;
    const float speed = sqrtf(vx * vx + vs * vs);
    if (speed > 0.05f) {
        const Vector3 axis = Vector3Normalize((Vector3){ -vs, 0.0f, -vx });
        const Quaternion q = QuaternionFromAxisAngle(axis, speed * dt / (float)DR_BALL_R);
        S.spin = QuaternionNormalize(QuaternionMultiply(q, S.spin));
    }

    terrain_render_update(&S.terrain, b->s, 2);                                 // at most two chunks per frame
    S.stats.chunks_rebuilt = S.terrain.rebuilt_total;
}

static void draw_obstacles(Game *g)
{
    const Ball *b = &g->ball;
    int drawn = 0;
    const int k0 = (int)floor((b->s - 20.0) / DR_CELL_LEN), k1 = (int)floor((b->s + 285.0) / DR_CELL_LEN);
    for (int k = k0; k <= k1; k++) {
        const WorldCell *c = world_cell(&g->world, k);
        for (int i = 0; i < c->count; i++) {
            const Obstacle *o = &c->obs[i];
            if (o->destroyed) continue;
            if (o->s < b->s - S.back + 1.0 || o->s > b->s + 285.0) continue;      // behind the camera / past the fog
            const float gy = (float)(terrain_height(o->x, o->s) - b->y);
            const float px = (float)o->x, pz = (float)(-(o->s - b->s));
            if (o->type == OBS_TREE) {
                const float sxz = (o->r * 1.25f) / 0.30f;
                DrawMesh(S.tree, S.mat_world, MatrixMultiply(MatrixScale(sxz, o->h, sxz), MatrixTranslate(px, gy, pz)));
            } else {
                DrawMesh(S.rock, S.mat_world, MatrixMultiply(MatrixScale(o->r, o->h * 0.6f, o->r), MatrixTranslate(px, gy + o->h * 0.4f, pz)));
            }
            drawn++;
        }
    }
    S.stats.obstacles_drawn = drawn;
}

// A soft dark disc on the ground under the ball, following the ground's shape: the only depth cue while airborne.
static void draw_shadow(const Ball *b)
{
    enum { N = 14 };
    const double ground = terrain_height(b->x, b->s);
    const float alt = (float)(b->y - DR_BALL_R - ground);
    float alpha = 0.58f - 0.035f * alt;
    if (alpha < 0.12f) alpha = 0.12f;
    float rad = (float)DR_BALL_R * (1.0f + 0.04f * alt);
    if (rad > 2.2f) rad = 2.2f;

    Vector3 ring[N + 1];
    for (int i = 0; i <= N; i++) {
        const float a = (float)i / N * 2.0f * (float)DR_PI;
        const double x = b->x + cosf(a) * rad, s = b->s + sinf(a) * rad;
        ring[i] = (Vector3){ (float)x, (float)(terrain_height(x, s) - b->y) + 0.07f, (float)(-(s - b->s)) };
    }
    const Vector3 centre = { (float)b->x, (float)(ground - b->y) + 0.07f, 0.0f };
    const unsigned char a8 = (unsigned char)(alpha * 255.0f);

    if (S.sh.hud_ok) BeginShaderMode(S.sh.hud);                 // highp: the default shader's mediump depth would flicker here
    rlBegin(RL_TRIANGLES);
    for (int i = 0; i < N; i++) {
        rlColor4ub(0, 0, 0, a8); rlVertex3f(centre.x, centre.y, centre.z);
        rlColor4ub(0, 0, 0, a8); rlVertex3f(ring[i + 1].x, ring[i + 1].y, ring[i + 1].z);
        rlColor4ub(0, 0, 0, a8); rlVertex3f(ring[i].x, ring[i].y, ring[i].z);
    }
    rlEnd();
    if (S.sh.hud_ok) EndShaderMode();
}

void scene_draw3d(Game *g, int sw, int sh)
{
    const Ball *b = &g->ball;
    const float aspect = (float)sw / (float)(sh > 0 ? sh : 1);

    // field of view: keep roughly 74 degrees across, within limits (a portrait phone cannot have that much), plus a kick with speed
    float fovy = 2.0f * atanf(tanf(37.0f * (float)DR_PI / 180.0f) / aspect) * 180.0f / (float)DR_PI;
    fovy = fovy < 46.0f ? 46.0f : (fovy > 80.0f ? 80.0f : fovy);
    float kick = ((float)b->vs - 20.0f) / 30.0f;
    kick = kick < 0.0f ? 0.0f : (kick > 1.0f ? 1.0f : kick);
    fovy += 7.0f * kick;

    const float shx = S.shake * 0.35f * sinf(S.t * 61.0f), shy = S.shake * 0.28f * sinf(S.t * 47.0f + 1.3f);
    const float bx = (float)b->x;
    Camera3D cam = { 0 };
    cam.position = (Vector3){ bx * 0.65f + shx, S.cam_dy + shy, S.back };
    cam.target   = (Vector3){ bx * 0.85f + shx * 0.5f, S.look_dy, -S.ahead };
    cam.up       = (Vector3){ 0.0f, 1.0f, 0.0f };
    cam.fovy     = fovy;
    cam.projection = CAMERA_PERSPECTIVE;

    rlSetClipPlanes(0.6, 700.0);
    BeginMode3D(cam);
    rlDisableBackfaceCulling();                                  // the ground and props are only ever seen from outside; this makes winding a non-issue

    terrain_render_draw(&S.terrain, S.mat_ground, b->y, b->s);
    draw_obstacles(g);
    draw_shadow(b);

    shaders_use_light(&S.sh, true);
    DrawMesh(S.ball, S.mat_ball, MatrixMultiply(QuaternionToMatrix(S.spin), MatrixTranslate(bx, 0.0f, 0.0f)));
    shaders_use_light(&S.sh, false);

    rlEnableBackfaceCulling();
    EndMode3D();
}
