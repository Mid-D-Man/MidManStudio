# Death Roll

A one-thumb downhill game for the MidManStudio portfolio. You are a big ball with the studio logo on it, rolling down an endless procedural slope: hills to ride, ramps to launch off, trees and rocks to dodge. **One hit ends the run** (shields, when they arrive, soak a hit instead). C99 + raylib, built to WebAssembly; the whole game is meant to stay well under 1 MB with no art files except one 256 px logo.

Working title. Status: milestone 1-3 (the core loop), plus variety in the props and a cartoon squash-and-stretch ball (see *Variety and the cartoon ball*). See *Roadmap*.

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
src/render/               shaders, chunk meshes, the tree and rock meshes, scene (camera, props, shadow, ball)
src/render/squash.c       the ball's squash and stretch (plain maths, no raylib, never read by the sim)
src/render/ball_xform.c   builds the ball's draw matrix from the squash (tested on its own)
src/ui/hud.c              menus and HUD, laid out from the screen size and orientation
src/storage/              best distance: browser localStorage on the web, a file elsewhere (failures are swallowed)
raylib/src/shell.html     the page around the game (build-web.sh passes it to emcc)
assets/logo.png           the only asset; the ball's texture is composed from it at start-up
tests/                    headless tests
```

## Variety and the cartoon ball

**Trees (6) and rocks (8).** Every obstacle gets a look from its own hash salts (`variant`, `yaw`, and for rocks a `stretch` that widens one axis and narrows the other), so the same seed always gives the same forest and adding looks did not move a single obstacle (checked with a placement fingerprint over 12 seeds x 14 km). The collision only knows `x, s, r, h` and the type: every mesh stays inside the same footprint and height, so a tall pine, a bare tree or a spire is exactly as dangerous as the spruce. The mix is set in `src/sim/world.c` (`TREE_MIX`, `ROCK_MIX`, percentages) and the shapes in `src/render/meshes.c`.

| Trees | | Rocks | |
| --- | --- | --- | --- |
| 0 spruce (30 %) | the original two-cone tree | 0 grey boulder (22 %) | the original rock |
| 1 tall pine (18 %) | four slim tiers, cooler green | 1 warm boulder (14 %) | sandstone |
| 2 oval green (16 %) | broadleaf on a trunk | 2 slate mesa (16 %) | steep sides, flat top |
| 3 autumn (14 %) | the same in orange | 3 mossy mesa (10 %) | green top faces |
| 4 birch (12 %) | pale trunk, two leaf clumps | 4 grey spire (14 %) | leaning point |
| 5 bare tree (10 %) | thick trunk, six branches | 5 warm spire (8 %) | leaning the other way |
| | | 6 shard cluster (10 %) | three shards |
| | | 7 mossy twins (6 %) | a big and a small boulder |

To add a look: raise `DR_TREE_VARIANTS` / `DR_ROCK_VARIANTS` in `src/sim/sim_common.h`, add a `case` in `meshes_build_tree_variant` / `meshes_build_rock_variant`, and add its percentage to the mix table (it must still add up to 100). The tests check every mesh's footprint, height and that no two variants are the same mesh.

**Squash and stretch.** The ball looks like a cartoon ball but is exactly the same ball to the game: `src/render/squash.c` only *reads* what the sim did (on the ground or not, speed, how hard the last landing was, the launch / land / hit / death events) and the scene draws the ball mesh with a few scale factors. Nothing flows back, and `tests/run_tests.sh` fails if the sim files ever mention a look-only field or the squash files include a sim header.

* **Landing**: snaps flat along the ground's normal (depth grows with impact speed, up to 58 % of its height), then overshoots tall, wobbles and settles like jelly (an under-damped spring, about a third of a second per wobble).
* **Take-off**: a stretch pop. **In the air**: stretched along the direction of travel, more the faster it goes (up to 28 %). **On the ground**: a hint of speed stretch (up to 5 %).
* **Shield hit and death**: their own squash.
* The squash is measured along the ground's normal, and the ball is moved by the same amount, so a squashed ball still touches the slope at the same point (tested numerically) and never sinks in or hovers. The volume is constant. The shadow widens as the ball flattens.
* Tuning: the constants at the top of `squash.c` (`SQ_OMEGA` speed of the wobble, `SQ_ZETA` how bouncy, the clamps) and the amounts in `squash_update`.

## Tuning

All the feel lives in a few numbers. Gameplay in `src/sim/sim_common.h` (slope, gravity, steering, drag, speeds), generation in `src/sim/terrain.c` (ramp chance and size, hill height, lane) and `src/sim/world.c` (obstacle count, difficulty ramp, lane width), camera in `view_params()` in `src/render/scene.c`, drag sensitivity in `src/input/input.c`. Terminal speed on the base slope is about `sqrt(DR_SLOPE * DR_SLOPE_ACCEL / DR_DRAG_Q)` (34 m/s), rising towards 50 m/s over the first 6 km.

## Where it lives, and how to build

This game lives inside the portfolio repo, in `game/death-roll/`, and builds straight into the site's static folder, so there is no separate project to create. Nothing here touches the SvelteKit build.

```
sh game/death-roll/build-web.sh
```

That runs the same `emcc` command mray runs (needs emscripten and raylib's web build; raylib is looked up in `~/raylib-test/raylib`, override with `RAYLIB=...`) and writes `index.html`, `index.js`, `index.wasm` and `index.data` to `static/play/death-roll/`. The deploy workflow (`.github/workflows/deploy.yml`) does exactly this on every push (emscripten and raylib 6.2 in CI), so you do not have to commit that folder; run it by hand only to try a build on the phone. The site serves it at `/play/death-roll/`. To try it first: `cd static/play/death-roll && python3 -m http.server 8080`, and add `?debug` to the URL for the overlay.

The studio page embeds it in its **Play** section (`src/lib/components/PlayGame.svelte`): a click-to-play iframe of `/play/death-roll/`, same origin as the page so the saved best distance is not partitioned away. If the folder is missing (a local run of the site without a game build) the section shows a "Web build coming soon" poster and becomes playable by itself once it exists.

## Tests

```
sh game/death-roll/tests/run_tests.sh        # simulation + render geometry; needs only a C compiler
sh game/death-roll/tests/platform_check.sh   # every file as desktop and as web (stub emscripten header), with -Wundef
```
Point `RAYLIB_SRC` at raylib's headers if they are not in `~/raylib-test/raylib/src`. The test files are empty translation units unless built with their `-D` flag, so a build that compiles every `.c` never sees a second `main()`.

The simulation tests play thousands of metres with bots: a lane-following bot never dies over 12 seeds x 20 km; a ramp bot launches and lands safely; shields absorb a hit; a crashed ball comes to rest. The render tests run the real mesh-building code with raylib's GPU calls stubbed: every triangle faces up, chunks stream correctly for 20 km at 2 rebuilds a frame, nothing handed to the GPU is large, every tree and rock variant fits its footprint and height and differs from the others, the squash spring settles, stays bounded and does not depend on the frame rate, and the ball's matrix keeps the contact point on the ground and the volume constant.

**Not yet verified:** anything that needs a GL context or a touchscreen (the look, the fog, the feel of the drag stick, frame rate on a phone, rotating the device). That is what the first run on the phone is for; the debug overlay says whether both shaders compiled.

## Roadmap

1. **Pickups**: coins (saved total), boost, shields (max 3; a hit slows you instead of ending the run). The shield rules are already in the sim and tested, there is just nothing to pick up yet.
2. Unlocks: spend saved points on ball skins and trails.
3. Smash mechanic: small bushes break and cost a little speed; only boulders and big trees kill.
4. Synthesised sound (reuse the synth from Tower of Babylon), unlocked on the first touch.
5. Speed effects, daily seed with a shareable score.
6. ~~The Play page on the site~~ Done: the Play section of the studio page (click-to-play, same origin so the saved score is not partitioned away).
