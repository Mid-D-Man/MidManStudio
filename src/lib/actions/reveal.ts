// FILE: src/lib/actions/reveal.ts
// Svelte action: adds .is-visible to an element the first time it enters
// the viewport. Pairs with the .reveal / .reveal.is-visible classes in
// base.css. Usage: <div class="reveal" use:reveal>

export function reveal(node: HTMLElement) {
  const observer = new IntersectionObserver(
    (entries) => {
      for (const entry of entries) {
        if (entry.isIntersecting) {
          node.classList.add('is-visible');
          observer.unobserve(node);
        }
      }
    },
    { threshold: 0.15, rootMargin: '0px 0px -40px 0px' }
  );

  observer.observe(node);

  return {
    destroy() {
      observer.disconnect();
    }
  };
    }
