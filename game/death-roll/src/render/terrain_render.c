#include "terrain_render.h"
#include "../sim/terrain.h"
#include "raymath.h"
#include <limits.h>
#include <math.h>
#include <string.h>

#define CH_LEN   32.0
#define COLS     28
#define ROWS     24
#define XL       27.0                 // the mesh spans -XL..XL (the ball can reach 21; the banks rise beyond)
#define AHEAD    9
#define BEHIND   1
#define NVERTS   (COLS * ROWS * 6)

static float lerpf(float a, float b, float t) { return a + (b - a) * t; }

static void fill_chunk(Mesh *m, int c)
{
    static float H[(COLS + 1) * (ROWS + 1)];
    const double s0 = c * CH_LEN, y0 = -DR_SLOPE * s0;
    const double dx = 2.0 * XL / COLS, ds = CH_LEN / ROWS;

    for (int j = 0; j <= ROWS; j++)
        for (int i = 0; i <= COLS; i++)
            H[j * (COLS + 1) + i] = (float)(terrain_height(-XL + i * dx, s0 + j * ds) - y0);

    float *pos = m->vertices;
    unsigned char *col = m->colors;
    int v = 0;
    for (int j = 0; j < ROWS; j++) {
        for (int i = 0; i < COLS; i++) {
            const float x0 = (float)(-XL + i * dx), x1 = (float)(-XL + (i + 1) * dx);
            const float zN = -(float)(j * ds), zF = -(float)((j + 1) * ds);
            const float hNL = H[j * (COLS + 1) + i],       hNR = H[j * (COLS + 1) + i + 1];
            const float hFL = H[(j + 1) * (COLS + 1) + i], hFR = H[(j + 1) * (COLS + 1) + i + 1];

            // the quad's base colour: track, banked wall, or ramp
            const double xc = 0.5 * (x0 + x1), sc = s0 + (j + 0.5) * ds;
            const float mask  = (float)terrain_ramp_mask(xc, sc);
            const float bankf = (float)dr_clamp((fabs(xc) - DR_TRACK_HALF) / 8.0, 0.0, 1.0);
            float r = lerpf(34.0f, 16.0f, bankf), g = lerpf(62.0f, 30.0f, bankf), b = lerpf(70.0f, 46.0f, bankf);
            r = lerpf(r, 84.0f, mask * 0.9f); g = lerpf(g, 236.0f, mask * 0.9f); b = lerpf(b, 120.0f, mask * 0.9f);
            const float stripe = (((long)floor(sc / 4.0)) & 1L) ? 1.07f : 0.93f;           // a speed cue on the plain slope
            const float vary = 0.94f + 0.12f * (float)dr_unit(dr_hash3((uint32_t)i, (uint32_t)(j + c * ROWS), 11u));

            // two triangles: (far-left, near-left, far-right) and (far-right, near-left, near-right)
            const float P[2][3][3] = {
                { { x0, hFL, zF }, { x0, hNL, zN }, { x1, hFR, zF } },
                { { x1, hFR, zF }, { x0, hNL, zN }, { x1, hNR, zN } } };
            for (int t = 0; t < 2; t++) {
                const float ux = P[t][1][0] - P[t][0][0], uy = P[t][1][1] - P[t][0][1], uz = P[t][1][2] - P[t][0][2];
                const float wx = P[t][2][0] - P[t][0][0], wy = P[t][2][1] - P[t][0][1], wz = P[t][2][2] - P[t][0][2];
                float nx = uy * wz - uz * wy, ny = uz * wx - ux * wz, nz = ux * wy - uy * wx;
                const float len = sqrtf(nx * nx + ny * ny + nz * nz);
                if (len > 1e-9f) { nx /= len; ny /= len; nz /= len; }
                if (ny < 0.0f) { nx = -nx; ny = -ny; nz = -nz; }
                float l = nx * -0.38f + ny * 0.82f + nz * 0.42f;
                if (l < 0.0f) l = 0.0f;
                const float shade = (0.40f + 0.85f * l) * stripe * vary;
                const float cr = r * shade, cg = g * shade, cb = b * shade;
                for (int k = 0; k < 3; k++) {
                    pos[v * 3 + 0] = P[t][k][0]; pos[v * 3 + 1] = P[t][k][1]; pos[v * 3 + 2] = P[t][k][2];
                    col[v * 4 + 0] = (unsigned char)(cr > 255.0f ? 255.0f : cr);
                    col[v * 4 + 1] = (unsigned char)(cg > 255.0f ? 255.0f : cg);
                    col[v * 4 + 2] = (unsigned char)(cb > 255.0f ? 255.0f : cb);
                    col[v * 4 + 3] = 255;
                    v++;
                }
            }
        }
    }
}

void terrain_render_init(TerrainRender *t)
{
    memset(t, 0, sizeof *t);
    for (int i = 0; i < TR_SLOTS; i++) {
        Mesh *m = &t->slots[i].mesh;
        m->vertexCount = NVERTS;
        m->triangleCount = NVERTS / 3;
        m->vertices = (float *)MemAlloc((unsigned)(NVERTS * 3) * sizeof(float));
        m->colors = (unsigned char *)MemAlloc((unsigned)(NVERTS * 4));
        m->texcoords = (float *)MemAlloc((unsigned)(NVERTS * 2) * sizeof(float));      // unused, but every attribute the shader
        m->normals = (float *)MemAlloc((unsigned)(NVERTS * 3) * sizeof(float));        // declares is supplied (see meshes.c)
        for (int v = 0; v < NVERTS; v++) m->normals[v * 3 + 1] = 1.0f;
        UploadMesh(m, true);                                   // dynamic: rewritten in place as the ball moves on
        t->slots[i].index = INT_MIN;
    }
}

void terrain_render_invalidate(TerrainRender *t)
{
    for (int i = 0; i < TR_SLOTS; i++) t->slots[i].index = INT_MIN;
}

static void rebuild(TerrainRender *t, TrSlot *s, int c)
{
    fill_chunk(&s->mesh, c);
    UpdateMeshBuffer(s->mesh, 0, s->mesh.vertices, NVERTS * 3 * (int)sizeof(float), 0);
    UpdateMeshBuffer(s->mesh, 3, s->mesh.colors, NVERTS * 4, 0);
    s->index = c;
    t->rebuilt_total++;
}

void terrain_render_update(TerrainRender *t, double ball_s, int max_rebuild)
{
    const int c0 = (int)floor(ball_s / CH_LEN);
    const int lo = c0 - BEHIND, hi = c0 + AHEAD;
    int built = 0;
    for (int n = 0; n <= BEHIND + AHEAD; n++) {                 // nearest first: the ball's own chunk, then ahead, then behind
        const int c = (n <= AHEAD) ? c0 + n : c0 - (n - AHEAD);
        int have = 0;
        for (int i = 0; i < TR_SLOTS; i++) if (t->slots[i].index == c) { have = 1; break; }
        if (have) continue;
        TrSlot *free_slot = NULL;
        for (int i = 0; i < TR_SLOTS; i++) {
            const int idx = t->slots[i].index;
            if (idx == INT_MIN || idx < lo || idx > hi) { free_slot = &t->slots[i]; break; }
        }
        if (!free_slot) continue;
        if (max_rebuild > 0 && built >= max_rebuild) return;
        rebuild(t, free_slot, c);
        built++;
    }
}

void terrain_render_draw(const TerrainRender *t, Material mat, double ball_y, double ball_s)
{
    for (int i = 0; i < TR_SLOTS; i++) {
        const int c = t->slots[i].index;
        if (c == INT_MIN) continue;
        const double s0 = c * CH_LEN;
        if (s0 + CH_LEN < ball_s - 25.0 || s0 > ball_s + 330.0) continue;
        const Matrix tr = MatrixTranslate(0.0f, (float)(-DR_SLOPE * s0 - ball_y), (float)(-(s0 - ball_s)));
        DrawMesh(t->slots[i].mesh, mat, tr);
    }
}
