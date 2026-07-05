<!-- FILE: src/lib/components/Hero.svelte -->
<script lang="ts">
  const techTags = ['Unity', 'Rust', 'C#', 'SIMD', 'NGO', 'WASM'];
</script>

<section id="hero" class="hero">
  <div class="hero-bg" aria-hidden="true">
    <div class="hero-grid"></div>
    <div class="orb orb--royal"></div>
    <div class="orb orb--crimson"></div>
  </div>

  <div class="container hero-content">
    <div class="hero-eyebrow">
      <span class="dot"></span>
      <span>Game Development Studio</span>
    </div>

    <h1 class="hero-headline">
      MidMan<span class="accent">Studio</span>
    </h1>

    <p class="hero-desc">
      Unity packages, a Rust-powered engine core, and shipped multiplayer
      systems — built by one studio, from the SIMD math up to the game feel.
    </p>

    <div class="hero-cta-row">
      <a href="#packages" class="btn btn-primary">See the Packages</a>
      <a href="#contact" class="btn btn-ghost">Get In Touch</a>
    </div>

    <div class="hero-tags" aria-hidden="true">
      {#each techTags as tag, i (tag)}
        <span class="tag-chip" style="animation-delay: {i * 0.15}s">{tag}</span>
      {/each}
    </div>
  </div>
</section>

<style>
  .hero {
    position: relative;
    min-height: 100svh;
    display: flex;
    align-items: center;
    padding-top: 72px;
    overflow: hidden;
    background: var(--grad-hero);
  }

  .hero-bg {
    position: absolute;
    inset: 0;
    pointer-events: none;
    z-index: 0;
  }

  .hero-grid {
    position: absolute;
    inset: 0;
    background-image:
      linear-gradient(var(--line) 1px, transparent 1px),
      linear-gradient(90deg, var(--line) 1px, transparent 1px);
    background-size: 56px 56px;
    mask-image: radial-gradient(ellipse 80% 60% at 50% 30%, black 40%, transparent 90%);
  }

  .orb {
    position: absolute;
    width: 40vw;
    height: 40vw;
    max-width: 520px;
    max-height: 520px;
    border-radius: 50%;
    filter: blur(90px);
    opacity: 0.35;
  }

  .orb--royal {
    top: -10%;
    left: -10%;
    background: var(--royal);
  }

  .orb--crimson {
    bottom: -15%;
    right: -10%;
    background: var(--crimson);
  }

  .hero-content {
    position: relative;
    z-index: 1;
    display: flex;
    flex-direction: column;
    gap: var(--sp-3);
  }

  .hero-eyebrow {
    display: inline-flex;
    align-items: center;
    gap: 0.6rem;
    font-family: var(--font-mono);
    font-size: var(--fs-sm);
    letter-spacing: 0.1em;
    text-transform: uppercase;
    color: var(--cyan);
  }

  .dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    background: var(--cyan);
    box-shadow: var(--glow-cyan);
  }

  .hero-headline {
    font-size: var(--fs-4xl);
    color: var(--ink-100);
  }

  .accent {
    background: var(--grad-accent);
    -webkit-background-clip: text;
    background-clip: text;
    -webkit-text-fill-color: transparent;
  }

  .hero-desc {
    max-width: 56ch;
    font-size: var(--fs-lg);
    color: var(--ink-300);
  }

  .hero-cta-row {
    display: flex;
    gap: var(--sp-2);
    margin-top: var(--sp-2);
  }

  .hero-tags {
    display: flex;
    flex-wrap: wrap;
    gap: 0.6rem;
    margin-top: var(--sp-4);
  }

  .hero-tags .tag-chip {
    animation: float 4s ease-in-out infinite;
  }

  @keyframes float {
    0%, 100% { transform: translateY(0); }
    50% { transform: translateY(-6px); }
  }

  @media (prefers-reduced-motion: reduce) {
    .hero-tags .tag-chip {
      animation: none;
    }
  }
</style>
