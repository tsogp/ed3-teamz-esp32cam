<template>
  <div class="relative w-full h-screen bg-gray-900">
    <div v-if="hasError" class="absolute inset-0 flex flex-col items-center justify-center space-y-4 text-white">
      <div class="animate-spin rounded-full h-32 w-32 border-t-4 border-blue-500"></div>
      <p class="text-lg font-semibold">Error loading stream. Retrying...</p>
    </div>
    <img v-else :src="streamUrl" alt="MJPEG Stream" class="absolute inset-0 w-full h-full object-contain"
      @error="handleStreamError" />
  
    <div class="absolute inset-0 flex flex-row justify-between p-14 pointer-events-none">
      <div class="self-end pointer-events-auto">
        <Joystick :movement-type="JoystickMovementType.OMNIDIRECTIONAL"
          @movement-changed="(event) => console.log('changed', event)"
          @movement-ended="(event) => console.log('ended', event)" />
      </div>
      <div class="self-end pointer-events-auto">
        <Joystick :movement-type="JoystickMovementType.ROTATIONAL"
          @movement-changed="(event) => console.log('changed', event)"
          @movement-ended="(event) => console.log('ended', event)" />
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue';
import Joystick from '../../components/joystick/Joystick.vue';
import { JoystickMovementType } from '../../components/joystick/types';
import { DEFAULT_URL } from '../../services/types';

const streamUrl = ref(DEFAULT_URL);
const hasError = ref(false);

const handleStreamError = () => {
  console.error("Error loading the MJPEG stream.");
  hasError.value = true;
};
</script>