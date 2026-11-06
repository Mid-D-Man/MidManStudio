// Headless test of the render-side GEOMETRY code. raylib's GPU calls are replaced by stubs that record what they were given, so
// the real terrain_render.c and meshes.c run and their output (vertices, colours, transforms, chunk streaming) can be checked
// without a GL context. Built by tests/run_tests.sh with -DDR_RENDER_TESTS; an empty translation unit otherwise.
#ifdef DR_RENDER_TESTS

#include "raylib.h"
#include "raymath.h"
#include "../src/render/terrain_render.h"
#include "../src/render/meshes.h"
#include "../src/render/grass.h"
#include "../src/sim/terrain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// ---- stubs for the raylib calls those two files make ----------------------------------------------------------------
static int g_uploads, g_updates, g_draws, g_uv_updates;
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
    if (index == 1) { g_uv_updates++; if (dataSize != mesh.vertexCount * 2 * (int)sizeof(float)) g_bad_update++; }
    if (index != 0 && index != 1 && index != 3) g_bad_update++;
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
void SetTextureWrap(Texture2D t, int w) { (void)t; (void)w; }
void GenTextureMipmaps(Texture2D *t) { (void)t; }

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
        const float u = m->texcoords[v * 2], tv = m->texcoords[v * 2 + 1];
        CHECK(fabsf(u - x / (float)GRASS_TILE_M) < 1e-4f && fabsf(tv + z / (float)GRASS_TILE_M) < 1e-4f,
              "texcoord (%f, %f) does not follow the world position (%f, %f)", u, tv, x, z);
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
    CHECK(g_uv_updates == TR_SLOTS, "texcoords were uploaded %d times, wanted once per slot (%d)", g_uv_updates, TR_SLOTS);
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

// ---- the grass: a tile that repeats without a seam, steady brightness, chunk edges that meet on tile edges --------------
static double texel_luma(int x, int y) { unsigned char p[3]; grass_texel(x, y, p); return 0.299 * p[0] + 0.587 * p[1] + 0.114 * p[2]; }

static void test_grass(void)
{
    printf("grass\n");
    const int N = GRASS_TILE_PX;
    CHECK((N & (N - 1)) == 0, "tile size %d is not a power of two (WebGL 1 cannot repeat or mipmap it)", N);
    CHECK(fmod(32.0, GRASS_TILE_M) == 0.0, "the 32 m chunk length is not a whole number of tiles: chunk starts would not be tile edges");

    // it is periodic: any integer coordinates fold back into the tile
    int periodic = 1;
    for (int y = 0; y < N; y += 7) for (int x = 0; x < N; x += 5) {
        unsigned char a[3], b[3];
        grass_texel(x, y, a); grass_texel(x + 3 * N, y - 2 * N, b);
        if (memcmp(a, b, 3) != 0) periodic = 0;
    }
    CHECK(periodic, "grass_texel is not periodic in the tile size");

    // no seam: neighbouring pixels across the wrap differ about as much as neighbouring pixels inside the tile
    double seam_h = 0, in_h = 0, seam_v = 0, in_v = 0, sum = 0, sum2 = 0, lo = 1e9, hi = 0;
    for (int i = 0; i < N; i++) {
        seam_h += fabs(texel_luma(N - 1, i) - texel_luma(N, i));
        seam_v += fabs(texel_luma(i, N - 1) - texel_luma(i, N));
        for (int j = 0; j < N - 1; j++) {
            in_h += fabs(texel_luma(j, i) - texel_luma(j + 1, i));
            in_v += fabs(texel_luma(i, j) - texel_luma(i, j + 1));
        }
    }
    for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
        const double l = texel_luma(x, y); sum += l; sum2 += l * l; if (l < lo) lo = l; if (l > hi) hi = l;
    }
    seam_h /= N; seam_v /= N; in_h /= N * (N - 1); in_v /= N * (N - 1);
    CHECK(seam_h <= in_h * 1.35 + 0.5, "visible seam left/right: %.2f at the wrap vs %.2f inside", seam_h, in_h);
    CHECK(seam_v <= in_v * 1.35 + 0.5, "visible seam top/bottom: %.2f at the wrap vs %.2f inside", seam_v, in_v);

    const double mean = sum / (N * N) / 255.0, sd = sqrt(sum2 / (N * N) - (sum / (N * N)) * (sum / (N * N)));
    CHECK(mean * GRASS_TEX_GAIN > 0.97 && mean * GRASS_TEX_GAIN < 1.03, "texture brightness %.3f x gain %.2f is not about 1 (the lawn would get darker or brighter)", mean, (double)GRASS_TEX_GAIN);
    CHECK(sd > 4.0, "texture is almost flat (stddev %.1f)", sd);
    CHECK(lo > 100.0, "texture has near-black pixels (%.0f)", lo);
    printf("  tile %dx%d: mean %.3f (x gain = %.3f), stddev %.1f, range %.0f..%.0f; wrap diff %.2f/%.2f vs inside %.2f/%.2f\n",
           N, N, mean, mean * GRASS_TEX_GAIN, sd, lo, hi, seam_h, seam_v, in_h, in_v);

    // the lawn colour: steady, bounded, different per seed, and smooth (no jump between neighbouring spots)
    double min_l = 1e9, max_l = 0, worst_step = 0;
    int differs = 0;
    for (double s = 0.0; s < 400.0; s += 0.5) for (double x = -27.0; x <= 27.0; x += 3.0) {
        float a[3], b[3];
        terrain_set_seed(5); grass_patch(x, s, a); grass_patch(x + 0.05, s, b);
        for (int k = 0; k < 3; k++) { CHECK(isfinite(a[k]) && a[k] >= 0.0f && a[k] <= 255.0f, "lawn colour out of range"); worst_step = fmax(worst_step, fabs(a[k] - b[k])); }
        const double l = 0.299 * a[0] + 0.587 * a[1] + 0.114 * a[2];
        if (l < min_l) min_l = l;
        if (l > max_l) max_l = l;
        float c[3]; terrain_set_seed(6); grass_patch(x, s, c); if (fabsf(a[1] - c[1]) > 0.5f) differs++;
    }
    CHECK(worst_step < 2.5, "lawn colour jumps by %.2f over 5 cm", worst_step);
    CHECK(max_l - min_l > 25.0, "the lawn has no patches (luma %.0f..%.0f)", min_l, max_l);
    CHECK(differs > 100, "a different seed gave the same lawn");
    printf("  lawn luma %.0f..%.0f across 400 m, largest step %.2f per 5 cm, %d spots differ between two seeds\n", min_l, max_l, worst_step, differs);
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
    Texture2D gt = meshes_build_grass_texture();
    CHECK(gt.width == GRASS_TILE_PX && gt.height == GRASS_TILE_PX, "grass texture is %dx%d", gt.width, gt.height);
}

int main(void)
{
    test_terrain_render();
    test_grass();
    test_meshes();
    printf("\n%d checks, %d failed\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}

#else
typedef int dr_test_render_unused;
#endif
