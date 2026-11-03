# MidManStudio

The studio site for **MidManStudio** — Unity packages, the Rust-powered tech underneath them, and the projects built on top. Live at [midmanstudio.pages.dev](https://midmanstudio.pages.dev).

SvelteKit 2 + Svelte 5 (TypeScript), deployed to Cloudflare Pages.

## Develop

```bash
npm install
npm run dev       # local dev server
npm run check     # svelte-check + type check
npm run build     # production build -> .svelte-kit/cloudflare
npm run preview   # serve the production build locally
```

## Where the content lives

Most updates are data edits — no component changes needed.

| File | What it drives |
| --- | --- |
| `src/lib/data/packages.ts` | Unity package cards (utilities, audio, netcode, patterns, projectile system). Only cards with a `media` field get a screenshot / GIF slot |
| `src/lib/data/ecosystem.ts` | "Core Tech" cards (DixScript, mid-engine / mid-math, MSX, projectile_core) |
| `src/lib/data/projects.ts` | Shipped projects |
| `src/lib/data/media.ts` | Showcase gallery (images, GIFs, clips) |

SIMD is deliberately attributed to **mid-math only** (the `mid-engine` entry in `ecosystem.ts`) — keep it out of the other cards.

## Adding showcase media

Everything is declared in `src/lib/data/media.ts`; files go under `static/media/`.

- **Image or GIF** — drop the file in `static/media/images/<folder>/`, then add an entry with `src: '/media/images/<folder>/<file>'`. A `.gif` is handled exactly like a `.png`. Set `width`/`height` (px) and a descriptive `alt`.
- **Video** — drop the file in `static/media/videos/`, then add an entry with `type: 'video'`, `src: '/media/videos/<file>.mp4'` and an optional `poster`.
- An entry with **no `src`** renders as a "coming soon" slot — fill it in later by adding the `src`.

Current folders: `game/`, `mdix-studio/` (the Unity editor tool), `mdix-lsp/` (DixScript in VS Code).

## Deploy

Pushing to `main` runs `.github/workflows/deploy.yml`, which builds the site and publishes it to the Cloudflare Pages project `midmanstudio`. The workflow needs the repository secrets `CLOUDFLARE_API_TOKEN` and `CLOUDFLARE_ACCOUNT_ID`.

## Scaffold / replacements

This repo uses [mdix-scaffold](https://github.com/Mid-D-Man/mdix-scaffold). Batches of changed files are applied by committing a `.tar.gz` next to `.mdix/replacements/replacements.mdix` and running the **Apply replacements** workflow (`.github/workflows/run-replacements.yml`).

## License

Proprietary — all rights reserved. See [LICENSE](LICENSE).
