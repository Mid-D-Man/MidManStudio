#include "meshes.h"
#include "../sim/sim_common.h"
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

Mesh meshes_build_tree(void)
{
    Soup s; soup_init(&s, 80);
    prism(&s, 0.05f, 0.0f, 0.22f, 6, (Color){ 92, 64, 44, 255 });
    cone(&s, 0.0f, 0.0f, 0.12f, 0.62f, 0.30f, 7, (Color){ 40, 148, 96, 255 }, (Color){ 30, 120, 84, 255 });
    cone(&s, 0.0f, 0.0f, 0.42f, 1.00f, 0.22f, 7, (Color){ 56, 176, 112, 255 }, (Color){ 40, 146, 98, 255 });
    return soup_to_mesh(&s);
}

Mesh meshes_build_rock(void)
{
    const float t = (1.0f + sqrtf(5.0f)) * 0.5f;
    Vector3 v[12] = {
        { -1,  t,  0 }, {  1,  t,  0 }, { -1, -t,  0 }, {  1, -t,  0 },
        {  0, -1,  t }, {  0,  1,  t }, {  0, -1, -t }, {  0,  1, -t },
        {  t,  0, -1 }, {  t,  0,  1 }, { -t,  0, -1 }, { -t,  0,  1 } };
    for (int i = 0; i < 12; i++) {                         // normalise, then roughen so no two faces look alike
        const float len = sqrtf(v[i].x * v[i].x + v[i].y * v[i].y + v[i].z * v[i].z);
        const float k = (0.84f + 0.30f * (float)dr_unit(dr_hash3((uint32_t)i, 0xA0u, 7u))) / len;
        v[i].x *= k; v[i].y *= k; v[i].z *= k;
    }
    static const int f[20][3] = {
        { 0, 11, 5 }, { 0, 5, 1 }, { 0, 1, 7 }, { 0, 7, 10 }, { 0, 10, 11 },
        { 1, 5, 9 }, { 5, 11, 4 }, { 11, 10, 2 }, { 10, 7, 6 }, { 7, 1, 8 },
        { 3, 9, 4 }, { 3, 4, 2 }, { 3, 2, 6 }, { 3, 6, 8 }, { 3, 8, 9 },
        { 4, 9, 5 }, { 2, 4, 11 }, { 6, 2, 10 }, { 8, 6, 7 }, { 9, 8, 1 } };
    Soup s; soup_init(&s, 24);
    for (int i = 0; i < 20; i++) soup_tri(&s, v[f[i][0]], v[f[i][1]], v[f[i][2]], (i & 1) ? (Color){ 150, 150, 160, 255 } : (Color){ 124, 126, 138, 255 });
    return soup_to_mesh(&s);
}

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
