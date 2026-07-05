<!-- FILE: src/lib/components/CoreTech.svelte -->
<script lang="ts">
  import { ecosystemItems } from '$lib/data/ecosystem';
  import { reveal } from '$lib/actions/reveal';
</script>

<section id="tech" class="section section--alt">
  <div class="container">
    <span class="section-label">02 — Core Tech</span>
    <h2 class="section-title reveal" use:reveal>
      What's running <em>under the hood</em>
    </h2>
    <p class="section-intro reveal" use:reveal>
      The Unity packages sit on top of a homegrown Rust stack — simulation cores,
      SIMD math, a custom scripting language, and a vector graphics engine, all
      built in-house rather than bolted on from a marketplace.
    </p>

    <div class="grid-2 tech-grid">
      {#each ecosystemItems as item, i (item.id)}
        <article
          class="card tech-card tech-card--{item.accent} reveal"
          use:reveal
          style="transition-delay: {i * 90}ms"
        >
          <p class="tech-tagline">{item.tagline}</p>
          <h3 class="tech-name">{item.name}</h3>
          <p class="tech-desc">{item.description}</p>
          <div class="tech-tags">
            {#each item.tags as t (t)}
              <span class="tag-chip">{t}</span>
            {/each}
          </div>
          <a href={item.link} target="_blank" rel="noopener noreferrer" class="tech-link">
            {item.linkLabel} <span aria-hidden="true">↗</span>
          </a>
        </article>
      {/each}
    </div>
  </div>
</section>

<style>
  .tech-grid {
    align-items: stretch;
  }

  .tech-card {
    display: flex;
    flex-direction: column;
    gap: var(--sp-2);
    border-top: 3px solid var(--line);
  }

  .tech-card--royal { border-top-color: var(--royal); }
  .tech-card--cyan { border-top-color: var(--cyan); }
  .tech-card--crimson { border-top-color: var(--crimson); }
  .tech-card--gold { border-top-color: var(--gold); }

  .tech-tagline {
    font-family: var(--font-mono);
    font-size: var(--fs-xs);
    letter-spacing: 0.05em;
    text-transform: uppercase;
    color: var(--ink-500);
  }

  .tech-name {
    font-size: var(--fs-xl);
  }

  .tech-desc {
    color: var(--ink-300);
    flex-grow: 1;
  }

  .tech-tags {
    display: flex;
    flex-wrap: wrap;
    gap: 0.5rem;
  }

  .tech-link {
    display: inline-flex;
    align-items: center;
    gap: 0.4em;
    font-family: var(--font-mono);
    font-size: var(--fs-sm);
    color: var(--cyan);
    margin-top: var(--sp-1);
    width: fit-content;
    transition: gap var(--dur-fast) var(--ease);
  }

  .tech-link:hover {
    gap: 0.6em;
  }
</style>
