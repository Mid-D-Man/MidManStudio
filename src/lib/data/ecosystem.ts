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
      'built-in AES-256 encryption, and automatic compression. Rust runtime with full ' +
      'multi-language bindings — C, C++, Java, Lua, Python, WASM/npm, and Odin — plus a ' +
      'VS Code and IntelliJ LSP.',
    tags: ['Rust', 'Language Design', 'LSP', 'Multi-language Bindings'],
    link: 'https://github.com/Mid-D-Man/DixScript-Rust',
    linkLabel: 'GitHub',
    accent: 'royal'
  },
  {
    id: 'mid-engine',
    name: 'mid-engine / mid-math',
    tagline: 'SIMD math across every architecture',
    description:
      'A Rust game engine workspace built from the ground up. mid-math targets SSE2, AVX2, ' +
      'AVX-512, NEON, and WASM SIMD128 with scalar fallback — the same math running whether ' +
      "it's compiled for a phone or a server. mid-common ports parts of Blender's blenlib string subsystem.",
    tags: ['Rust', 'SIMD', 'NEON', 'AVX-512', 'WASM'],
    link: 'https://github.com/Mid-D-Man',
    linkLabel: 'GitHub',
    accent: 'cyan'
  },
  {
    id: 'msx',
    name: 'MSX',
    tagline: 'MidStroke eXchange — vector graphics engine',
    description:
      'A full vector graphics rendering format with a custom binary format and DixScript ' +
      'source, applying Grease Pencil-derived rendering patterns: dual-buffer OIT compositing, ' +
      'ping-pong FBOs, SMAA, and Halton-sequence supersampling.',
    tags: ['Rust', 'Rendering', 'Vector Graphics', 'Binary Format'],
    link: 'https://github.com/Mid-D-Man',
    linkLabel: 'GitHub',
    accent: 'crimson'
  },
  {
    id: 'projectile-core',
    name: 'projectile_core',
    tagline: 'The simulation running the Unity package above',
    description:
      'Native Rust simulation core backing com.midmanstudio.projectilesystem — SSE2 SIMD, ' +
      'four-wide batch simulation, compiled per-platform and pulled into Unity through FFI.',
    tags: ['Rust', 'SIMD', 'FFI', 'Game Simulation'],
    link: 'https://github.com/MidManStudio/MidManStudio_Unity',
    linkLabel: 'GitHub',
    accent: 'gold'
  }
];
