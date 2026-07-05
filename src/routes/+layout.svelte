<!-- FILE: src/routes/+layout.svelte -->
<script lang="ts">
  import '$lib/styles/tokens.css';
  import '$lib/styles/base.css';
  import '$lib/styles/responsive.css';
  import NavBar from '$lib/components/NavBar.svelte';

  let { children } = $props();
</script>

<a href="#main" class="sr-only">Skip to content</a>

<NavBar />

<main id="main">
  {@render children()}
</main>
