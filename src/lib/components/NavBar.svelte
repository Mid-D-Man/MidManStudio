<!-- FILE: src/lib/components/NavBar.svelte -->
<script lang="ts">
  import { onMount } from 'svelte';

  let scrolled = $state(false);
  let menuOpen = $state(false);

  const links = [
    { href: '#packages', label: 'Packages' },
    { href: '#tech', label: 'Core Tech' },
    { href: '#showcase', label: 'Showcase' },
    { href: '#projects', label: 'Beyond Games' },
    { href: '#about', label: 'About' },
    { href: '#contact', label: 'Contact' }
  ];

  function closeMenu() {
    menuOpen = false;
  }

  onMount(() => {
    const onScroll = () => {
      scrolled = window.scrollY > 24;
    };
    onScroll();
    window.addEventListener('scroll', onScroll, { passive: true });
    return () => window.removeEventListener('scroll', onScroll);
  });
</script>

<header class="nav" class:nav--scrolled={scrolled}>
  <div class="container nav-inner">
    <a href="#hero" class="nav-logo" onclick={closeMenu}>
      <span class="logo-mark">M</span>
      <span class="logo-text">MidMan<em>Studio</em></span>
    </a>

    <nav class="nav-links" aria-label="Primary">
      {#each links as link (link.href)}
        <a href={link.href}>{link.label}</a>
      {/each}
    </nav>

    <button
      class="nav-burger"
      aria-label={menuOpen ? 'Close menu' : 'Open menu'}
      aria-expanded={menuOpen}
      onclick={() => (menuOpen = !menuOpen)}
    >
      <span class:open={menuOpen}></span>
      <span class:open={menuOpen}></span>
      <span class:open={menuOpen}></span>
    </button>
  </div>

  {#if menuOpen}
    <nav class="nav-mobile" aria-label="Mobile">
      {#each links as link (link.href)}
        <a href={link.href} onclick={closeMenu}>{link.label}</a>
      {/each}
    </nav>
  {/if}
</header>

<style>
  .nav {
    position: fixed;
    inset: 0 0 auto 0;
    z-index: 100;
    border-bottom: 1px solid transparent;
    transition: background var(--dur-normal) var(--ease), border-color var(--dur-normal) var(--ease);
  }

  .nav--scrolled {
    background: rgba(4, 6, 12, 0.82);
    backdrop-filter: blur(14px);
    -webkit-backdrop-filter: blur(14px);
    border-bottom-color: var(--line);
  }

  .nav-inner {
    display: flex;
    align-items: center;
    justify-content: space-between;
    height: 72px;
  }

  .nav-logo {
    display: inline-flex;
    align-items: center;
    gap: 0.6rem;
    font-family: var(--font-display);
  }

  .logo-mark {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    width: 34px;
    height: 34px;
    border-radius: 8px;
    background: var(--grad-brand);
    color: var(--void-950);
    font-weight: 900;
    box-shadow: var(--glow-cyan);
  }

  .logo-text {
    font-size: var(--fs-base);
    font-weight: 700;
    letter-spacing: 0.02em;
    color: var(--ink-100);
  }

  .logo-text em {
    font-style: normal;
    background: var(--grad-accent);
    -webkit-background-clip: text;
    background-clip: text;
    -webkit-text-fill-color: transparent;
  }

  .nav-links {
    align-items: center;
    gap: var(--sp-4);
    font-size: var(--fs-sm);
    font-weight: 500;
  }

  .nav-links a {
    color: var(--ink-300);
    transition: color var(--dur-fast) var(--ease);
  }

  .nav-links a:hover {
    color: var(--cyan);
  }

  .nav-burger {
    display: flex;
    flex-direction: column;
    justify-content: center;
    gap: 5px;
    width: 34px;
    height: 34px;
    background: none;
    border: none;
    cursor: pointer;
    padding: 0;
  }

  .nav-burger span {
    height: 2px;
    background: var(--ink-100);
    border-radius: 2px;
    transition: transform var(--dur-fast) var(--ease), opacity var(--dur-fast) var(--ease);
  }

  .nav-burger span.open:nth-child(1) {
    transform: translateY(7px) rotate(45deg);
  }
  .nav-burger span.open:nth-child(2) {
    opacity: 0;
  }
  .nav-burger span.open:nth-child(3) {
    transform: translateY(-7px) rotate(-45deg);
  }

  .nav-mobile {
    display: flex;
    flex-direction: column;
    padding: var(--sp-2) var(--container-pad) var(--sp-4);
    background: rgba(4, 6, 12, 0.96);
    border-bottom: 1px solid var(--line);
  }

  .nav-mobile a {
    padding: var(--sp-2) 0;
    border-bottom: 1px solid var(--line);
    color: var(--ink-100);
    font-size: var(--fs-lg);
  }

  .nav-mobile a:last-child {
    border-bottom: none;
  }
</style>
