// FILE: src/lib/data/media.ts
// Showcase gallery content (rendered by MediaGallery.svelte).
//
// Adding media:
//   image       -> put the file in static/media/images/ and add an entry with
//                  src: '/media/images/<folder>/<file>'
//   GIF         -> put the file in static/media/gifs/ and a still frame (WebP) of it in static/media/gifs/posters/ with the same
//                  name, then add gif('<id>', '<file name without .gif>', ...) below. The page loads the still first and only
//                  plays the GIF while it is on screen (GifMedia.svelte). A GIF entry without a poster renders as a plain image.
//   video       -> put the file in static/media/videos/ and add an entry with
//                  type: 'video', src: '/media/videos/<file>.mp4', optional poster
// Every entry needs a real `src`: the gallery shows no empty placeholder cards.

export type MediaKind = 'gameplay' | 'mdix-studio' | 'mdix-lsp' | 'screenshot' | 'concept-art';
export type MediaFilterId = MediaKind | 'all';
export type MediaType = 'image' | 'gif' | 'video';

export interface MediaItem {
  id: string;
  title: string;
  kind: MediaKind;
  /** Defaults to 'image'. 'gif' with a poster plays only while on screen; without a poster it renders like an image. */
  type?: MediaType;
  src: string;
  /** Still frame: shown before a video plays, and in place of a GIF whenever it is not playing. */
  poster?: string;
  alt?: string;
  /** Intrinsic size in px — lets the browser reserve space before the image loads. */
  width?: number;
  height?: number;
}

export const mediaFilters: { id: MediaFilterId; label: string }[] = [
  { id: 'all', label: 'All' },
  { id: 'gameplay', label: 'Gameplay' },
  { id: 'mdix-studio', label: 'MDIX Studio' },
  { id: 'mdix-lsp', label: 'DixScript LSP' },
  { id: 'screenshot', label: 'Screenshots' },
  { id: 'concept-art', label: 'Concept Art' }
];

const IMG = '/media/images';
const GIF = '/media/gifs';

/** An animated GIF with its poster still; `file` is the name without the extension, the same in gifs/ and gifs/posters/. */
function gif(
  id: string,
  file: string,
  title: string,
  kind: MediaKind,
  alt: string,
  width: number,
  height: number
): MediaItem {
  return { id, title, kind, type: 'gif', src: `${GIF}/${file}.gif`, poster: `${GIF}/posters/${file}.webp`, alt, width, height };
}

export const mediaItems: MediaItem[] = [
  // ---- Gameplay ----
  {
    id: 'game-map',
    title: 'Projectile System test scene — arena overview',
    kind: 'gameplay',
    src: `${IMG}/game/map-preview.png`,
    alt: 'Top-down debug view of a walled arena with colour-coded floor and wall surfaces and several cylindrical obstacles.',
    width: 586,
    height: 365
  },
  {
    id: 'game-weapon',
    title: 'Projectile System test scene — weapon view',
    kind: 'gameplay',
    src: `${IMG}/game/stuff.jpg`,
    alt: 'First-person view of a shotgun in a corridor with yellow checkered walls, an FPS counter in the corner and an on-screen fire button.',
    width: 1280,
    height: 720
  },
  {
    id: 'game-wide',
    title: 'Projectile System test scene — in-game view',
    kind: 'gameplay',
    src: `${IMG}/game/basic.jpg`,
    alt: 'In-game view with a health bar and green orb in the HUD, a blue capsule character in the distance and a purple textured block on a yellow checkered floor.',
    width: 719,
    height: 376
  },

  gif(
    '3d-1', '3d_projectile_showcase_1', 'Projectile System — 3D, first person (1 of 4)', 'gameplay',
    'Looping first-person capture: a shotgun held in a checkered arena, with a green orb floating above a grey block on a yellow and purple floor.',
    422, 238
  ),
  gif(
    '3d-2', '3d_projectile_showcase_2', 'Projectile System — 3D, first person (2 of 4)', 'gameplay',
    'Looping first-person capture: an assault rifle held in front of a smooth grey slope, with the FPS counter in the corner.',
    422, 238
  ),
  gif(
    '3d-3', '3d_projectile_showcase_3', 'Projectile System — 3D, first person (3 of 4)', 'gameplay',
    'Looping first-person capture: a shotgun in a corridor of yellow checkered walls over a purple grid floor, with green targets in the distance.',
    422, 238
  ),
  gif(
    '3d-4', '3d_projectile_showcase_4', 'Projectile System — 3D, first person (4 of 4)', 'gameplay',
    'Looping first-person capture: a shotgun on a purple grid floor with a large green sphere floating ahead and green projectiles in the air.',
    422, 238
  ),
  gif(
    '2d-1', '2d_projectile_showcase_1', 'Projectile System — 2D, side view (1 of 4)', 'gameplay',
    'Looping side-view capture: a blue capsule character above a yellow checkered platform, with a purple hexagon, a grey cylinder and a red cone at the edges.',
    422, 238
  ),
  gif(
    '2d-2', '2d_projectile_showcase_2', 'Projectile System — 2D, side view (2 of 4)', 'gameplay',
    'Looping side-view capture: a green burst effect above the platform and a row of green orbs along its front edge.',
    422, 238
  ),
  gif(
    '2d-3', '2d_projectile_showcase_3', 'Projectile System — 2D, side view (3 of 4)', 'gameplay',
    'Looping side-view capture: a green starburst of particles at the top, a red triangle, and a row of green orbs on the platform.',
    422, 238
  ),
  gif(
    '2d-4', '2d_projectile_showcase_4', 'Projectile System — 2D, side view (4 of 4)', 'gameplay',
    'Looping side-view capture: a large lilac block tumbling across the yellow checkered platform.',
    422, 238
  ),
  gif(
    'lobby', 'Offline_Lobby_showcase', 'Netcode lobby — LAN discovery', 'gameplay',
    'Looping capture of the Unity editor with the PackageSandbox game view open: a yellow checkered scene with Host and Find buttons along the bottom.',
    422, 264
  ),

  // ---- MDIX Studio (Unity editor tool) ----
  {
    id: 'studio-1',
    title: 'MDIX Studio — Explorer',
    kind: 'mdix-studio',
    src: `${IMG}/mdix-studio/preview-1.png`,
    alt: 'MDIX Studio Explorer tab showing typed values, an items array as a table and a nested object, with Copy, Fold All, Save and Compile buttons.',
    width: 1280,
    height: 800
  },
  {
    id: 'studio-2',
    title: 'Unity inspector — Open in MDIX Studio',
    kind: 'mdix-studio',
    src: `${IMG}/mdix-studio/preview-2.png`,
    alt: 'Unity inspector for a secrets.mdix asset, with the MDIX importer, an Open in MDIX Studio button and a Generate ScriptableObject button.',
    width: 1280,
    height: 800
  },
  {
    id: 'studio-3',
    title: 'MDIX Studio — inline editing',
    kind: 'mdix-studio',
    src: `${IMG}/mdix-studio/preview-3.png`,
    alt: 'MDIX Studio Explorer with the false value of a nested boolean selected for inline editing.',
    width: 1280,
    height: 800
  },
  {
    id: 'studio-4',
    title: 'MDIX Studio — file templates',
    kind: 'mdix-studio',
    src: `${IMG}/mdix-studio/preview-4.png`,
    alt: 'MDIX Studio Templates tab offering Blank, Game Enemies, Inventory Items, App Config, Server Config, Encrypted Secrets and Player Save templates.',
    width: 1280,
    height: 800
  },
  {
    id: 'studio-5',
    title: 'MDIX Studio — Editor with LSP diagnostics',
    kind: 'mdix-studio',
    src: `${IMG}/mdix-studio/preview-5.png`,
    alt: 'MDIX Studio Editor tab with syntax-highlighted DixScript and a diagnostics line reading mdix-lsp: no problems.',
    width: 1280,
    height: 800
  },

  // ---- DixScript LSP in VS Code ----
  {
    id: 'lsp-1',
    title: 'VS Code — syntax highlighting & CodeLens',
    kind: 'mdix-lsp',
    src: `${IMG}/mdix-lsp/preview-1.png`,
    alt: 'VS Code showing a DixScript file with syntax highlighting and JSON, TOML, Minify, Resolve and Compile CodeLens actions above the code.',
    width: 1280,
    height: 800
  },
  {
    id: 'lsp-2',
    title: 'VS Code — block folding',
    kind: 'mdix-lsp',
    src: `${IMG}/mdix-lsp/preview-2.png`,
    alt: 'The same DixScript file in VS Code with code-folding arrows beside the QuickFuncs block.',
    width: 1280,
    height: 800
  },
  {
    id: 'lsp-3',
    title: 'Large-file test — 27k-line JSON output',
    kind: 'mdix-lsp',
    src: `${IMG}/mdix-lsp/preview-3.png`,
    alt: 'The final lines of a 27,358-line elements_database.json open in VS Code, listing physical constants.',
    width: 1280,
    height: 800
  },
  {
    id: 'lsp-4',
    title: 'Large-file test — 8.5k-line .mdix source',
    kind: 'mdix-lsp',
    src: `${IMG}/mdix-lsp/preview-4.png`,
    alt: 'The final lines of an 8,530-line elements_database.mdix file in VS Code, with builder and physics function calls per element.',
    width: 1280,
    height: 800
  },
  {
    id: 'lsp-5',
    title: 'JSON output — head of the file',
    kind: 'mdix-lsp',
    src: `${IMG}/mdix-lsp/preview-5.png`,
    alt: 'The top of elements_database.json showing physical constants followed by an elements object that starts with actinium.',
    width: 1280,
    height: 800
  },
  {
    id: 'lsp-6',
    title: '.mdix source — @CONFIG, @IMPORTS, typed @DATA',
    kind: 'mdix-lsp',
    src: `${IMG}/mdix-lsp/preview-6.png`,
    alt: 'The top of elements_database.mdix showing @CONFIG, @IMPORTS and typed @DATA entries for physical constants.',
    width: 1280,
    height: 800
  },
  {
    id: 'lsp-7',
    title: 'Inline type hints, regex tester & blob preview',
    kind: 'mdix-lsp',
    src: `${IMG}/mdix-lsp/preview-7.png`,
    alt: 'A DixScript file with every data type, showing inline type hints and CodeLens actions such as Test Regex and Preview blob.',
    width: 1280,
    height: 800
  },
  {
    id: 'lsp-8',
    title: 'Enums in DixScript',
    kind: 'mdix-lsp',
    src: `${IMG}/mdix-lsp/preview-8.png`,
    alt: 'A DixScript file with an @ENUMS block and enum-typed values in @DATA.',
    width: 1280,
    height: 800
  },
  {
    id: 'lsp-9',
    title: 'Hover info on literals',
    kind: 'mdix-lsp',
    src: `${IMG}/mdix-lsp/preview-9.png`,
    alt: 'A basic DixScript file with a hover tooltip showing a string literal and its length.',
    width: 1280,
    height: 800
  },

  // ---- MSX (vector graphics format, web tools) ----
  gif(
    'msx-1', 'Msx_showcase_1', 'MSX — Samples gallery', 'screenshot',
    'Looping capture of the MSX samples page scrolling through a grid of rendered example scenes, including a layer swap, a z-index swap and a QR code.',
    422, 264
  ),
  gif(
    'msx-2', 'Msx_showcase_2', 'MSX — Playground', 'screenshot',
    'Capture of the MSX playground: DixScript source on the left and a live preview of a blue circle on the right.',
    422, 264
  ),
  gif(
    'msx-3', 'Msx_showcase_3', 'MSX — SVG to MSX converter', 'screenshot',
    'Looping capture of the MSX converter: SVG source on the left, the converted MSX source on the right, and the original and converted animations of a blue square and two circles below, with a playback bar.',
    422, 264
  )
];
