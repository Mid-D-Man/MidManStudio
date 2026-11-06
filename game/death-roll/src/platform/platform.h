// ============================================================================
// The one place that knows which platform this is built for. Nothing else in the game tests __EMSCRIPTEN__: code asks for
// what it needs (a CAPABILITY below) and this file says which platforms have it. Every macro is always defined, as 1 or 0,
// and used as `#if DR_x` (never `#ifdef`), so a typo cannot silently turn a feature off: tests/platform_check.sh builds
// everything with -Wundef.
// ============================================================================
#ifndef DR_PLATFORM_H
#define DR_PLATFORM_H

#include <stdbool.h>

// ---- which platform (exactly one is 1) -----------------------------------------------------------------------------
#if defined(__EMSCRIPTEN__)
    #define DR_PLATFORM_WEB     1
    #define DR_PLATFORM_DESKTOP 0
    #define DR_PLATFORM_NAME    "web"
#else
    #define DR_PLATFORM_WEB     0
    #define DR_PLATFORM_DESKTOP 1
    #define DR_PLATFORM_NAME    "desktop"
#endif

// ---- capabilities --------------------------------------------------------------------------------------------------
// The graphics driver is OpenGL ES 2 (a browser's WebGL 1): GLSL ES 1.00 shaders compile, and raylib's own default vertex
// shader is mediump (16 bit on a phone GPU), so everything here draws with highp shaders of its own.
#define DR_GLES2                    DR_PLATFORM_WEB
#define DR_CUSTOM_SHADERS           DR_GLES2

// Touch is read (floating drag stick) and a touch layout is offered. The browser may be on a phone; desktop has no screen
// to touch. Keyboard and mouse are read on every platform: a browser can be on a PC, and a phone can have a keyboard.
#define DR_INPUT_TOUCH              DR_PLATFORM_WEB
#define DR_INPUT_KEYBOARD           1
#define DR_INPUT_MOUSE              1

// The best distance is kept in the browser's localStorage on the web, in a small file elsewhere.
#define DR_STORAGE_BROWSER          DR_PLATFORM_WEB
#define DR_STORAGE_FILE             (!DR_PLATFORM_WEB)

// The canvas follows the browser viewport (rotation, address bar, an iframe being resized) and `?debug` in the page URL
// turns the diagnostics overlay on. Neither exists off the web.
#define DR_VIEWPORT_FOLLOWS_BROWSER DR_PLATFORM_WEB
#define DR_URL_DEBUG                DR_PLATFORM_WEB

// ---- the few calls that need the browser (implemented in platform.c; harmless stubs elsewhere) ----------------------
int  platform_viewport_w(void);        // browser viewport in CSS pixels, 0 when not applicable
int  platform_viewport_h(void);
bool platform_touch_capable(void);     // the device's primary pointer is coarse (a finger)
bool platform_debug_requested(void);   // ?debug in the URL

#endif
