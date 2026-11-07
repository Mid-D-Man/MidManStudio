<!-- FILE: src/lib/components/MediaGallery.svelte -->
<script lang="ts">
  import { reveal } from '$lib/actions/reveal';
  import GifMedia from '$lib/components/GifMedia.svelte';
  import { mediaItems, mediaFilters, type MediaFilterId } from '$lib/data/media';

  // Content lives in $lib/data/media.ts — add images, GIFs and clips there.
  let active = $state<MediaFilterId>('all');

  // Only offer a filter that has something to show.
  const filters = mediaFilters.filter((f) => f.id === 'all' || mediaItems.some((i) => i.kind === f.id));

  const visible = $derived(
    active === 'all' ? mediaItems : mediaItems.filter((i) => i.kind === active)
  );
</script>

<section id="showcase" class="section">
  <div class="container">
    <span class="section-label">03 — Showcase</span>
    <h2 class="section-title reveal" use:reveal>
      Screens, clips, <em>and process</em>
    </h2>
    <p class="section-intro reveal" use:reveal>
      Gameplay captures, editor tooling in action, and the DixScript toolchain at work.
      Tap an image to open it full size, or an animation to pause and play it.
    </p>

    <div class="gallery-filters reveal" use:reveal role="group" aria-label="Filter the showcase">
      {#each filters as f (f.id)}
        <button
          class="filter-chip"
          class:active={active === f.id}
          aria-pressed={active === f.id}
          onclick={() => (active = f.id)}
        >
          {f.label}
        </button>
      {/each}
    </div>

    <div class="media-gallery-grid">
      {#each visible as item, i (item.id)}
        <figure class="gallery-item reveal" use:reveal style="transition-delay: {(i % 3) * 80}ms">
          <div class="media-slot gallery-media gallery-media--filled">
            {#if item.type === 'video'}
              <!-- svelte-ignore a11y_media_has_caption -->
              <video
                src={item.src}
                poster={item.poster}
                aria-label={item.alt ?? item.title}
                muted
                loop
                playsinline
                controls
                preload="metadata"
              ></video>
            {:else if item.type === 'gif' && item.poster}
              <GifMedia
                src={item.src}
                poster={item.poster}
                alt={item.alt ?? item.title}
                title={item.title}
                width={item.width}
                height={item.height}
              />
            {:else}
              <a href={item.src} target="_blank" rel="noopener noreferrer" class="gallery-link">
                <img
                  src={item.src}
                  alt={item.alt ?? item.title}
                  width={item.width}
                  height={item.height}
                  loading="lazy"
                  decoding="async"
                />
              </a>
            {/if}
          </div>
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
    display: flex;
    flex-direction: column;
    gap: var(--sp-1);
    margin: 0;
  }

  /* Real media: solid backdrop instead of the striped placeholder, and show the
     whole frame (editor screenshots have UI at every edge, so no cropping). */
  .gallery-media--filled {
    background: var(--void-800);
  }

  .gallery-link {
    display: block;
    width: 100%;
    height: 100%;
  }

  .gallery-media img,
  .gallery-media video {
    display: block;
    object-fit: contain;
  }

  .gallery-caption {
    font-size: var(--fs-sm);
    color: var(--ink-300);
    line-height: 1.4;
  }
</style>
