// FILE: src/lib/data/packages.ts
// Unity packages published under the com.midmanstudio scope.
// Source: MidManStudio_Unity monorepo (packages/).

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
}

export const unityPackages: UnityPackage[] = [
  {
    id: 'utilities',
    name: 'Utilities',
    scope: 'com.midmanstudio.utilities',
    tagline: 'The foundation every project is built on',
    description:
      'Core utilities used across every MidManStudio Unity project — tick dispatcher, ' +
      'logger, timers, object pools, audio limiting, FX, scene management, and UI state. ' +
      'Includes an Auto Reference system with fuzzy name matching and a Hierarchy Arranger ' +
      'with ten arrangement modes.',
    highlights: [
      'Auto Reference — fuzzy matcher (Levenshtein + token Jaccard) wires up component refs automatically',
      'Hierarchy Arranger — 10 arrangement modes for keeping scenes clean',
      'Tick dispatcher, pooled timers, and a native audio limiter backed by Rust'
    ],
    tech: ['Unity', 'C#', 'Rust FFI', 'Editor Tooling'],
    status: 'shipping',
    repoPath: 'packages/com.midmanstudio.utilities'
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
    id: 'projectilesystem',
    name: 'Projectile System',
    scope: 'com.midmanstudio.projectilesystem',
    tagline: 'Server-authoritative, SIMD-simulated, 2D-first',
    description:
      'High-performance server-authoritative projectile system with a Rust simulation core. ' +
      'Firing client handles the visual locally, the server runs the authoritative simulation, ' +
      'and every other client receives state over RPC — with reconciliation and snapshot-based ' +
      'velocity extrapolation for non-linear movement types.',
    highlights: [
      'Rust core (projectile_core) — SSE2 SIMD, 4-wide batched simulation',
      'Primarily 2D, with a 3D path kept as a reference implementation',
      'RequireOwnership = false bridge for physics projectiles, raycast sender-exclusion for no double-fires'
    ],
    tech: ['Unity', 'Rust', 'NGO', 'SIMD'],
    status: 'active-dev',
    repoPath: 'packages/com.midmanstudio.projectilesystem'
  }
];
