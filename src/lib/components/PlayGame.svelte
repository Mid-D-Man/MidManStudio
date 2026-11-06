<!-- FILE: src/lib/components/PlayGame.svelte -->
<script lang="ts">
  import { onMount } from 'svelte';
  import { reveal } from '$lib/actions/reveal';

  // The game is built from game/death-roll/ into static/play/death-roll/ (see build-web.sh)
  // and served from the same origin as this page, so its saved best distance is not
  // partitioned away by the browser.
  const GAME_URL = '/play/death-roll/';

  type Phase = 'checking' | 'ready' | 'missing' | 'playing';

  let phase = $state<Phase>('checking');
  let stage = $state<HTMLDivElement | null>(null);
  let frame = $state<HTMLIFrameElement | null>(null);
  let canFullscreen = $state(false);

  const controls = [
    { device: 'Touch / mouse', action: 'Drag left or right to steer' },
    { device: 'Keyboard', action: 'A / D or the arrow keys' },
    { device: 'Retry', action: 'Tap, click, or press space' }
  ];

  onMount(() => {
    canFullscreen = document.fullscreenEnabled === true;

    // Only a definite 404 means "not published yet"; anything else lets the player try.
    const ctrl = new AbortController();
    fetch(GAME_URL, { method: 'HEAD', cache: 'no-store', signal: ctrl.signal })
      .then((res) => {
        phase = res.status === 404 ? 'missing' : 'ready';
      })
      .catch(() => {
        if (!ctrl.signal.aborted) phase = 'ready';
      });

    return () => ctrl.abort();
  });

  function play() {
    if (phase === 'ready') phase = 'playing';
  }

  function stop() {
    if (document.fullscreenElement) void document.exitFullscreen();
    phase = 'ready';
  }

  function goFullscreen() {
    void stage?.requestFullscreen?.();
  }

  // Give the game keyboard focus as soon as it has loaded (same origin, so this is allowed).
  function focusGame() {
    frame?.contentWindow?.focus();
  }
</script>

<section id="play" class="section section--alt">
  <div class="container">
    <span class="section-label">04 — Play</span>
    <h2 class="section-title reveal" use:reveal>
      Try <em>Death Roll</em>
    </h2>
    <p class="section-intro reveal" use:reveal>
      A one-thumb downhill game written in C with raylib and compiled to WebAssembly, so it
      runs right here in your browser. You are a big ball with the studio logo on it: ride the
      hills, launch off ramps, and don't touch a tree. One hit and the run is over.
    </p>

    <div class="play-grid reveal" use:reveal>
      <div class="play-main">
        <div class="stage" bind:this={stage} role="region" aria-label="Death Roll game">
          {#if phase === 'playing'}
            <iframe
              bind:this={frame}
              class="game-frame"
              src={GAME_URL}
              title="Death Roll"
              allow="fullscreen; autoplay; gamepad"
              allowfullscreen
              onload={focusGame}
            ></iframe>
          {:else}
            <div class="poster">
              <span class="poster-slope" aria-hidden="true"></span>
              <img class="poster-ball" src="/brand/logo-mark.png" alt="" width="84" height="84" />
              <p class="poster-title">Death Roll</p>
              <p class="poster-tag">Endless downhill · one hit = death</p>

              {#if phase === 'missing'}
                <span class="badge badge--gold">Web build coming soon</span>
              {:else}
                <button
                  type="button"
                  class="btn btn-primary"
                  disabled={phase !== 'ready'}
                  aria-busy={phase === 'checking'}
                  onclick={play}
                >
                  {phase === 'checking' ? 'Loading…' : 'Play now'}
                </button>
              {/if}
            </div>
          {/if}
        </div>

        <div class="stage-actions">
          {#if phase === 'playing'}
            {#if canFullscreen}
              <button type="button" class="btn btn-ghost" onclick={goFullscreen}>Full screen</button>
            {/if}
            <a class="btn btn-ghost" href={GAME_URL} target="_blank" rel="noopener">Open full page ↗</a>
            <button type="button" class="btn btn-ghost" onclick={stop}>Stop</button>
          {:else if phase === 'ready'}
            <a class="btn btn-ghost" href={GAME_URL} target="_blank" rel="noopener">Open full page ↗</a>
          {/if}
        </div>

        {#if phase === 'playing'}
          <p class="stage-hint">
            The game takes over touch while it runs. Press <strong>Stop</strong> to scroll the
            page again; on a phone, <strong>Full screen</strong> or <strong>Open full page</strong>
            gives the best view.
          </p>
        {/if}

        <noscript>
          <a class="btn btn-primary" href={GAME_URL}>Play Death Roll</a>
        </noscript>
      </div>

      <aside class="play-side" aria-label="How to play Death Roll">
        <h3 class="side-title">How to play</h3>

        <dl class="controls">
          {#each controls as c (c.device)}
            <div class="control-row">
              <dt>{c.device}</dt>
              <dd>{c.action}</dd>
            </div>
          {/each}
        </dl>

        <p class="side-text">
          Ramps throw you into the air; trees and rocks end the run. Your best distance is
          saved in this browser.
        </p>

        <div class="side-tags">
          <span class="tag-chip">C</span>
          <span class="tag-chip">raylib</span>
          <span class="tag-chip">WebAssembly</span>
        </div>

        <p class="side-note">
          <span class="badge badge--crimson">Early build</span>
          Just the core loop so far: pickups, unlocks and sound come next.
        </p>
      </aside>
    </div>
  </div>
</section>

<style>
  .play-grid {
    display: grid;
    gap: var(--sp-4);
    align-items: start;
  }

  .play-main {
    min-width: 0;
  }

  /* ---------- Stage ---------- */

  .stage {
    position: relative;
    width: 100%;
    aspect-ratio: 4 / 5;
    max-height: 82svh;
    overflow: hidden;
    border-radius: var(--radius-md);
    border: 1px solid var(--line-strong);
    background: var(--void-950);
    box-shadow: var(--shadow-card);
  }

  .stage:fullscreen {
    aspect-ratio: auto;
    max-height: none;
    border: 0;
    border-radius: 0;
  }

  .game-frame {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    border: 0;
    display: block;
    background: var(--void-950);
  }

  /* ---------- Poster (before play) ---------- */

  .poster {
    position: absolute;
    inset: 0;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    gap: var(--sp-1);
    padding: var(--sp-3);
    text-align: center;
    overflow: hidden;
    background:
      radial-gradient(70% 55% at 50% 0%, rgba(255, 61, 94, 0.22), transparent 70%),
      linear-gradient(180deg, #0d1830 0%, var(--void-950) 100%);
  }

  /* a slope running down to the right, behind everything */
  .poster-slope {
    position: absolute;
    inset: 0;
    background: linear-gradient(180deg, rgba(0, 212, 255, 0.14), rgba(65, 105, 225, 0.04));
    clip-path: polygon(0 62%, 100% 92%, 100% 100%, 0 100%);
    pointer-events: none;
  }

  .poster > :not(.poster-slope) {
    position: relative;
  }

  .poster-ball {
    width: 84px;
    height: 84px;
    object-fit: contain;
    filter: drop-shadow(0 0 14px rgba(255, 61, 94, 0.45));
  }

  @media (prefers-reduced-motion: no-preference) {
    .poster-ball {
      animation: roll 6s linear infinite;
    }
  }

  @keyframes roll {
    to {
      transform: rotate(360deg);
    }
  }

  .poster-title {
    font-family: var(--font-display);
    font-size: var(--fs-3xl);
    font-weight: 800;
    letter-spacing: 0.04em;
    text-transform: uppercase;
    background: var(--grad-crimson);
    -webkit-background-clip: text;
    background-clip: text;
    -webkit-text-fill-color: transparent;
  }

  .poster-tag {
    margin-bottom: var(--sp-2);
    font-family: var(--font-mono);
    font-size: var(--fs-xs);
    letter-spacing: 0.1em;
    text-transform: uppercase;
    color: var(--ink-300);
  }

  .poster .btn:disabled {
    opacity: 0.6;
    cursor: progress;
    transform: none;
  }

  /* ---------- Actions under the stage ---------- */

  .stage-actions {
    display: flex;
    flex-wrap: wrap;
    gap: var(--sp-1);
    margin-top: var(--sp-2);
  }

  .stage-actions:empty {
    display: none;
  }

  .stage-actions .btn {
    padding: 0.65em 1.2em;
  }

  .stage-hint {
    margin-top: var(--sp-2);
    font-size: var(--fs-sm);
    color: var(--ink-500);
  }

  .stage-hint strong {
    color: var(--ink-300);
    font-weight: 600;
  }

  /* ---------- Side panel ---------- */

  .play-side {
    padding: var(--sp-3);
    border-radius: var(--radius-md);
    border: 1px solid var(--line);
    background: var(--void-800);
  }

  .side-title {
    margin-bottom: var(--sp-2);
    font-size: var(--fs-lg);
  }

  .controls {
    display: grid;
    gap: var(--sp-1);
    margin: 0 0 var(--sp-2);
  }

  .control-row {
    display: flex;
    flex-direction: column;
    gap: 0.15rem;
    padding-bottom: var(--sp-1);
    border-bottom: 1px solid var(--line);
  }

  .control-row:last-child {
    border-bottom: none;
  }

  .control-row dt {
    font-family: var(--font-mono);
    font-size: var(--fs-xs);
    letter-spacing: 0.08em;
    text-transform: uppercase;
    color: var(--cyan);
  }

  .control-row dd {
    margin: 0;
    color: var(--ink-100);
  }

  .side-text {
    margin-bottom: var(--sp-2);
    color: var(--ink-300);
  }

  .side-tags {
    display: flex;
    flex-wrap: wrap;
    gap: 0.5rem;
    margin-bottom: var(--sp-2);
  }

  .side-note {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 0.6rem;
    font-size: var(--fs-sm);
    color: var(--ink-500);
  }

  /* ---------- Layout ---------- */

  @media (min-width: 700px) {
    .stage {
      aspect-ratio: 16 / 9;
      max-height: none;
    }
  }

  @media (min-width: 960px) {
    .play-grid {
      grid-template-columns: minmax(0, 2fr) minmax(0, 1fr);
    }
  }
</style>
