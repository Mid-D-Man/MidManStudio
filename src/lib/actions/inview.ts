// FILE: src/lib/actions/inview.ts
// Svelte action: calls back with true when the element is on screen (or within `rootMargin` of it) and false when it leaves,
// every time it changes. reveal.ts only reports the first entry; this one is for things that start and stop (GIFs).
// Usage: <div use:inview={(on) => (visible = on)}>

export type InviewCallback = (visible: boolean) => void;

export function inview(node: HTMLElement, callback: InviewCallback) {
  let cb = callback;

  // No IntersectionObserver (very old browser): behave as if always on screen.
  if (typeof IntersectionObserver === 'undefined') {
    cb(true);
    return {
      update(next: InviewCallback) {
        cb = next;
      }
    };
  }

  // The margin starts a clip a little before it scrolls in, so it has loaded by the time it is seen.
  const observer = new IntersectionObserver(
    (entries) => {
      const last = entries[entries.length - 1];
      if (last) cb(last.isIntersecting);
    },
    { rootMargin: '120px 0px', threshold: 0 }
  );
  observer.observe(node);

  return {
    update(next: InviewCallback) {
      cb = next;
    },
    destroy() {
      observer.disconnect();
    }
  };
}
