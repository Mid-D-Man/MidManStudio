// Stand-in for <emscripten/emscripten.h> so the web-only branches can be compile-checked with a plain C compiler
// (tests/platform_check.sh). Nothing here runs: it only has to make the code type-check.
#ifndef STUB_EMSCRIPTEN_H
#define STUB_EMSCRIPTEN_H
typedef void (*em_callback_func)(void);
void emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop);
// the real EM_JS keeps its body as a string for the browser; here only the signature matters
#define EM_JS(ret, name, params, ...) ret name params { return (ret)0; }
#endif
