// FILE: src/lib/data/ecosystem.ts
// The Rust-powered tech running underneath the Unity packages and tools —
// the "how it actually works" layer.

export interface EcosystemItem {
  id: string;
  name: string;
  tagline: string;
  description: string;
  tags: string[];
  link: string;
  linkLabel: string;
  accent: 'royal' | 'cyan' | 'crimson' | 'gold';
}

export const ecosystemItems: EcosystemItem[] = [
  {
    id: 'dixscript',
    name: 'DixScript',
    tagline: 'Config · code · encryption, one format',
    description:
      'A data interchange format (.mdix) combining configuration, compile-time functions, ' +
      'built-in AES-256 encryption, and automatic compression. Core Rust runtime published to ' +
      'crates.io, with published bindings for Node/WASM, Python, and C#, plus a CLI and LSP. ' +
      'Go, Java, Lua, PHP, and Odin bindings are code-complete and shipping soon. Also ships ' +
      'first-class Unity integration — asset import, localization, and WebGL support — straight ' +
      'from the same repo.',
    tags: ['Rust', 'Language Design', 'LSP', 'Multi-language Bindings', 'Published'],
    link: 'https://github.com/Mid-D-Man/DixScript-Rust',
    linkLabel: 'GitHub',
    accent: 'royal'
  },
  {
    id: 'mid-math',
    name: 'mid-math',
    tagline: 'SIMD math across every architecture',
    description:
      'Part of a larger Rust game-tech ecosystem — a math library with vectors, matrices, ' +
      'quaternions, curves, noise, and color. SIMD-accelerated on SSE2, AVX2, AVX-512, NEON, and ' +
      'WASM SIMD128 with a scalar fallback, so the same code runs on a phone or a server.',
    tags: ['Rust', 'SIMD', 'NEON', 'AVX-512', 'WASM'],
    link: 'https://github.com/Mid-D-Man/mid-engine',
    linkLabel: 'GitHub',
    accent: 'cyan'
  },
  {
    id: 'msx',
    name: 'MSX',
    tagline: 'MidStroke eXtension — vector graphics engine',
    description:
      'A vector graphics rendering format with a custom binary format and DixScript source. ' +
      'Under active development, with the core rendering pipeline already working.',
    tags: ['Rust', 'Rendering', 'Vector Graphics', 'Binary Format'],
    link: 'https://github.com/Mid-D-Man/msx',
    linkLabel: 'GitHub',
    accent: 'crimson'
  },
  {
    id: 'projectile-core',
    name: 'projectile_core',
    tagline: 'The simulation running the Unity package above',
    description:
      'Native Rust simulation core backing com.midmanstudio.projectilesystem — batched ' +
      'simulation, compiled per-platform and pulled into Unity through FFI.',
    tags: ['Rust', 'FFI', 'Game Simulation'],
    link: 'https://github.com/MidManStudio/MidManStudio_Unity',
    linkLabel: 'GitHub',
    accent: 'gold'
  }
];
