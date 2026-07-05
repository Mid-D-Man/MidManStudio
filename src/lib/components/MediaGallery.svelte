<!-- FILE: src/lib/components/MediaGallery.svelte -->
<script lang="ts">
  import { reveal } from '$lib/actions/reveal';

  // Placeholder gallery items. To swap in real media:
  //   - image: put the file in /static/media/images/ and set `src` to
  //            "/media/images/your-file.jpg", then render <img src={item.src} alt={item.alt} />
  //   - video: put the file in /static/media/videos/ and render
  //            <video src="/media/videos/your-file.mp4" poster={item.src} muted loop playsinline controls />
  // The .media-slot / .media-slot__label styling (base.css) keeps working either way —
  // just replace the placeholder <span> with the real <img>/<video> tag.
  type Kind = 'screenshot' | 'gameplay' | 'concept-art';

  interface MediaItem {
    id: string;
    title: string;
    kind: Kind;
  }

  const items: MediaItem[] = [
    { id: 'm1', title: 'Projectile System — reconciliation demo', kind: 'gameplay' },
    { id: 'm2', title: 'Netcode lobby — LAN discovery', kind: 'gameplay' },
    { id: 'm3', title: 'Hierarchy Arranger in action', kind: 'screenshot' },
    { id: 'm4', title: 'MSX vector render — dual-buffer OIT', kind: 'screenshot' },
    { id: 'm5', title: 'Concept pass — enemy silhouette set', kind: 'concept-art' },
    { id: 'm6', title: 'Auto Reference — fuzzy matcher UI', kind: 'screenshot' }
  ];

  const filters: { id: Kind | 'all'; label: string }[] = [
    { id: 'all', label: 'All' },
    { id: 'gameplay', label: 'Gameplay' },
    { id: 'screenshot', label: 'Screenshots' },
    { id: 'concept-art', label: 'Concept Art' }
  ];

  let active = $state<Kind | 'all'>('all');

  const visible = $derived(active === 'all' ? items : items.filter((i) => i.kind === active));
</script>

<section id="showcase" class="section">
  <div class="container">
    <span class="section-label">03 — Showcase</span>
    <h2 class="section-title reveal" use:reveal>
      Screens, clips, <em>and process</em>
    </h2>
    <p class="section-intro reveal" use:reveal>
      Gameplay captures, editor tooling in action, and concept work. Slots below are
      wired up and ready — drop images into <code>/static/media/images</code> and
      clips into <code>/static/media/videos</code>.
    </p>

    <div class="gallery-filters reveal" use:reveal>
      {#each filters as f (f.id)}
        <button
          class="filter-chip"
          class:active={active === f.id}
          onclick={() => (active = f.id)}
        >
          {f.label}
        </button>
      {/each}
    </div>

    <div class="media-gallery-grid">
      {#each visible as item, i (item.id)}
        <figure class="media-slot gallery-item reveal" use:reveal style="transition-delay: {i * 60}ms">
          <span class="media-slot__label">{item.kind === 'gameplay' ? 'Video' : 'Image'}</span>
          <figcaption class="gallery-caption">{item.title}</figcaption>
        </figure>
      {/each}
    </div>
  </div>
</section>

<style>
  .gallery-filters {
    display: flex;
    flex-wrap: wrap;
    gap: 0.6rem;
    margin-bottom: var(--sp-4);
  }

  .filter-chip {
    font-family: var(--font-mono);
    font-size: var(--fs-xs);
    letter-spacing: 0.04em;
    text-transform: uppercase;
    color: var(--ink-300);
    background: transparent;
    border: 1px solid var(--line-strong);
    border-radius: 999px;
    padding: 0.5em 1.1em;
    cursor: pointer;
    transition: color var(--dur-fast) var(--ease), border-color var(--dur-fast) var(--ease);
  }

  .filter-chip:hover {
    color: var(--ink-100);
  }

  .filter-chip.active {
    color: var(--void-950);
    background: var(--grad-accent);
    border-color: transparent;
  }

  .gallery-item {
    flex-direction: column;
    gap: var(--sp-2);
    margin: 0;
  }

  .gallery-caption {
    position: absolute;
    left: var(--sp-2);
    bottom: var(--sp-2);
    right: var(--sp-2);
    font-size: var(--fs-sm);
    color: var(--ink-100);
    background: rgba(4, 6, 12, 0.7);
    padding: 0.4em 0.7em;
    border-radius: 8px;
    z-index: 1;
  }
</style>
