// Headless test of the render-side GEOMETRY code. raylib's GPU calls are replaced by stubs that record what they were given, so
// the real terrain_render.c and meshes.c run and their output (vertices, colours, transforms, chunk streaming) can be checked
// without a GL context. Built by tests/run_tests.sh with -DDR_RENDER_TESTS; an empty translation unit otherwise.
#ifdef DR_RENDER_TESTS

#include "raylib.h"
#include "raymath.h"
#include "../src/render/terrain_render.h"
#include "../src/render/meshes.h"
#include "../src/sim/terrain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// ---- stubs for the raylib calls those two files make ----------------------------------------------------------------
static int g_uploads, g_updates, g_draws;
static Matrix g_last_draw;
static int g_bad_update;

void *MemAlloc(unsigned int size) { return calloc(1, size); }
void UploadMesh(Mesh *mesh, bool dynamic) { (void)dynamic; g_uploads++; mesh->vaoId = 1; mesh->vboId = (unsigned int *)calloc(7, sizeof(unsigned int)); }
void UpdateMeshBuffer(Mesh mesh, int index, const void *data, int dataSize, int offset)
{
    g_updates++;
    if (!data || offset != 0) g_bad_update++;
    if (index == 0 && dataSize != mesh.vertexCount * 3 * (int)sizeof(float)) g_bad_update++;
    if (index == 3 && dataSize != mesh.vertexCount * 4) g_bad_update++;
    if (index != 0 && index != 3) g_bad_update++;
}
void DrawMesh(Mesh mesh, Material material, Matrix transform) { (void)mesh; (void)material; g_draws++; g_last_draw = transform; }
Image GenImageColor(int w, int h, Color c) { (void)c; Image i = { 0 }; i.width = w; i.height = h; return i; }
void ImageDrawRectangle(Image *dst, int x, int y, int w, int h, Color c) { (void)dst; (void)x; (void)y; (void)w; (void)h; (void)c; }
Image LoadImage(const char *f) { (void)f; Image i = { 0 }; return i; }                       // "logo missing": the fallback path
void ImageFormat(Image *i, int f) { (void)i; (void)f; }
Image ImageCopy(Image i) { return i; }
void ImageResize(Image *i, int w, int h) { (void)i; (void)w; (void)h; }
void ImageDrawImage(Image *dst, Image src, int x, int y, Color t) { (void)dst; (void)src; (void)x; (void)y; (void)t; }
void UnloadImage(Image i) { (void)i; }
Texture2D LoadTextureFromImage(Image i) { Texture2D t = { 0 }; t.width = i.width; t.height = i.height; return t; }
void SetTextureFilter(Texture2D t, int f) { (void)t; (void)f; }

static int g_fail = 0, g_checks = 0;
#define CHECK(cond, ...) do { g_checks++; if (!(cond)) { g_fail++; printf("  FAIL %s:%d  ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static void check_chunk_mesh(const Mesh *m, int chunk, int *tris_total, int *tris_up)
{
    CHECK(m->vertexCount > 0 && m->vertexCount % 3 == 0, "bad vertex count %d", m->vertexCount);
    CHECK(m->texcoords != NULL && m->normals != NULL && m->colors != NULL, "a vertex attribute array is missing");
    for (int v = 0; v < m->vertexCount; v++) {
        const float x = m->vertices[v * 3], y = m->vertices[v * 3 + 1], z = m->vertices[v * 3 + 2];
        if (!(isfinite(x) && isfinite(y) && isfinite(z))) { CHECK(0, "non-finite vertex in chunk %d", chunk); return; }
        CHECK(fabsf(x) <= 27.0f + 1e-3f, "x out of range: %f", x);
        CHECK(z <= 1e-3f && z >= -32.0f - 1e-3f, "chunk-local z out of range: %f", z);
        CHECK(fabsf(y) < 60.0f, "chunk-local y out of range: %f", y);      // heights are relative to the chunk's own base
        CHECK(m->colors[v * 4 + 3] == 255, "colour alpha not opaque");
    }
    for (int t = 0; t < m->vertexCount / 3; t++) {
        const float *a = &m->vertices[(t * 3) * 3], *b = &m->vertices[(t * 3 + 1) * 3], *c = &m->vertices[(t * 3 + 2) * 3];
        const float ux = b[0] - a[0], uz = b[2] - a[2], wx = c[0] - a[0], wz = c[2] - a[2];
        const float ny = uz * wx - ux * wz;                                 // y of (b-a) x (c-a): > 0 is counter-clockwise seen from above
        (*tris_total)++;
        if (ny > 0.0f) (*tris_up)++;
    }
}

static void test_terrain_render(void)
{
    printf("terrain chunks\n");
    terrain_set_seed(31337);
    TerrainRender t;
    terrain_render_init(&t);
    CHECK(g_uploads == TR_SLOTS, "uploads %d, wanted %d", g_uploads, TR_SLOTS);

    terrain_render_update(&t, 0.0, 0);                                      // everything at once (a new run)
    int have[TR_SLOTS], n = 0;
    for (int i = 0; i < TR_SLOTS; i++) if (t.slots[i].index != INT_MIN) have[n++] = t.slots[i].index;
    CHECK(n == TR_SLOTS, "only %d of %d chunks built at reset", n, TR_SLOTS);
    for (int c = -1; c <= 9; c++) { int f = 0; for (int i = 0; i < n; i++) if (have[i] == c) f = 1; CHECK(f, "chunk %d missing at reset", c); }

    int tris = 0, up = 0;
    for (int i = 0; i < TR_SLOTS; i++) check_chunk_mesh(&t.slots[i].mesh, t.slots[i].index, &tris, &up);
    CHECK(up * 100 >= tris * 99, "only %d of %d triangles wind counter-clockwise from above", up, tris);
    printf("  %d triangles, %.2f%% face up\n", tris, tris ? 100.0 * up / tris : 0.0);

    // a long run: the ball moves on at 45 m/s, 60 fps, at most 2 chunks rebuilt per frame; the needed chunks must always exist
    int worst_missing = 0;
    for (double s = 0.0; s < 20000.0; s += 45.0 / 60.0) {
        terrain_render_update(&t, s, 2);
        const int c0 = (int)floor(s / 32.0);
        int missing = 0;
        for (int c = c0; c <= c0 + 9; c++) { int f = 0; for (int i = 0; i < TR_SLOTS; i++) if (t.slots[i].index == c) f = 1; if (!f) missing++; }
        if (missing > worst_missing) worst_missing = missing;
        for (int i = 0; i < TR_SLOTS; i++) for (int j = i + 1; j < TR_SLOTS; j++)
            if (t.slots[i].index != INT_MIN && t.slots[i].index == t.slots[j].index) { CHECK(0, "chunk %d held twice", t.slots[i].index); i = j = TR_SLOTS; }
    }
    CHECK(worst_missing == 0, "%d needed chunks were missing at some frame", worst_missing);
    CHECK(g_bad_update == 0, "%d malformed UpdateMeshBuffer calls", g_bad_update);
    printf("  streamed 20 km: %d chunk rebuilds, %d buffer updates\n", t.rebuilt_total, g_updates);

    // seed change: invalidate then rebuild must produce a different ground
    float before = t.slots[0].mesh.vertices[1];
    terrain_set_seed(777); terrain_render_invalidate(&t); terrain_render_update(&t, 20000.0, 0);
    int rebuilt_ok = 1; for (int i = 0; i < TR_SLOTS; i++) if (t.slots[i].index == INT_MIN) rebuilt_ok = 0;
    CHECK(rebuilt_ok, "invalidate + update did not rebuild every chunk");
    (void)before;

    // drawing: every chunk's transform is small (nothing the GPU sees is large) even 20 km down the slope
    g_draws = 0;
    Material m = { 0 };
    const double ball_s = 20000.0, ball_y = terrain_height(0.0, ball_s) + 1.0;
    terrain_render_draw(&t, m, ball_y, ball_s);
    CHECK(g_draws >= 8 && g_draws <= TR_SLOTS, "drew %d chunks", g_draws);
    CHECK(fabsf(g_last_draw.m12) < 1e-6f, "chunk translated sideways");
    CHECK(fabsf(g_last_draw.m13) < 500.0f && fabsf(g_last_draw.m14) < 500.0f, "chunk offset is large (%f, %f): precision would suffer", g_last_draw.m13, g_last_draw.m14);
}

static void check_soup(const char *name, Mesh m, float max_extent, float min_height, float max_height)
{
    CHECK(m.vertexCount > 0 && m.vertexCount % 3 == 0, "%s: bad vertex count", name);
    CHECK(m.texcoords != NULL && m.normals != NULL && m.colors != NULL, "%s: a vertex attribute array is missing", name);
    float top = -1e9f, bot = 1e9f;
    for (int v = 0; v < m.vertexCount; v++) {
        const float x = m.vertices[v * 3], y = m.vertices[v * 3 + 1], z = m.vertices[v * 3 + 2];
        CHECK(isfinite(x) && isfinite(y) && isfinite(z), "%s: non-finite vertex", name);
        CHECK(sqrtf(x * x + z * z) <= max_extent + 1e-3f, "%s: too wide (%f)", name, sqrtf(x * x + z * z));
        if (y > top) top = y;
        if (y < bot) bot = y;
        CHECK(m.colors[v * 4 + 3] == 255, "%s: alpha", name);
    }
    CHECK(top >= min_height && top <= max_height, "%s: top at %f, wanted %f..%f", name, top, min_height, max_height);
    printf("  %s: %d vertices, y %.2f..%.2f\n", name, m.vertexCount, bot, top);
}

static void test_meshes(void)
{
    printf("generated meshes\n");
    terrain_set_seed(1);
    check_soup("tree", meshes_build_tree(), 0.31f, 0.999f, 1.001f);
    check_soup("rock", meshes_build_rock(), 1.2f, 0.7f, 1.3f);
    Texture2D t = meshes_build_ball_texture();
    CHECK(t.width == 512 && t.height == 256, "ball texture is %dx%d, wanted 512x256 (2:1 keeps the logo square on the sphere)", t.width, t.height);
}

int main(void)
{
    test_terrain_render();
    test_meshes();
    printf("\n%d checks, %d failed\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}

#else
typedef int dr_test_render_unused;
#endif
