// FILE: src/lib/data/packages.ts
// Unity packages under the com.midmanstudio scope.
// Source: MidManStudio_Unity monorepo (packages/), plus MDIX Studio (com.midmanstudio.mdix),
// which lives in the DixScript-Rust repo — its repoPath is prefixed with that repo's name.
//
// `media` is optional: only a card that has a media field gets a screenshot / GIF slot.
// Leave it off for packages with nothing visual to show. To fill the slot later, add `src`
// (a file under static/media/images/, e.g. '/media/images/packages/projectile.gif').

export interface PackageMedia {
  /** Image or GIF path under static/media/. Omit to show a "coming soon" slot. */
  src?: string;
  alt?: string;
  /** Placeholder text shown while there is no `src`. */
  label?: string;
}

export interface UnityPackage {
  id: string;
  name: string;
  scope: string;
  tagline: string;
  description: string;
  highlights: string[];
  tech: string[];
  status: 'shipping' | 'active-dev';
  repoPath: string;
  media?: PackageMedia;
}

export const unityPackages: UnityPackage[] = [
  {
    id: 'mdix-studio',
    name: 'MDIX Studio',
    scope: 'com.midmanstudio.mdix',
    tagline: 'More powerful than PlayerPrefs, lighter than SQLite',
    description:
      'The DixScript runtime for Unity, with an in-editor studio. Load typed, encrypted .mdix ' +
      'data straight into your classes, and edit it in MDIX Studio — Explorer, Editor, Search, ' +
      'and Templates tabs with live language-server diagnostics.',
    highlights: [
      '.mdix files import as Unity assets — open them in MDIX Studio or generate ScriptableObjects',
      'LoadAs<T> fills typed classes, lists, and dictionaries from a .mdix file',
      'Built-in encryption and LINQ-style queries'
    ],
    tech: ['Unity', 'C#', 'DixScript', 'Editor Tooling'],
    status: 'shipping',
    repoPath: 'DixScript-Rust/com.midmanstudio.mdix',
    media: {
      src: '/media/images/mdix-studio/preview-1.png',
      alt: 'MDIX Studio Explorer tab showing typed values, an items array as a table and a nested object.'
    }
  },
  {
    id: 'utilities',
    name: 'Utilities',
    scope: 'com.midmanstudio.utilities',
    tagline: 'The foundation every project is built on',
    description:
      'Core utilities used across every MidManStudio Unity project — tick dispatcher, logger, ' +
      'timers, object and particle pools, singletons, an event bus, scene management, and UI state. ' +
      'Includes an Auto Reference system with fuzzy name matching and a Hierarchy Arranger ' +
      'with ten arrangement modes.',
    highlights: [
      'Auto Reference — fuzzy name matching wires up component references automatically',
      'Hierarchy Arranger — 10 arrangement modes for keeping scenes clean',
      'Zero-allocation tick dispatcher, timers, and a typed event bus'
    ],
    tech: ['Unity', 'C#', 'Editor Tooling'],
    status: 'shipping',
    repoPath: 'packages/com.midmanstudio.utilities'
  },
  {
    id: 'audio',
    name: 'Audio',
    scope: 'com.midmanstudio.audio',
    tagline: 'Music, SFX, and a limiter for the final mix',
    description:
      'Audio for Unity — music with crossfade and pitch glide, SFX played by string id or ' +
      'generated enum, audio categories with mixer groups, pooled voices and spatial audio ' +
      'objects, and a peak limiter on the final mix.',
    highlights: [
      'SFX by generated enum — an editor generator keeps the ids stable',
      'Pooled voices that play clips without allocating',
      'Peak limiter — Rust DSP on desktop and mobile, C# fallback on WebGL'
    ],
    tech: ['Unity', 'C#', 'Rust DSP'],
    status: 'active-dev',
    repoPath: 'packages/com.midmanstudio.audio'
  },
  {
    id: 'netcode',
    name: 'Netcode',
    scope: 'com.midmanstudio.netcode',
    tagline: 'NGO utilities for actually shipping multiplayer',
    description:
      'Netcode for GameObjects utilities: network singletons, object pooling, connection ' +
      'management, and LAN/WiFi lobby discovery. Built from real debugging sessions on live ' +
      'multiplayer projects, not written in the abstract.',
    highlights: [
      'Local lobby manager with client/host discovery over LAN',
      'Network-safe object pooling patterns for NGO',
      'Connection lifecycle management that survives reconnects'
    ],
    tech: ['Unity', 'Netcode for GameObjects', 'C#'],
    status: 'shipping',
    repoPath: 'packages/com.midmanstudio.netcode'
  },
  {
    id: 'patterns',
    name: 'Patterns',
    scope: 'com.midmanstudio.patterns',
    tagline: 'Design patterns as small, engine-independent building blocks',
    description:
      'Generic design-pattern building blocks for Unity — command history with undo/redo, ' +
      'state machines, event queues, dependency injection, broker chains with stat modifiers, ' +
      'spatial partitioning, a fixed-step game loop, and more. The runtime has no engine ' +
      'references, so it also runs and tests outside Unity.',
    highlights: [
      'Command — undo/redo history with grouping, merging, and composite commands',
      'Broker Chain — priority-ordered handlers with a stat sheet for buffs and modifiers',
      'No dependencies; the runtime assembly never references the engine'
    ],
    tech: ['Unity', 'C#', 'Design Patterns'],
    status: 'active-dev',
    repoPath: 'packages/com.midmanstudio.patterns'
  },
  {
    id: 'projectilesystem',
    name: 'Projectile System',
    scope: 'com.midmanstudio.projectilesystem',
    tagline: 'Server-authoritative, Rust-simulated, 2D-first',
    description:
      'High-performance server-authoritative projectile system with a Rust simulation core. ' +
      'Firing client handles the visual locally, the server runs the authoritative simulation, ' +
      'and every other client receives state over RPC — with reconciliation and snapshot-based ' +
      'velocity extrapolation for non-linear movement types.',
    highlights: [
      'Rust core (projectile_core) — batched simulation, called from Unity over FFI',
      'Primarily 2D, with a 3D path kept as a reference implementation',
      'Raycast, Rust-simulated, and physics-based projectiles with configurable firing patterns'
    ],
    tech: ['Unity', 'Rust', 'NGO'],
    status: 'active-dev',
    repoPath: 'packages/com.midmanstudio.projectilesystem',
    media: { label: 'GIF · coming soon' }
  }
];
