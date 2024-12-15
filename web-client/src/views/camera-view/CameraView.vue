<template>
  <div class="relative w-[800px] h-[800px] bg-gray-900">
    <div v-if="hasError" class="absolute inset-0 flex flex-col items-center justify-center space-y-4 text-white">
      <div class="animate-spin rounded-full h-32 w-32 border-t-4 border-blue-500"></div>
      <p class="text-lg font-semibold">Error loading stream. Retrying...</p>
    </div>
    <img v-else :src="streamUrl" alt="MJPEG Stream" class="absolute inset-0 w-full h-full" @error="handleStreamError" />

    <div class="absolute inset-0 flex flex-row justify-between p-14 pointer-events-none">
      <div class="self-end pointer-events-auto">
        <Joystick :movement-type="JoystickMovementType.OMNIDIRECTIONAL"
          @movement-changed="(event) => sendJoystickData(event, JoystickMovementStatus.MOVING, JoystickMovementType.OMNIDIRECTIONAL)"
          @movement-ended="(event) => sendJoystickData(event, JoystickMovementStatus.STOPPED, JoystickMovementType.OMNIDIRECTIONAL)" />
      </div>
      <div class="self-end pointer-events-auto">
        <Joystick :movement-type="JoystickMovementType.ROTATIONAL"
          @movement-changed="(event) => sendJoystickData(event, JoystickMovementStatus.MOVING, JoystickMovementType.ROTATIONAL)"
          @movement-ended="(event) => sendJoystickData(event, JoystickMovementStatus.STOPPED, JoystickMovementType.ROTATIONAL)" />
      </div>
    </div>
  </div>

  <div>
    FPS: {{ fps }}
  </div>
</template>

<script setup>
import { onMounted, onUnmounted, ref } from 'vue';
import Joystick from '../../components/joystick/Joystick.vue';
import { JoystickMovementStatus, JoystickMovementType } from '../../components/joystick/types';
import api from '../../services/generic-api';
import { DEFAULT_URL } from '../../services/types';

const sendJoystickData = async (event, status, type) => {
  const data = { t: type, s: status, d: event };
  if (wsConnected.value) {
    api.sendWebSocketMessage(data);
  }
}

const streamUrl = ref(DEFAULT_URL);
const hasError = ref(false);
const wsConnected = ref(false);
const fps = ref("")

const handleStreamError = () => {
  console.error("Error loading the MJPEG stream.");
  hasError.value = true;
};

onMounted(() => {
  api.connectWebSocket("/controls");
  wsConnected.value = true;

  api.addWsListener("message", (data) => {
    fps.value = data;
  });
});

onUnmounted(() => {
  api.closeWebSocket();
  wsConnected.value = false;
});
</script>