<!-- FILE: src/lib/components/UnityPackages.svelte -->
<script lang="ts">
  import { unityPackages } from '$lib/data/packages';
  import { reveal } from '$lib/actions/reveal';
</script>

<section id="packages" class="section">
  <div class="container">
    <span class="section-label">01 — Unity Packages</span>
    <h2 class="section-title reveal" use:reveal>
      Tools other <em>developers ship with</em>
    </h2>
    <p class="section-intro reveal" use:reveal>
      Three UPM packages published under the <code>com.midmanstudio</code> scope —
      built for real projects first, released as reusable packages second.
    </p>

    <div class="packages-list">
      {#each unityPackages as pkg, i (pkg.id)}
        <article class="card package-card reveal" use:reveal style="transition-delay: {i * 90}ms">
          <div class="package-head">
            <div>
              <span class="badge" class:badge--gold={pkg.status === 'shipping'} class:badge--cyan={pkg.status === 'active-dev'}>
                {pkg.status === 'shipping' ? 'Shipping' : 'Active Dev'}
              </span>
              <h3 class="package-name">{pkg.name}</h3>
              <p class="package-scope">{pkg.scope}</p>
            </div>
            <div class="media-slot package-media">
              <span class="media-slot__label">Screenshot / GIF</span>
            </div>
          </div>

          <p class="package-tagline">{pkg.tagline}</p>
          <p class="package-desc">{pkg.description}</p>

          <ul class="package-highlights">
            {#each pkg.highlights as h (h)}
              <li>{h}</li>
            {/each}
          </ul>

          <div class="package-tags">
            {#each pkg.tech as t (t)}
              <span class="tag-chip">{t}</span>
            {/each}
          </div>
        </article>
      {/each}
    </div>
  </div>
</section>

<style>
  .packages-list {
    display: flex;
    flex-direction: column;
    gap: var(--sp-4);
  }

  .package-card {
    display: flex;
    flex-direction: column;
    gap: var(--sp-3);
  }

  .package-head {
    display: flex;
    flex-direction: column-reverse;
    gap: var(--sp-3);
  }

  .package-media {
    width: 100%;
  }

  .package-name {
    font-size: var(--fs-2xl);
    margin-top: var(--sp-1);
  }

  .package-scope {
    font-family: var(--font-mono);
    font-size: var(--fs-xs);
    color: var(--ink-500);
    margin-top: 0.25rem;
  }

  .package-tagline {
    font-size: var(--fs-lg);
    color: var(--cyan);
    font-weight: 600;
  }

  .package-desc {
    color: var(--ink-300);
  }

  .package-highlights {
    margin: 0;
    padding-left: 1.1rem;
    display: flex;
    flex-direction: column;
    gap: 0.4rem;
    color: var(--ink-300);
    font-size: var(--fs-sm);
  }

  .package-tags {
    display: flex;
    flex-wrap: wrap;
    gap: 0.5rem;
  }

  @media (min-width: 768px) {
    .package-head {
      flex-direction: row;
      align-items: flex-start;
      justify-content: space-between;
    }

    .package-media {
      width: 40%;
      flex-shrink: 0;
    }
  }
</style>
