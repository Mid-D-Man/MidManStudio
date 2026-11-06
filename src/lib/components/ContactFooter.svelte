<!-- FILE: src/lib/components/ContactFooter.svelte -->
<script lang="ts">
  const year = new Date().getFullYear();

  const email = 'midmanstudio@proton.me';

  const links = [
    { label: 'Telegram', href: 'https://t.me/midmanstudio' },
    { label: 'Discord', href: 'https://discord.gg/vtxVx4XNU' },
    { label: 'GitHub — Mid-D-Man', href: 'https://github.com/Mid-D-Man' },
    { label: 'GitHub — MidManStudio', href: 'https://github.com/MidManStudio' }
  ];

  const footerNav = [
    { label: 'Packages', href: '#packages' },
    { label: 'Core Tech', href: '#tech' },
    { label: 'Showcase', href: '#showcase' },
    { label: 'Play', href: '#play' },
    { label: 'Beyond Games', href: '#projects' },
    { label: 'About', href: '#about' }
  ];
</script>

<section id="contact" class="section contact-section">
  <div class="container contact-inner">
    <span class="section-label">07 — Contact</span>
    <h2 class="section-title">
      Let's build <em>something</em>
    </h2>
    <p class="section-intro">
      Open to collaborations, contract work, and questions about any of the
      packages or tools above. Reach out by email, on Telegram, or in the Discord server.
    </p>

    <p class="contact-email">
      <a href="mailto:{email}">{email}</a>
    </p>

    <div class="contact-cta-row">
      <a href="mailto:{email}" class="btn btn-primary">Email Us</a>
      {#each links as l (l.href)}
        <a href={l.href} target="_blank" rel="noopener noreferrer" class="btn btn-ghost">
          {l.label}
        </a>
      {/each}
    </div>
  </div>
</section>

<footer class="footer">
  <div class="container footer-inner">
    <div class="footer-brand">
      <img class="logo-mark" src="/brand/logo-mark.png" alt="" width="28" height="28" />
      <span class="footer-brand-text">MidMan<em>Studio</em></span>
    </div>

    <nav class="footer-nav" aria-label="Footer">
      {#each footerNav as n (n.href)}
        <a href={n.href}>{n.label}</a>
      {/each}
    </nav>

    <p class="footer-copy">© {year} MidManStudio. Built with SvelteKit, deployed on Cloudflare Pages.</p>
  </div>
</footer>

<style>
  .contact-section {
    text-align: center;
  }

  .contact-inner {
    display: flex;
    flex-direction: column;
    align-items: center;
  }

  .contact-section .section-intro {
    margin-inline: auto;
  }

  .contact-email {
    margin-bottom: var(--sp-3);
    font-family: var(--font-mono);
    font-size: var(--fs-base);
    overflow-wrap: anywhere;
  }

  .contact-email a {
    color: var(--cyan);
  }

  .contact-email a:hover {
    text-decoration: underline;
  }

  .contact-cta-row {
    display: flex;
    flex-wrap: wrap;
    justify-content: center;
    gap: var(--sp-2);
    margin-top: var(--sp-2);
  }

  .footer {
    border-top: 1px solid var(--line);
    background: var(--void-900);
    padding-block: var(--sp-4);
  }

  .footer-inner {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: var(--sp-3);
    text-align: center;
  }

  .footer-brand {
    display: inline-flex;
    align-items: center;
    gap: 0.5rem;
    font-family: var(--font-display);
  }

  .logo-mark {
    width: 28px;
    height: 28px;
    object-fit: contain;
    filter: drop-shadow(0 0 5px rgba(57, 255, 20, 0.35));
  }

  .footer-brand-text {
    font-size: var(--fs-sm);
    font-weight: 700;
    color: var(--ink-100);
  }

  .footer-brand-text em {
    font-style: normal;
    background: var(--grad-accent);
    -webkit-background-clip: text;
    background-clip: text;
    -webkit-text-fill-color: transparent;
  }

  .footer-nav {
    display: flex;
    flex-wrap: wrap;
    justify-content: center;
    gap: var(--sp-3);
    font-size: var(--fs-sm);
  }

  .footer-nav a {
    color: var(--ink-500);
    transition: color var(--dur-fast) var(--ease);
  }

  .footer-nav a:hover {
    color: var(--cyan);
  }

  .footer-copy {
    font-size: var(--fs-xs);
    color: var(--ink-700);
  }

  @media (min-width: 768px) {
    .footer-inner {
      flex-direction: row;
      justify-content: space-between;
      text-align: left;
    }
  }
</style>
