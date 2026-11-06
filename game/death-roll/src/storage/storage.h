// The best distance, kept between visits: the browser's localStorage on the web, a small file elsewhere. Every failure
// (storage blocked in a private window or a sandboxed iframe, no write permission) is swallowed: the game just forgets.
#ifndef DR_STORAGE_H
#define DR_STORAGE_H

int  storage_load_best(void);        // 0 when nothing is stored
void storage_save_best(int metres);

#endif
