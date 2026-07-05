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
    id: 'mid-qr',
    number: '03',
    title: 'mid-qr',
    subtitle: 'Full QR Code Ecosystem',
    description:
      'Rust/WASM core with a TypeScript/npm wrapper and a Blazor NuGet package, covering 14 ' +
      'content types — vCard, WhatsApp, Instagram, Telegram, and more.',
    tech: ['Rust', 'WASM', 'Blazor', 'TypeScript'],
    link: 'https://github.com/Mid-D-Man',
    linkLabel: 'GitHub',
    status: 'completed'
  },
  {
    id: 'vantagesolutions',
    number: '04',
    title: 'VantageSolutions',
    subtitle: 'Git-CMS Marketing Site',
    description:
      'SvelteKit site with articles pulled from a JSON file in the repo via raw.githubusercontent.com — ' +
      'no backend, no redeploy needed for content updates thanks to Fastly CDN caching.',
    tech: ['SvelteKit', 'Git-CMS'],
    link: 'https://github.com/Mid-D-Man',
    linkLabel: 'GitHub',
    status: 'completed'
  }
];
