<!-- FILE: src/lib/components/Projects.svelte -->
<script lang="ts">
  import { projects } from '$lib/data/projects';
  import { reveal } from '$lib/actions/reveal';

  const statusLabel = { featured: 'Featured', 'in-progress': 'In Progress', completed: 'Completed' } as const;
</script>

<section id="projects" class="section section--alt">
  <div class="container">
    <span class="section-label">04 — Beyond Games</span>
    <h2 class="section-title reveal" use:reveal>
      Web work <em>and research</em>
    </h2>
    <p class="section-intro reveal" use:reveal>
      Game dev is the focus, not the whole story — attendance platforms, a
      compression algorithm under active research, and a few web builds.
    </p>

    <div class="grid-2 projects-grid">
      {#each projects as p, i (p.id)}
        <article class="card project-card reveal" use:reveal style="transition-delay: {i * 80}ms">
          <div class="project-head">
            <span class="project-number">{p.number}</span>
            <span
              class="badge"
              class:badge--gold={p.status === 'featured'}
              class:badge--cyan={p.status === 'in-progress'}
            >
              {statusLabel[p.status]}
            </span>
          </div>
          <h3 class="project-title">{p.title}</h3>
          <p class="project-subtitle">{p.subtitle}</p>
          <p class="project-desc">{p.description}</p>
          <div class="project-tags">
            {#each p.tech as t (t)}
              <span class="tag-chip">{t}</span>
            {/each}
          </div>
          <a href={p.link} target="_blank" rel="noopener noreferrer" class="project-link">
            {p.linkLabel} <span aria-hidden="true">↗</span>
          </a>
        </article>
      {/each}
    </div>
  </div>
</section>

<style>
  .projects-grid {
    align-items: stretch;
  }

  .project-card {
    display: flex;
    flex-direction: column;
    gap: var(--sp-2);
  }

  .project-head {
    display: flex;
    align-items: center;
    justify-content: space-between;
  }

  .project-number {
    font-family: var(--font-mono);
    color: var(--ink-500);
    font-size: var(--fs-sm);
  }

  .project-title {
    font-size: var(--fs-xl);
  }

  .project-subtitle {
    color: var(--cyan);
    font-weight: 600;
    font-size: var(--fs-sm);
  }

  .project-desc {
    color: var(--ink-300);
    flex-grow: 1;
  }

  .project-tags {
    display: flex;
    flex-wrap: wrap;
    gap: 0.5rem;
  }

  .project-link {
    display: inline-flex;
    align-items: center;
    gap: 0.4em;
    font-family: var(--font-mono);
    font-size: var(--fs-sm);
    color: var(--gold);
    width: fit-content;
  }
</style>
