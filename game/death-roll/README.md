# Death Roll

A one-thumb downhill game for the MidManStudio portfolio. You are a big ball with the studio logo on it, rolling down an endless procedural slope: hills to ride, ramps to launch off, trees and rocks to dodge. **One hit ends the run** (shields, when they arrive, soak a hit instead). C99 + raylib, built to WebAssembly; the whole game is meant to stay well under 1 MB with no art files except one 256 px logo.

Working title. Status: milestone 1-3 (the core loop). See *Roadmap*.

## Controls (and how they are gated)

| Device | Steer | Start / retry |
| --- | --- | --- |
| Touch | drag left / right anywhere (floating stick) | tap |
| Mouse | hold the left button and drag, same as touch | click |
| Keyboard | A / D or the arrow keys | space or enter |

* Which device is "current" is whichever was used last; it only picks the hints and prompts shown.
* Touch is read only where the platform has a screen (`DR_INPUT_TOUCH`). Browsers also send emulated mouse events after a touch, so the mouse is ignored for 0.8 s after any touch.
* The drag stick's neutral point is where the finger lands and follows the finger if it goes past full steer, so returning is quick. Full steer is 15 % of the **short** screen side: the same distance under the thumb in portrait and landscape.
* Orientation is read every frame from the screen size. Portrait: score centred at the top, camera further back and higher. Landscape: score top-left, best top-right. The field of view keeps about 74 degrees across, within limits.
* The canvas follows the browser viewport (rotation, address bar, an iframe being resized).
* A crash cannot be skipped by accident: retry needs a fresh press, 0.6 s after the crash.
* `F3`, or `?debug` in the page URL, shows an overlay: fps, device, orientation, shader status, speed, seed.

## How it works

The slope is a function, not data: `terrain_height(x, s)` = base gradient + rolling hills (value noise) + ramps + banked walls at the sides. Nothing is stored, so it is endless and free. The ball is a point that follows that ground, and leaves it when the ground falls away faster than free fall: ramps launch it, crests make it hop. No physics engine, no collision meshes. Obstacles are generated per 24 m cell from a hash of (seed, cell), so they can be thrown away behind you and regenerated identically.

Fairness rules (tested): a winding lane is always kept clear and wide enough for the ball; nothing in the first 70 m; each ramp keeps a corridor clear of obstacles (approach, lip, 75 m of landing).

Rendering draws everything **relative to the ball**, so no coordinate the GPU sees is large; with highp shaders of our own (raylib's default vertex shader is mediump, which breaks on phone GPUs) and distance fog. The ground is eleven 32 m flat-shaded chunks, rebuilt in place as you move on.

```
build-web.sh              builds the game into static/play/death-roll/
main.c                    window, main loop, run state (menu / run / crashed)
src/platform/platform.h   the ONE place that knows the platform; capability macros, always defined, used as #if DR_x
src/sim/                  plain C, no raylib: terrain, ball physics, obstacle generation, rules (headless-testable)
src/input/                device-gated steering
src/render/               shaders, chunk meshes, scene (camera, props, shadow, ball)
src/ui/hud.c              menus and HUD, laid out from the screen size and orientation
src/storage/              best distance: browser localStorage on the web, a file elsewhere (failures are swallowed)
raylib/src/shell.html     the page around the game (build-web.sh passes it to emcc)
assets/logo.png           the only asset; the ball's texture is composed from it at start-up
tests/                    headless tests
```

## Tuning

All the feel lives in a few numbers. Gameplay in `src/sim/sim_common.h` (slope, gravity, steering, drag, speeds), generation in `src/sim/terrain.c` (ramp chance and size, hill height, lane) and `src/sim/world.c` (obstacle count, difficulty ramp, lane width), camera in `view_params()` in `src/render/scene.c`, drag sensitivity in `src/input/input.c`. Terminal speed on the base slope is about `sqrt(DR_SLOPE * DR_SLOPE_ACCEL / DR_DRAG_Q)` (34 m/s), rising towards 50 m/s over the first 6 km.

## Where it lives, and how to build

This game lives inside the portfolio repo, in `game/death-roll/`, and builds straight into the site's static folder, so there is no separate project to create. Nothing here touches the SvelteKit build.

```
sh game/death-roll/build-web.sh
```

That runs the same `emcc` command mray runs (needs emscripten and raylib's web build; raylib is looked up in `~/raylib-test/raylib`, override with `RAYLIB=...`) and writes `index.html`, `index.js`, `index.wasm` and `index.data` to `static/play/death-roll/`. Commit that folder; the site serves it at `/play/death-roll/`. To try it first: `cd static/play/death-roll && python3 -m http.server 8080`, and add `?debug` to the URL for the overlay.

## Tests

```
sh game/death-roll/tests/run_tests.sh        # simulation + render geometry; needs only a C compiler
sh game/death-roll/tests/platform_check.sh   # every file as desktop and as web (stub emscripten header), with -Wundef
```
Point `RAYLIB_SRC` at raylib's headers if they are not in `~/raylib-test/raylib/src`. The test files are empty translation units unless built with their `-D` flag, so a build that compiles every `.c` never sees a second `main()`.

The simulation tests play thousands of metres with bots: a lane-following bot never dies over 12 seeds x 20 km; a ramp bot launches and lands safely; shields absorb a hit; a crashed ball comes to rest. The render tests run the real mesh-building code with raylib's GPU calls stubbed: every triangle faces up, chunks stream correctly for 20 km at 2 rebuilds a frame, nothing handed to the GPU is large.

**Not yet verified:** anything that needs a GL context or a touchscreen (the look, the fog, the feel of the drag stick, frame rate on a phone, rotating the device). That is what the first run on the phone is for; the debug overlay says whether both shaders compiled.

## Roadmap

1. **Pickups**: coins (saved total), boost, shields (max 3; a hit slows you instead of ending the run). The shield rules are already in the sim and tested, there is just nothing to pick up yet.
2. Unlocks: spend saved points on ball skins and trails.
3. Smash mechanic: small bushes break and cost a little speed; only boulders and big trees kill.
4. Synthesised sound (reuse the synth from Tower of Babylon), unlocked on the first touch.
5. Speed effects, daily seed with a shareable score.
6. The Play page on the site: click-to-play, served from the same origin as the page so the saved score is not partitioned away.
