// FILE: src/lib/data/media.ts
// Showcase gallery content (rendered by MediaGallery.svelte).
//
// Adding media:
//   image / GIF -> put the file in static/media/images/ and add an entry with
//                  src: '/media/images/<folder>/<file>' (a .gif works the same as a .png)
//                  — for a slot that is still empty, set type: 'gif' so it is labelled "GIF"
//   video       -> put the file in static/media/videos/ and add an entry with
//                  type: 'video', src: '/media/videos/<file>.mp4', optional poster
// An entry with no `src` renders as a labelled "coming soon" slot.

export type MediaKind = 'gameplay' | 'mdix-studio' | 'mdix-lsp' | 'screenshot' | 'concept-art';
export type MediaFilterId = MediaKind | 'all';
export type MediaType = 'image' | 'gif' | 'video';

export interface MediaItem {
  id: string;
  title: string;
  kind: MediaKind;
  /** Defaults to 'image'. 'gif' renders like an image; it only changes the placeholder label. */
  type?: MediaType;
  src?: string;
  /** Still frame shown before a video plays. */
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

  // ---- Coming soon (GIFs, clips and further captures) ----
  { id: 'projectile-gif', title: 'Projectile System — in action', kind: 'gameplay', type: 'gif' },
  { id: 'studio-gif', title: 'MDIX Studio — in action', kind: 'mdix-studio', type: 'gif' },
  { id: 'm1', title: 'Projectile System — reconciliation demo', kind: 'gameplay', type: 'video' },
  { id: 'm2', title: 'Netcode lobby — LAN discovery', kind: 'gameplay', type: 'video' },
  { id: 'm3', title: 'Hierarchy Arranger in action', kind: 'screenshot' },
  { id: 'm4', title: 'MSX vector render — dual-buffer OIT', kind: 'screenshot' },
  { id: 'm5', title: 'Concept pass — enemy silhouette set', kind: 'concept-art' },
  { id: 'm6', title: 'Auto Reference — fuzzy matcher UI', kind: 'screenshot' }
];
