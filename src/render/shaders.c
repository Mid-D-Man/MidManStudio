#include "shaders.h"
#include "../platform/platform.h"
#include "rlgl.h"

// Sky and fog share one colour so far ground fades into the horizon.
#define FOG_R 0.086f
#define FOG_G 0.133f
#define FOG_B 0.196f
#define FOG_NEAR  95.0f
#define FOG_FAR  290.0f

#if DR_CUSTOM_SHADERS
// raylib's default ES2 shaders with only the precision lines changed (same as the Tower of Babylon game's highp shader).
static const char *const HUD_VS =
    "#version 100                       \n"
    "precision highp float;             \n"
    "attribute vec3 vertexPosition;     \n"
    "attribute vec2 vertexTexCoord;     \n"
    "attribute vec4 vertexColor;        \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
    "uniform mat4 mvp;                  \n"
    "void main()                        \n"
    "{                                  \n"
    "    fragTexCoord = vertexTexCoord; \n"
    "    fragColor = vertexColor;       \n"
    "    gl_Position = mvp*vec4(vertexPosition, 1.0); \n"
    "}                                  \n";

static const char *const HUD_FS =
    "#version 100                       \n"
    "#ifdef GL_FRAGMENT_PRECISION_HIGH  \n"
    "precision highp float;             \n"
    "#else                              \n"
    "precision mediump float;           \n"
    "#endif                             \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
    "uniform sampler2D texture0;        \n"
    "uniform vec4 colDiffuse;           \n"
    "void main()                        \n"
    "{                                  \n"
    "    vec4 texelColor = texture2D(texture0, fragTexCoord); \n"
    "    gl_FragColor = texelColor*colDiffuse*fragColor;      \n"
    "}                                  \n";

// World shader. Vertex colour * texture * colDiffuse, optionally lit by the vertex normal, then faded into the fog by view
// depth (gl_Position.w is the distance along the view direction). `useLight` is set per draw; with it off the normal is never
// touched (meshes without normals would feed it garbage).
static const char *const WORLD_VS =
    "#version 100                                  \n"
    "precision highp float;                        \n"
    "attribute vec3 vertexPosition;                \n"
    "attribute vec2 vertexTexCoord;                \n"
    "attribute vec3 vertexNormal;                  \n"
    "attribute vec4 vertexColor;                   \n"
    "uniform mat4 mvp;                             \n"
    "uniform mat4 matNormal;                       \n"
    "uniform float useLight;                       \n"
    "uniform vec3 lightDir;                        \n"
    "varying vec2 fragTexCoord;                    \n"
    "varying vec4 fragColor;                       \n"
    "varying float fragDepth;                      \n"
    "varying float fragLight;                      \n"
    "void main()                                   \n"
    "{                                             \n"
    "    fragTexCoord = vertexTexCoord;            \n"
    "    fragColor = vertexColor;                  \n"
    "    float lit = 1.0;                          \n"
    "    if (useLight > 0.5)                       \n"
    "    {                                         \n"
    "        vec3 n = normalize((matNormal*vec4(vertexNormal, 0.0)).xyz); \n"
    "        lit = 0.42 + 0.70*max(dot(n, lightDir), 0.0);                \n"
    "    }                                         \n"
    "    fragLight = lit;                          \n"
    "    vec4 p = mvp*vec4(vertexPosition, 1.0);   \n"
    "    fragDepth = p.w;                          \n"
    "    gl_Position = p;                          \n"
    "}                                             \n";

static const char *const WORLD_FS =
    "#version 100                                  \n"
    "#ifdef GL_FRAGMENT_PRECISION_HIGH             \n"
    "precision highp float;                        \n"
    "#else                                         \n"
    "precision mediump float;                      \n"
    "#endif                                        \n"
    "varying vec2 fragTexCoord;                    \n"
    "varying vec4 fragColor;                       \n"
    "varying float fragDepth;                      \n"
    "varying float fragLight;                      \n"
    "uniform sampler2D texture0;                   \n"
    "uniform vec4 colDiffuse;                      \n"
    "uniform vec4 fogColor;                        \n"
    "uniform vec2 fogRange;                        \n"
    "void main()                                   \n"
    "{                                             \n"
    "    vec4 c = texture2D(texture0, fragTexCoord)*colDiffuse*fragColor; \n"
    "    float f = clamp((fragDepth - fogRange.x)/(fogRange.y - fogRange.x), 0.0, 1.0); \n"
    "    f = f*f;                                  \n"
    "    gl_FragColor = vec4(mix(c.rgb*fragLight, fogColor.rgb, f), c.a); \n"
    "}                                             \n";
#endif

void shaders_init(Shaders *s)
{
    s->hud_ok = false;
    s->world_ok = false;
    s->loc_fog_color = s->loc_fog_range = s->loc_light_dir = s->loc_use_light = -1;
    s->hud   = (Shader){ 0 };
    s->world = (Shader){ 0 };

#if DR_CUSTOM_SHADERS
    // raylib hands back its default shader when a program fails to compile; that is the sign to fall back.
    s->hud = LoadShaderFromMemory(HUD_VS, HUD_FS);
    s->hud_ok = IsShaderValid(s->hud) && s->hud.id != rlGetShaderIdDefault();

    s->world = LoadShaderFromMemory(WORLD_VS, WORLD_FS);
    s->world_ok = IsShaderValid(s->world) && s->world.id != rlGetShaderIdDefault();
    if (s->world_ok) {
        s->loc_fog_color = GetShaderLocation(s->world, "fogColor");
        s->loc_fog_range = GetShaderLocation(s->world, "fogRange");
        s->loc_light_dir = GetShaderLocation(s->world, "lightDir");
        s->loc_use_light = GetShaderLocation(s->world, "useLight");

        const float fog[4]   = { FOG_R, FOG_G, FOG_B, 1.0f };
        const float range[2] = { FOG_NEAR, FOG_FAR };
        const float light[3] = { -0.35f, 0.80f, 0.48f };      // from above, a little to the left and toward the camera
        SetShaderValue(s->world, s->loc_fog_color, fog, SHADER_UNIFORM_VEC4);
        SetShaderValue(s->world, s->loc_fog_range, range, SHADER_UNIFORM_VEC2);
        SetShaderValue(s->world, s->loc_light_dir, light, SHADER_UNIFORM_VEC3);
    }
    shaders_use_light(s, false);
#endif
}

void shaders_use_light(const Shaders *s, bool on)
{
#if DR_CUSTOM_SHADERS
    if (!s->world_ok) return;
    const float v = on ? 1.0f : 0.0f;
    SetShaderValue(s->world, s->loc_use_light, &v, SHADER_UNIFORM_FLOAT);
#else
    (void)s; (void)on;
#endif
}
