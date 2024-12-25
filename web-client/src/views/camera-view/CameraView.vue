<template>
  <div class="relative w-[700px] h-[700px] bg-gray-900">
    <div v-if="hasError" class="absolute inset-0 flex flex-col items-center justify-center space-y-4 text-white">
      <div class="animate-spin rounded-full h-32 w-32 border-t-4 border-blue-500"></div>
      <p class="text-lg font-semibold">Error loading stream. Retrying...</p>
    </div>
    <img v-else :src="streamUrl" alt="MJPEG Stream" class="absolute inset-0 w-full h-full" @error="handleStreamError" />
  </div>
</template>

<script setup>
import { ref } from 'vue';
import { DEFAULT_URL } from '../../services/types';

const streamUrl = ref(DEFAULT_URL);
const hasError = ref(false);

const handleStreamError = () => {
  console.error("Error loading the MJPEG stream.");
  hasError.value = true;
};
</script>