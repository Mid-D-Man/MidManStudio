// FILE: src/lib/data/projects.ts
// Work beyond the game-dev core — web, tooling, research. Secondary section
// ("Beyond Games") so the gamedev material above stays the main event.

export interface Project {
  id: string;
  number: string;
  title: string;
  subtitle: string;
  description: string;
  tech: string[];
  link: string;
  linkLabel: string;
  status: 'featured' | 'in-progress' | 'completed';
}

export const projects: Project[] = [
  {
    id: 'aircode',
    number: '01',
    title: 'AirCode',
    subtitle: 'QR-Based Attendance Platform',
    description:
      'Higher-institution attendance tracking via QR codes. Real-time dashboards for ' +
      'lecturers and admins, tamper-resistant check-ins.',
    tech: ['C#', 'Blazor', 'Firebase', 'QR Code'],
    link: 'https://mid-d-man.github.io/AirCode/',
    linkLabel: 'Live Demo',
    status: 'featured'
  },
  {
    id: 'mbfa',
    number: '02',
    title: 'MBFA',
    subtitle: "MidMan's Bit Folding Algorithm",
    description:
      'A multi-fold iterative compression algorithm under active research — each fold sees ' +
      'a structurally different layer of the data than the last. Already beating gzip on ' +
      'highly repetitive data; entropy coding in progress.',
    tech: ['Rust', 'Compression', 'Research'],
    link: 'https://github.com/Mid-D-Man/mbfa',
    linkLabel: 'GitHub',
    status: 'in-progress'
  },
  {
    id: 'mpx',
    number: '03',
    title: 'MPX',
    subtitle: 'Lossless Image Format',
    description:
      'A lossless image codec built directly on top of MBFA\'s fold-compression — the same ' +
      'research applied to raster images instead of general-purpose data.',
    tech: ['Rust', 'Image Format', 'Compression'],
    link: 'https://github.com/Mid-D-Man/mpx',
    linkLabel: 'GitHub',
    status: 'completed'
  },
  {
    id: 'mdix-scaffold',
    number: '04',
    title: 'mdix-scaffold',
    subtitle: 'Declarative Project Scaffolding',
    description:
      'Idempotent project-structure generator driven entirely by DixScript templates — define ' +
      'a layout once, run it as a GitHub Action or a local CLI command. Only new entries get ' +
      'written; existing files are never touched by default.',
    tech: ['Rust', 'DixScript', 'GitHub Actions'],
    link: 'https://github.com/Mid-D-Man/mdix-scaffold',
    linkLabel: 'GitHub',
    status: 'completed'
  },
  {
    id: 'mid-qr',
    number: '05',
    title: 'mid-qr',
    subtitle: 'Full QR Code Ecosystem',
    description:
      'Rust/WASM core with a TypeScript/npm wrapper and a Blazor NuGet package, covering 14 ' +
      'content types — vCard, WhatsApp, Instagram, Telegram, and more.',
    tech: ['Rust', 'WASM', 'Blazor', 'TypeScript'],
    link: 'https://github.com/MidManStudio/mid_qr_code_lib',
    linkLabel: 'GitHub',
    status: 'completed'
  },
  {
    id: 'midman-pulse',
    number: '06',
    title: 'MidMan Pulse',
    subtitle: 'Automated Editorial Feed',
    description:
      'AI-driven content pipeline — scrapes sources across a wide spread of categories, then ' +
      'generates a daily feed and personalized posts without a human touching each one.',
    tech: ['SvelteKit', 'Cloudflare', 'AI'],
    link: 'PLACEHOLDER_LIVE_URL', // TODO(Mid): confirm the live pages.dev / custom domain
    linkLabel: 'Visit Site',
    status: 'completed'
  },
  {
    id: 'vantagesolutions',
    number: '07',
    title: 'VantageSolutions',
    subtitle: 'Git-CMS Marketing Site',
    description:
      'SvelteKit site with articles pulled from a JSON file in the repo via raw.githubusercontent.com — ' +
      'no backend, no redeploy needed for content updates thanks to Fastly CDN caching.',
    tech: ['SvelteKit', 'Git-CMS'],
    link: 'PLACEHOLDER_LIVE_URL', // TODO(Mid): confirm the live pages.dev / custom domain
    linkLabel: 'Visit Site',
    status: 'completed'
  }
];
