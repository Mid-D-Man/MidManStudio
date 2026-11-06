<!-- FILE: src/lib/components/GifMedia.svelte -->
<script lang="ts">
  // One animated GIF in the gallery. The page always starts with a tiny still (the poster, about 10 KB); the GIF itself (a
  // few MB) is only fetched once the clip is near the screen, and it is swapped back to the still when it scrolls away, which
  // stops the animation. A visitor who asked for less motion or less data (reduced motion, Data Saver) gets the still and a
  // play button instead. A tap plays or pauses at any time.
  import { onMount } from 'svelte';
  import { inview } from '$lib/actions/inview';

  interface Props {
    src: string;
    poster: string;
    alt: string;
    title: string;
    width?: number;
    height?: number;
  }
  let { src, poster, alt, title, width, height }: Props = $props();

  let onScreen = $state(false);
  let autoplay = $state(false); // decided on the client in onMount; the server render and first paint show the still
  let choice = $state<boolean | null>(null); // null until the visitor taps play or pause

  // Off screen always means the still. On screen follows the visitor's choice, else the autoplay default.
  const playing = $derived(onScreen && (choice ?? autoplay));

  onMount(() => {
    const reduced = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    const conn = (navigator as Navigator & { connection?: { saveData?: boolean } }).connection;
    autoplay = !reduced && !conn?.saveData;
  });

  function toggle() {
    choice = !playing;
  }
</script>

<button
  type="button"
  class="gif"
  class:gif--playing={playing}
  aria-pressed={playing}
  aria-label="{playing ? 'Pause' : 'Play'} animation: {title}"
  onclick={toggle}
  use:inview={(on) => (onScreen = on)}
>
  <img
    src={playing ? src : poster}
    {alt}
    {width}
    {height}
    decoding="async"
    draggable="false"
  />
  <span class="gif__badge" aria-hidden="true">{playing ? 'GIF' : '▶ GIF'}</span>
</button>

<style>
  .gif {
    position: relative;
    display: block;
    width: 100%;
    height: 100%;
    padding: 0;
    border: 0;
    background: none;
    cursor: pointer;
    -webkit-tap-highlight-color: transparent;
  }

  /* More specific than the global `.media-slot img` (cover): editor and browser captures have UI at every edge. */
  .gif img {
    display: block;
    width: 100%;
    height: 100%;
    object-fit: contain;
  }

  .gif:focus-visible {
    outline: 2px solid var(--ink-100, #fff);
    outline-offset: -4px;
  }

  .gif__badge {
    position: absolute;
    left: 0.6rem;
    bottom: 0.6rem;
    font-family: var(--font-mono);
    font-size: var(--fs-xs);
    letter-spacing: 0.06em;
    text-transform: uppercase;
    color: var(--ink-100);
    background: rgba(5, 8, 16, 0.78);
    background: color-mix(in srgb, var(--void-950) 78%, transparent);
    border: 1px solid var(--line-strong);
    border-radius: var(--radius-sm);
    padding: 0.2em 0.55em;
    pointer-events: none;
    transition: opacity var(--dur-fast) var(--ease);
  }

  .gif--playing .gif__badge {
    opacity: 0.55;
  }

  @media (hover: hover) {
    .gif--playing:hover .gif__badge {
      opacity: 1;
    }
  }
</style>
