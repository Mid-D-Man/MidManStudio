#include "platform.h"

#if DR_PLATFORM_WEB
#include <emscripten/emscripten.h>

EM_JS(int, dr_js_inner_w, (void), { return window.innerWidth | 0; });
EM_JS(int, dr_js_inner_h, (void), { return window.innerHeight | 0; });
EM_JS(int, dr_js_touch_capable, (void), {
    var coarse = window.matchMedia && window.matchMedia("(pointer: coarse)").matches;
    return ((navigator.maxTouchPoints | 0) > 0 && coarse) ? 1 : 0;
});
EM_JS(int, dr_js_debug, (void), { return (window.location.search.indexOf("debug") >= 0) ? 1 : 0; });

int  platform_viewport_w(void)      { return dr_js_inner_w(); }
int  platform_viewport_h(void)      { return dr_js_inner_h(); }
bool platform_touch_capable(void)   { return dr_js_touch_capable() != 0; }
bool platform_debug_requested(void) { return dr_js_debug() != 0; }
#else
int  platform_viewport_w(void)      { return 0; }
int  platform_viewport_h(void)      { return 0; }
bool platform_touch_capable(void)   { return false; }
bool platform_debug_requested(void) { return false; }
#endif
