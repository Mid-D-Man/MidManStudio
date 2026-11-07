#include "meshes.h"
#include "../sim/sim_common.h"
#include "grass.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// ---- a tiny triangle soup builder with flat per-face lighting baked into the vertex colours -------------------------
typedef struct { float *pos; unsigned char *col; int n, cap; } Soup;

static void soup_init(Soup *s, int cap_tris) { s->cap = cap_tris * 3; s->n = 0; s->pos = (float *)MemAlloc((unsigned)(s->cap * 3) * sizeof(float)); s->col = (unsigned char *)MemAlloc((unsigned)(s->cap * 4)); }

static float shade_for(Vector3 a, Vector3 b, Vector3 c)
{
    const float ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z, vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
    float nx = uy * vz - uz * vy, ny = uz * vx - ux * vz, nz = ux * vy - uy * vx;
    const float len = sqrtf(nx * nx + ny * ny + nz * nz);
    if (len < 1e-9f) return 1.0f;
    nx /= len; ny /= len; nz /= len;
    // outward is away from the object's centre line; the sign of the winding does not matter, so use the facing of the light
    const float l = fabsf(nx * -0.38f + ny * 0.82f + nz * 0.42f);
    return 0.42f + 0.78f * l;
}

static void soup_tri(Soup *s, Vector3 a, Vector3 b, Vector3 c, Color base)
{
    if (s->n + 3 > s->cap) return;
    {   // a zero-area triangle (the collapsed end of a ring) draws nothing: leave it out
        const float ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z, vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
        const float cx = uy * vz - uz * vy, cy = uz * vx - ux * vz, cz = ux * vy - uy * vx;
        if (cx * cx + cy * cy + cz * cz < 1e-12f) return;
    }
    const float sh = shade_for(a, b, c);
    const Vector3 v[3] = { a, b, c };
    for (int i = 0; i < 3; i++) {
        float *p = &s->pos[(s->n + i) * 3];
        p[0] = v[i].x; p[1] = v[i].y; p[2] = v[i].z;
        unsigned char *q = &s->col[(s->n + i) * 4];
        const float r = base.r * sh, g = base.g * sh, bl = base.b * sh;
        q[0] = (unsigned char)(r > 255.0f ? 255.0f : r);
        q[1] = (unsigned char)(g > 255.0f ? 255.0f : g);
        q[2] = (unsigned char)(bl > 255.0f ? 255.0f : bl);
        q[3] = 255;
    }
    s->n += 3;
}

static Mesh soup_to_mesh(Soup *s)
{
    Mesh m;
    memset(&m, 0, sizeof m);
    m.vertexCount = s->n;
    m.triangleCount = s->n / 3;
    m.vertices = s->pos;
    m.colors = s->col;
    // Normals and texcoords are not used (the lighting is baked into the colours) but are supplied anyway: a shader that
    // declares those attributes needs them bound on every draw, and WebGL 1 without vertex array objects fails the draw otherwise.
    m.texcoords = (float *)MemAlloc((unsigned)(s->n * 2) * sizeof(float));
    m.normals = (float *)MemAlloc((unsigned)(s->n * 3) * sizeof(float));
    for (int i = 0; i < s->n; i++) m.normals[i * 3 + 1] = 1.0f;
    UploadMesh(&m, false);
    return m;
}

static void cone(Soup *s, float cx, float cz, float y0, float y1, float radius, int slices, Color a, Color b)
{
    for (int i = 0; i < slices; i++) {
        const float a0 = (float)i / slices * 2.0f * (float)DR_PI, a1 = (float)(i + 1) / slices * 2.0f * (float)DR_PI;
        const Vector3 p0 = { cx + cosf(a0) * radius, y0, cz + sinf(a0) * radius };
        const Vector3 p1 = { cx + cosf(a1) * radius, y0, cz + sinf(a1) * radius };
        const Vector3 top = { cx, y1, cz };
        soup_tri(s, p0, top, p1, (i & 1) ? a : b);
    }
}

static void prism(Soup *s, float radius, float y0, float y1, int slices, Color base)
{
    for (int i = 0; i < slices; i++) {
        const float a0 = (float)i / slices * 2.0f * (float)DR_PI, a1 = (float)(i + 1) / slices * 2.0f * (float)DR_PI;
        const Vector3 b0 = { cosf(a0) * radius, y0, sinf(a0) * radius }, b1 = { cosf(a1) * radius, y0, sinf(a1) * radius };
        const Vector3 t0 = { b0.x, y1, b0.z }, t1 = { b1.x, y1, b1.z };
        soup_tri(s, b0, t0, b1, base);
        soup_tri(s, b1, t0, t1, base);
    }
}

// ---- a faceted solid of revolution: the building block of the new trees and rocks ---------------------------------------
// `rings` run bottom to top: t is the height along the solid (0..1), rho the radius there as a fraction of `radius` (0 collapses
// the ring to a point, which is how a tip or a bottom closes). Two rings at the same t make a flat cap. Radii and angles are
// jittered by a hash so each facet differs; `lean` slides the upper rings sideways (t squared) for a bent spire.
typedef struct { float t, rho; } Ring;

#define NO_COLOR ((Color){ 0, 0, 0, 0 })      // alpha 0: "no separate top colour"

static Vector3 lathe_pt(float cx, float cz, float y0, float y1, float radius, const Ring *r, int ring, int seg, int segs,
                        float jitter, float lean_x, float lean_z, uint32_t seed)
{
    const int j = seg % segs;
    float rho = r[ring].rho * radius;
    float ang = (float)j / (float)segs;
    if (rho > 0.0f) {
        const float u = (float)dr_unit(dr_hash3(seed, (uint32_t)ring, (uint32_t)j));
        const float w = (float)dr_unit(dr_hash3(seed ^ 0x5bd1e995u, (uint32_t)ring, (uint32_t)j));
        rho *= 1.0f + jitter * (2.0f * u - 1.0f);
        ang += (w - 0.5f) * 0.35f / (float)segs;
    }
    const float t = r[ring].t;
    const float a = ang * 2.0f * (float)DR_PI;
    return (Vector3){ cx + cosf(a) * rho + lean_x * t * t, y0 + (y1 - y0) * t, cz + sinf(a) * rho + lean_z * t * t };
}

static void lathe(Soup *s, float cx, float cz, float y0, float y1, float radius, const Ring *rings, int n, int segs,
                  float jitter, float lean_x, float lean_z, uint32_t seed, Color a, Color b, Color top)
{
    const float mid_y = y0 + 0.5f * (y1 - y0);
    for (int i = 0; i + 1 < n; i++) {
        for (int j = 0; j < segs; j++) {
            const Vector3 p00 = lathe_pt(cx, cz, y0, y1, radius, rings, i,     j,     segs, jitter, lean_x, lean_z, seed);
            const Vector3 p01 = lathe_pt(cx, cz, y0, y1, radius, rings, i,     j + 1, segs, jitter, lean_x, lean_z, seed);
            const Vector3 p11 = lathe_pt(cx, cz, y0, y1, radius, rings, i + 1, j + 1, segs, jitter, lean_x, lean_z, seed);
            const Vector3 p10 = lathe_pt(cx, cz, y0, y1, radius, rings, i + 1, j,     segs, jitter, lean_x, lean_z, seed);
            const Vector3 tri[2][3] = { { p00, p01, p11 }, { p00, p11, p10 } };
            for (int k = 0; k < 2; k++) {
                Color col = ((i + j + k) & 1) ? a : b;
                if (top.a != 0) {      // faces that look up, in the upper half: the moss, the autumn crown, the snow
                    const Vector3 u = { tri[k][1].x - tri[k][0].x, tri[k][1].y - tri[k][0].y, tri[k][1].z - tri[k][0].z };
                    const Vector3 v = { tri[k][2].x - tri[k][0].x, tri[k][2].y - tri[k][0].y, tri[k][2].z - tri[k][0].z };
                    const float nx = u.y * v.z - u.z * v.y, ny = u.z * v.x - u.x * v.z, nz = u.x * v.y - u.y * v.x;
                    const float len = sqrtf(nx * nx + ny * ny + nz * nz);
                    const float cy = (tri[k][0].y + tri[k][1].y + tri[k][2].y) / 3.0f;
                    if (len > 1e-9f && fabsf(ny) / len > 0.5f && cy > mid_y) col = top;
                }
                soup_tri(s, tri[k][0], tri[k][1], tri[k][2], col);
            }
        }
    }
}

// a thin four-sided tube from A to B (the branches of the bare tree)
static void branch(Soup *s, Vector3 A, Vector3 B, float rA, float rB, Color c0, Color c1)
{
    Vector3 d = { B.x - A.x, B.y - A.y, B.z - A.z };
    const float L = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
    if (L < 1e-6f) return;
    d.x /= L; d.y /= L; d.z /= L;
    const Vector3 ref = fabsf(d.y) > 0.9f ? (Vector3){ 1.0f, 0.0f, 0.0f } : (Vector3){ 0.0f, 1.0f, 0.0f };
    Vector3 u = { d.y * ref.z - d.z * ref.y, d.z * ref.x - d.x * ref.z, d.x * ref.y - d.y * ref.x };
    const float ul = sqrtf(u.x * u.x + u.y * u.y + u.z * u.z);
    u.x /= ul; u.y /= ul; u.z /= ul;
    const Vector3 w = { d.y * u.z - d.z * u.y, d.z * u.x - d.x * u.z, d.x * u.y - d.y * u.x };
    for (int i = 0; i < 4; i++) {
        const float a0 = (float)i / 4.0f * 2.0f * (float)DR_PI, a1 = (float)(i + 1) / 4.0f * 2.0f * (float)DR_PI;
        const Vector3 o0 = { u.x * cosf(a0) + w.x * sinf(a0), u.y * cosf(a0) + w.y * sinf(a0), u.z * cosf(a0) + w.z * sinf(a0) };
        const Vector3 o1 = { u.x * cosf(a1) + w.x * sinf(a1), u.y * cosf(a1) + w.y * sinf(a1), u.z * cosf(a1) + w.z * sinf(a1) };
        const Vector3 p0 = { A.x + o0.x * rA, A.y + o0.y * rA, A.z + o0.z * rA }, p1 = { A.x + o1.x * rA, A.y + o1.y * rA, A.z + o1.z * rA };
        const Vector3 q0 = { B.x + o0.x * rB, B.y + o0.y * rB, B.z + o0.z * rB }, q1 = { B.x + o1.x * rB, B.y + o1.y * rB, B.z + o1.z * rB };
        soup_tri(s, p0, q0, p1, (i & 1) ? c0 : c1);
        soup_tri(s, p1, q0, q1, (i & 1) ? c0 : c1);
    }
}

// ---- trees: unit height 1 with the base at y = 0, canopy radius at most 0.30 (the scene scales the radius by the obstacle's r) ----
static const Ring OVAL[6]   = { { 0.00f, 0.00f }, { 0.16f, 0.74f }, { 0.42f, 1.00f }, { 0.70f, 0.88f }, { 0.90f, 0.52f }, { 1.00f, 0.00f } };
static const Ring TRUNK[3]  = { { 0.00f, 1.00f }, { 0.30f, 0.62f }, { 1.00f, 0.35f } };

Mesh meshes_build_tree_variant(int v)
{
    Soup s; soup_init(&s, 300);
    switch (v) {
    default:
    case 0:     // spruce: two cones (the original tree)
        prism(&s, 0.05f, 0.0f, 0.22f, 6, (Color){ 92, 64, 44, 255 });
        cone(&s, 0.0f, 0.0f, 0.12f, 0.62f, 0.30f, 7, (Color){ 40, 148, 96, 255 }, (Color){ 30, 120, 84, 255 });
        cone(&s, 0.0f, 0.0f, 0.42f, 1.00f, 0.22f, 7, (Color){ 56, 176, 112, 255 }, (Color){ 40, 146, 98, 255 });
        break;
    case 1:     // tall pine: four slim tiers, cooler and darker
        prism(&s, 0.045f, 0.0f, 0.22f, 6, (Color){ 88, 62, 46, 255 });
        cone(&s, 0.0f, 0.0f, 0.10f, 0.46f, 0.30f, 8, (Color){ 34, 112, 92, 255 }, (Color){ 26, 92, 76, 255 });
        cone(&s, 0.0f, 0.0f, 0.30f, 0.66f, 0.255f, 8, (Color){ 42, 128, 100, 255 }, (Color){ 32, 106, 86, 255 });
        cone(&s, 0.0f, 0.0f, 0.50f, 0.84f, 0.20f, 8, (Color){ 52, 146, 108, 255 }, (Color){ 40, 122, 94, 255 });
        cone(&s, 0.0f, 0.0f, 0.68f, 1.00f, 0.13f, 8, (Color){ 66, 164, 118, 255 }, (Color){ 50, 140, 104, 255 });
        break;
    case 2:     // oval broadleaf, green
        prism(&s, 0.05f, 0.0f, 0.36f, 6, (Color){ 96, 66, 46, 255 });
        lathe(&s, 0.0f, 0.0f, 0.22f, 1.0f, 0.28f, OVAL, 6, 7, 0.08f, 0.0f, 0.0f, 21u,
              (Color){ 72, 170, 84, 255 }, (Color){ 58, 146, 72, 255 }, (Color){ 104, 196, 100, 255 });
        break;
    case 3:     // the same tree in autumn colours
        prism(&s, 0.05f, 0.0f, 0.36f, 6, (Color){ 84, 58, 42, 255 });
        lathe(&s, 0.0f, 0.0f, 0.22f, 1.0f, 0.28f, OVAL, 6, 7, 0.08f, 0.0f, 0.0f, 31u,
              (Color){ 236, 138, 44, 255 }, (Color){ 206, 98, 40, 255 }, (Color){ 250, 178, 64, 255 });
        break;
    case 4:     // birch: pale trunk, two light clumps of leaves
        prism(&s, 0.038f, 0.0f, 0.80f, 6, (Color){ 226, 224, 212, 255 });
        lathe(&s, 0.0f, 0.0f, 0.46f, 1.0f, 0.24f, OVAL, 6, 7, 0.08f, 0.0f, 0.0f, 41u,
              (Color){ 150, 202, 96, 255 }, (Color){ 124, 178, 84, 255 }, (Color){ 182, 220, 114, 255 });
        lathe(&s, 0.10f, -0.06f, 0.30f, 0.78f, 0.15f, OVAL, 6, 6, 0.08f, 0.0f, 0.0f, 42u,
              (Color){ 138, 192, 90, 255 }, (Color){ 112, 166, 78, 255 }, (Color){ 168, 210, 106, 255 });
        break;
    case 5: {   // bare dead tree: a thick tapering trunk with six branches (wide enough that a hit never feels like it came from nothing)
        const Color c0 = { 104, 92, 82, 255 }, c1 = { 72, 62, 56, 255 };
        lathe(&s, 0.0f, 0.0f, 0.0f, 1.0f, 0.085f, TRUNK, 3, 6, 0.06f, 0.0f, 0.0f, 51u, c0, c1, NO_COLOR);
        branch(&s, (Vector3){ 0.0f, 0.22f, 0.0f }, (Vector3){  0.27f, 0.46f,  0.02f }, 0.035f, 0.012f, c0, c1);
        branch(&s, (Vector3){ 0.0f, 0.34f, 0.0f }, (Vector3){ -0.25f, 0.66f,  0.10f }, 0.030f, 0.010f, c0, c1);
        branch(&s, (Vector3){ 0.0f, 0.46f, 0.0f }, (Vector3){  0.04f, 0.72f,  0.27f }, 0.026f, 0.009f, c0, c1);
        branch(&s, (Vector3){ 0.0f, 0.50f, 0.0f }, (Vector3){  0.10f, 0.80f, -0.24f }, 0.028f, 0.010f, c0, c1);
        branch(&s, (Vector3){ 0.0f, 0.62f, 0.0f }, (Vector3){ -0.12f, 0.90f, -0.18f }, 0.024f, 0.008f, c0, c1);
        branch(&s, (Vector3){ 0.0f, 0.78f, 0.0f }, (Vector3){  0.16f, 0.98f,  0.10f }, 0.020f, 0.008f, c0, c1);
        break; }
    }
    return soup_to_mesh(&s);
}

Mesh meshes_build_tree(void) { return meshes_build_tree_variant(0); }

// ---- rocks: unit radius, y from -1 to 1 around the origin (the scene squashes and buries the lower part) --------------------
static const Ring BOULDER[6] = { { 0.00f, 0.00f }, { 0.20f, 0.80f }, { 0.50f, 1.00f }, { 0.78f, 0.82f }, { 0.94f, 0.44f }, { 1.00f, 0.00f } };
static const Ring MESA[7]    = { { 0.00f, 0.00f }, { 0.10f, 0.72f }, { 0.24f, 0.98f }, { 0.66f, 1.00f }, { 0.88f, 0.88f }, { 1.00f, 0.74f }, { 1.00f, 0.00f } };
static const Ring SPIRE[5]   = { { 0.00f, 0.00f }, { 0.12f, 0.92f }, { 0.40f, 0.66f }, { 0.72f, 0.32f }, { 1.00f, 0.00f } };
static const Ring SHARD[4]   = { { 0.00f, 0.00f }, { 0.14f, 0.90f }, { 0.55f, 0.72f }, { 1.00f, 0.00f } };

Mesh meshes_build_rock_variant(int v)
{
    const Color grey_a = { 150, 150, 160, 255 }, grey_b = { 124, 126, 138, 255 };
    Soup s; soup_init(&s, 300);
    switch (v) {
    default:
    case 0: {   // grey boulder: a roughened icosahedron (the original rock)
        const float t = (1.0f + sqrtf(5.0f)) * 0.5f;
        Vector3 p[12] = {
            { -1,  t,  0 }, {  1,  t,  0 }, { -1, -t,  0 }, {  1, -t,  0 },
            {  0, -1,  t }, {  0,  1,  t }, {  0, -1, -t }, {  0,  1, -t },
            {  t,  0, -1 }, {  t,  0,  1 }, { -t,  0, -1 }, { -t,  0,  1 } };
        for (int i = 0; i < 12; i++) {                     // normalise, then roughen so no two faces look alike
            const float len = sqrtf(p[i].x * p[i].x + p[i].y * p[i].y + p[i].z * p[i].z);
            const float k = (0.84f + 0.30f * (float)dr_unit(dr_hash3((uint32_t)i, 0xA0u, 7u))) / len;
            p[i].x *= k; p[i].y *= k; p[i].z *= k;
        }
        static const int f[20][3] = {
            { 0, 11, 5 }, { 0, 5, 1 }, { 0, 1, 7 }, { 0, 7, 10 }, { 0, 10, 11 },
            { 1, 5, 9 }, { 5, 11, 4 }, { 11, 10, 2 }, { 10, 7, 6 }, { 7, 1, 8 },
            { 3, 9, 4 }, { 3, 4, 2 }, { 3, 2, 6 }, { 3, 6, 8 }, { 3, 8, 9 },
            { 4, 9, 5 }, { 2, 4, 11 }, { 6, 2, 10 }, { 8, 6, 7 }, { 9, 8, 1 } };
        for (int i = 0; i < 20; i++) soup_tri(&s, p[f[i][0]], p[f[i][1]], p[f[i][2]], (i & 1) ? (Color){ 150, 150, 160, 255 } : (Color){ 124, 126, 138, 255 });
        break; }
    case 1:     // warm sandstone boulder
        lathe(&s, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, BOULDER, 6, 7, 0.14f, 0.08f, 0.04f, 71u,
              (Color){ 176, 150, 122, 255 }, (Color){ 150, 126, 102, 255 }, NO_COLOR);
        break;
    case 2:     // dark slate mesa: steep sides, a flat top
        lathe(&s, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, MESA, 7, 6, 0.10f, 0.0f, 0.0f, 52u,
              (Color){ 96, 102, 116, 255 }, (Color){ 76, 82, 98, 255 }, NO_COLOR);
        break;
    case 3:     // the same with a mossy top
        lathe(&s, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, MESA, 7, 7, 0.12f, 0.0f, 0.0f, 53u,
              (Color){ 132, 130, 126, 255 }, (Color){ 108, 106, 104, 255 }, (Color){ 88, 142, 78, 255 });
        break;
    case 4:     // grey spire leaning one way
        lathe(&s, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, SPIRE, 5, 6, 0.12f, 0.20f, -0.06f, 54u, grey_a, grey_b, NO_COLOR);
        break;
    case 5:     // warm spire leaning the other way
        lathe(&s, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, SPIRE, 5, 6, 0.12f, -0.16f, 0.10f, 55u,
              (Color){ 190, 164, 128, 255 }, (Color){ 160, 134, 104, 255 }, NO_COLOR);
        break;
    case 6:     // a cluster of three shards
        lathe(&s, 0.00f,  0.00f, -1.0f, 1.00f, 0.62f, SHARD, 4, 6, 0.10f,  0.05f, 0.03f, 61u, grey_a, grey_b, NO_COLOR);
        lathe(&s, 0.48f,  0.22f, -1.0f, 0.40f, 0.46f, SHARD, 4, 6, 0.10f,  0.10f, 0.00f, 62u, grey_a, grey_b, NO_COLOR);
        lathe(&s, -0.42f, -0.34f, -1.0f, 0.05f, 0.42f, SHARD, 4, 6, 0.10f, -0.08f, 0.05f, 63u, grey_a, grey_b, NO_COLOR);
        break;
    case 7:     // two mossy boulders, one big one small
        lathe(&s, -0.28f, 0.05f, -1.0f, 1.00f, 0.74f, BOULDER, 6, 7, 0.12f, 0.05f, 0.0f, 81u,
              (Color){ 128, 128, 124, 255 }, (Color){ 104, 104, 104, 255 }, (Color){ 92, 146, 80, 255 });
        lathe(&s, 0.52f, -0.12f, -1.0f, 0.35f, 0.50f, BOULDER, 6, 6, 0.12f, -0.04f, 0.0f, 82u,
              (Color){ 128, 128, 124, 255 }, (Color){ 104, 104, 104, 255 }, (Color){ 92, 146, 80, 255 });
        break;
    }
    return soup_to_mesh(&s);
}

Mesh meshes_build_rock(void) { return meshes_build_rock_variant(0); }

Texture2D meshes_build_ball_texture(void)
{
    // 512 x 256 maps once around the ball, so a square on the equator stays square. Body: pearl; logo: four around the
    // equator and four smaller ones above and below, offset by an eighth of a turn; two neon bands for spin to read against.
    Image img = GenImageColor(512, 256, (Color){ 226, 233, 238, 255 });
    ImageDrawRectangle(&img, 0, 70, 512, 3, (Color){ 60, 255, 90, 255 });     // between the small logos and the equator logos
    ImageDrawRectangle(&img, 0, 184, 512, 3, (Color){ 60, 255, 90, 255 });

    Image logo = LoadImage("assets/logo.png");
    if (logo.data != NULL) {
        ImageFormat(&logo, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        Image big = ImageCopy(logo);
        ImageResize(&big, 104, 104);
        ImageResize(&logo, 64, 64);
        for (int i = 0; i < 4; i++) {
            ImageDrawImage(&img, big, i * 128 + 12, 76, WHITE);               // equator: 104 px, centred at u = i/4 + 1/8
            ImageDrawImage(&img, logo, i * 128 + 64, 4, WHITE);               // above and below, shifted a sixteenth of a turn
            ImageDrawImage(&img, logo, i * 128 + 64, 188, WHITE);
        }
        UnloadImage(big);
        UnloadImage(logo);
    }
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
    return tex;
}

Texture2D meshes_build_grass_texture(void)
{
    // GRASS_TILE_PX is a power of two on purpose: WebGL 1 only repeats and mipmaps those. Repeat wrap is what makes the ground
    // seamless, mipmaps keep the far ground from shimmering at speed.
    Image img = GenImageColor(GRASS_TILE_PX, GRASS_TILE_PX, WHITE);            // R8G8B8A8
    if (img.data != NULL) {
        unsigned char *px = (unsigned char *)img.data;
        for (int y = 0; y < GRASS_TILE_PX; y++)
            for (int x = 0; x < GRASS_TILE_PX; x++) {
                unsigned char *p = &px[(y * GRASS_TILE_PX + x) * 4];
                grass_texel(x, y, p);
                p[3] = 255;
            }
    }
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureWrap(tex, TEXTURE_WRAP_REPEAT);
    GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);                          // after the mipmaps exist
    return tex;
}
