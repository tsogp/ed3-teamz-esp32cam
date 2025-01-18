<template>
  <div class="relative w-full h-full bg-gray-900">

    <div v-if="loading" class="absolute inset-0 flex flex-col items-center justify-center space-y-4 text-white">
      <div class="animate-spin rounded-full h-32 w-32 border-t-4 border-blue-500"></div>
      <p v-if="hasError" class="text-lg font-semibold">Error loading stream. Retrying...</p>
    </div>

    <img v-show="!loading" :src="streamUrl" alt="MJPEG Stream" class="relative inset-0 w-full h-full object-contain"
      @error="handleStreamError" @load="handleStreamLoaded">
    <FPSComponent :fps="fps" class="absolute top-4 right-4 z-10 p-1" />
  </div>
</template>

<script setup>
import { onMounted, ref } from 'vue';
import { DEFAULT_URL } from '../../services/types';
import FPSComponent from '../../components/fps/FPSComponent.vue';
import api from '../../services/generic-api';

const streamUrl = ref(DEFAULT_URL);
const hasError = ref(false);
const loading = ref(true);
const fps = ref(0);

const handleStreamLoaded = () => {
  console.log("Picture stream loaded.");
  loading.value = false;
};

const handleStreamError = (event) => {
  console.error("Error loading the MJPEG stream:", event);

  if (event && event.message && event.message.includes("ERR_INVALID_CHUNKED_ENCODING")) {
    console.error("Invalid chunked encoding detected. Reloading the page...");
    window.location.reload();
  } else {
    hasError.value = true;
    loading.value = true;
    console.error("An unknown error occurred. Retrying...");
    setTimeout(() => {
      window.location.reload();
    }, 1000);
  }
};

onMounted(() => {
  api.addWsListener("message", (data) => {
    try {
      const num = Number(data);
      if (num) {
        fps.value = num;
      }
    } catch (e) { }
  });
})
</script>
