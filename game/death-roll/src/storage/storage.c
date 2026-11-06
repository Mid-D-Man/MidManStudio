#include "storage.h"
#include "../platform/platform.h"

#if DR_STORAGE_BROWSER
#include <emscripten/emscripten.h>

EM_JS(int, dr_js_load_best, (void), {
    try { var v = window.localStorage.getItem("midmanstudio.deathroll.best"); return v ? (parseInt(v, 10) | 0) : 0; }
    catch (e) { return 0; }
});
EM_JS(void, dr_js_save_best, (int v), {
    try { window.localStorage.setItem("midmanstudio.deathroll.best", String(v)); } catch (e) { }
});

int  storage_load_best(void)      { return dr_js_load_best(); }
void storage_save_best(int m)     { dr_js_save_best(m); }
#endif

#if DR_STORAGE_FILE
#include <stdio.h>

#define BEST_FILE "deathroll_best.dat"

int storage_load_best(void)
{
    FILE *f = fopen(BEST_FILE, "rb");
    if (!f) return 0;
    int v = 0;
    if (fread(&v, sizeof v, 1, f) != 1) v = 0;
    fclose(f);
    return v < 0 ? 0 : v;
}

void storage_save_best(int m)
{
    FILE *f = fopen(BEST_FILE, "wb");
    if (!f) return;
    fwrite(&m, sizeof m, 1, f);
    fclose(f);
}
#endif
