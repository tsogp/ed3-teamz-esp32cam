<template>
  <div class="rounded bg-white flex flex-row">
    <span class="mr-1 font-semibold">FPS:</span>
    <div :class="fpsClass" class="font-semibold">
      {{ fps }}
    </div>
  </div>
</template>

<script setup>
import { computed } from 'vue';

const props = defineProps({
  fps: {
    type: Number,
    required: true
  }
});

const fpsClass = computed(() => {
  if (props.fps < 10) {
    return 'text-red-500';
  } else if (props.fps >= 10 && props.fps <= 20) {
    return 'text-orange-500';
  } else {
    return 'text-green-500';
  }
});

</script>
