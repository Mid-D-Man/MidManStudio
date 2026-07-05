<!-- FILE: src/routes/+page.svelte -->
<script lang="ts">
  import Hero from '$lib/components/Hero.svelte';
  import UnityPackages from '$lib/components/UnityPackages.svelte';
  import CoreTech from '$lib/components/CoreTech.svelte';
  import MediaGallery from '$lib/components/MediaGallery.svelte';
  import Projects from '$lib/components/Projects.svelte';
  import About from '$lib/components/About.svelte';
  import ContactFooter from '$lib/components/ContactFooter.svelte';
</script>

<svelte:head>
  <title>MidManStudio — Game Development Studio</title>
  <meta
    name="description"
    content="MidManStudio: Unity packages, custom Rust-powered engine tech, and shipped game & software projects."
  />
</svelte:head>

<Hero />
<UnityPackages />
<CoreTech />
<MediaGallery />
<Projects />
<About />
<ContactFooter />
