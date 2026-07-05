<!-- FILE: src/lib/components/About.svelte -->
<script lang="ts">
  import { reveal } from '$lib/actions/reveal';

  const stats = [
    { value: '3', label: 'Unity Packages' },
    { value: '5+', label: 'Rust Systems' },
    { value: '1', label: 'Studio, One Founder' }
  ];

  const focus = [
    { label: 'Unity / C#', level: 95 },
    { label: 'Rust', level: 85 },
    { label: 'Systems & SIMD', level: 78 },
    { label: 'Multiplayer / NGO', level: 80 },
    { label: 'Language & Tooling Design', level: 82 }
  ];
</script>

<section id="about" class="section">
  <div class="container about-grid">
    <div class="about-text reveal" use:reveal>
      <span class="section-label">05 — About</span>
      <h2 class="section-title">
        One studio, <em>built end to end</em>
      </h2>

      <p class="about-para">
        MidManStudio is an indie game development studio founded by Mid — AbdulHamid
        Muhammad Suleiman — a game developer and systems programmer who builds most of
        the stack in-house rather than reaching for off-the-shelf packages.
      </p>

      <p class="about-para">
        That means Unity games backed by a Rust simulation core instead of pure
        C# physics, a custom scripting language (DixScript) instead of raw JSON
        config, and a vector graphics engine built from first principles instead
        of a licensed renderer. The throughline across every project is the same:
        understand the layer underneath before shipping the layer on top of it.
      </p>

      <div class="stats-row">
        {#each stats as s (s.label)}
          <div class="stat-card">
            <span class="stat-value">{s.value}</span>
            <span class="stat-label">{s.label}</span>
          </div>
        {/each}
      </div>
    </div>

    <div class="about-side reveal" use:reveal style="transition-delay: 120ms">
      <div class="media-slot about-media">
        <span class="media-slot__label">Studio / Founder Photo</span>
      </div>

      <div class="focus-block">
        <h3 class="focus-heading">Core Focus</h3>
        {#each focus as f (f.label)}
          <div class="focus-row">
            <div class="focus-top">
              <span>{f.label}</span>
              <span class="focus-pct">{f.level}%</span>
            </div>
            <div class="focus-track">
              <div class="focus-fill" style="width: {f.level}%"></div>
            </div>
          </div>
        {/each}
      </div>
    </div>
  </div>
</section>

<style>
  .about-grid {
    display: grid;
    gap: var(--sp-5);
  }

  .about-para {
    margin-top: var(--sp-2);
    color: var(--ink-300);
  }

  .stats-row {
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    gap: var(--sp-2);
    margin-top: var(--sp-4);
  }

  .stat-card {
    display: flex;
    flex-direction: column;
    gap: 0.2rem;
    padding: var(--sp-2);
    border: 1px solid var(--line);
    border-radius: var(--radius-sm);
    text-align: center;
  }

  .stat-value {
    font-family: var(--font-display);
    font-size: var(--fs-xl);
    color: var(--gold);
  }

  .stat-label {
    font-size: var(--fs-xs);
    color: var(--ink-500);
  }

  .about-side {
    display: flex;
    flex-direction: column;
    gap: var(--sp-4);
  }

  .focus-block {
    display: flex;
    flex-direction: column;
    gap: var(--sp-2);
  }

  .focus-heading {
    font-size: var(--fs-lg);
    margin-bottom: 0.25rem;
  }

  .focus-top {
    display: flex;
    justify-content: space-between;
    font-size: var(--fs-sm);
    color: var(--ink-300);
    margin-bottom: 0.3rem;
  }

  .focus-pct {
    font-family: var(--font-mono);
    color: var(--cyan);
  }

  .focus-track {
    height: 6px;
    border-radius: 4px;
    background: var(--void-700);
    overflow: hidden;
  }

  .focus-fill {
    height: 100%;
    background: var(--grad-cyan);
    border-radius: 4px;
  }
</style>
