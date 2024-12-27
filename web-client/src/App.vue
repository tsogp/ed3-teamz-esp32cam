<script setup>
import CameraView from './views/camera-view/CameraView.vue'
import Dropdown from './components/dropdown/Dropdown.vue';
import { AVAILABLE_RESOLUTIONS, AVAILABLE_JPEG_QUALITY, OPERATION_MODES, OperationMode, AutoOperationMode } from './types';
import api from './services/generic-api';
import { ref, onMounted, onUnmounted } from 'vue';
import Slider from './components/slider/Slider.vue';
import IconButton from './components/icon-button/IconButton.vue';
import Button from './components/button/Button.vue';
import Joystick from './components/joystick/Joystick.vue';
import { JoystickMovementType } from './components/joystick/types';

const fps = ref("")
const operationMode = ref(OperationMode.JOYSTICK_MANUAL)
const wsConnected = ref(false);
const throttle = ref(0);

const changeResolution = async (resId) => {
  try {
    const response = await api.post("/reload-camera-config", { rs: resId });
    console.log("Changing camera resolution OK");
  } catch (error) {
    console.log("Error changing camera resolution: ", error);
  }
}

const changeQuality = async (qId) => {
  try {
    const response = await api.post("/reload-camera-config", { q: qId });
    console.log("Changing camera resolution OK");
  } catch (error) {
    console.log("Error changing camera resolution: ", error);
  }
}

const sendManualData = async (event, mode, type) => {
  console.log(event)
  const data = { t: type, m: mode, x: event.x, y: event.y };
  if (wsConnected.value) {
    api.sendWebSocketMessage(data);
  }
}

const sendButtonData = async (event, mode, type, throttle) => {
  const data = { t: type, m: mode, x: event.x, y: event.y, th: throttle };
  if (wsConnected.value) {
    api.sendWebSocketMessage(data);
  }
}

const sendAutoButtonData = async (mode, type, throttle) => {
  const data = { t: type, m: mode, th: throttle };
  if (wsConnected.value) {
    api.sendWebSocketMessage(data);
  }
}

onMounted(async () => {
  api.connectWebSocket("/controls");
  wsConnected.value = true;

  api.addWsListener("message", (data) => {
    fps.value = data;
  });

  try {
    const response = await api.options("");
    console.log("CORS handled ok");
  } catch (error) {
    console.log("Error handling CORS: ", error);
  }
})

onUnmounted(() => {
  api.closeWebSocket();
  wsConnected.value = false;
});

</script>

<template>
  <div class="p-5 flex flex-col gap-5">
    <div class="flex flex-row gap-x-2">
      <CameraView class="" />
      <div class="flex flex-row gap-x-2">
        <Dropdown button-label="Video Resolution" :items="AVAILABLE_RESOLUTIONS" title="Choose the resolution"
          @select="(event) => changeResolution(event)" />
        <Dropdown button-label="JPEG Quality" :items="AVAILABLE_JPEG_QUALITY" title="Choose the quality"
          @select="(event) => changeQuality(event)" />
        <Dropdown button-label="Operation Mode" :items="OPERATION_MODES" title="Choose operation mode"
          @select="(value) => operationMode = value" />
      </div>
      <span>FPS: {{ fps }}</span>
    </div>
    <div class="flex flex-col w-full h-full">
      <div class="flex flex-row gap-x-5">
        <Slider :min="0" :max="3" :initial-value="0" name="Throttle"
          v-if="operationMode === OperationMode.BUTTONS_MANUAL || operationMode === OperationMode.BUTTONS_AUTO"
          @update:value="(event) => throttle = event" />
        <div class="grid grid-cols-3 grid-rows-3 gap-4 w-52 h-52" v-if="operationMode === OperationMode.BUTTONS_MANUAL">
          <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(-45deg)"
            @click="sendButtonData({ x: -0.71, y: 0.71 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />
          <IconButton icon="arrow.svg" altText="Example Icon"
            @click="sendButtonData({ x: 0, y: 1 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />
          <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(45deg)"
            @click="sendButtonData({ x: 0.71, y: 0.71 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />
          <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(-90deg)"
            @click="sendButtonData({ x: -1, y: 0 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />
          <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(90deg)"
            class="col-start-3 col-end-4"
            @click="sendButtonData({ x: 1, y: 0 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />
          <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(-135deg)"
            @click="sendButtonData({ x: -0.71, y: -0.71 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />
          <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(180deg)"
            @click="sendButtonData({ x: 0, y: -1 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />
          <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(135deg)"
            @click="sendButtonData({ x: 0.71, y: -0.71 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />

          <button
            @click="sendButtonData({ x: 0, y: 0 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)">stop</button>
        </div>
        <div class="flex flex-col gap-y-3" v-if="operationMode === OperationMode.BUTTONS_AUTO">
          <Button text="Square with turn"
            @click="sendAutoButtonData(OperationMode.BUTTONS_AUTO, AutoOperationMode.SQUARE_WITH_TURN, throttle)" />
          <Button text="Circle with turn"
            @click="sendAutoButtonData(OperationMode.BUTTONS_AUTO, AutoOperationMode.CIRCLE_WITH_TURN, throttle)" />
          <Button text="Square no turn"
            @click="sendAutoButtonData(OperationMode.BUTTONS_AUTO, AutoOperationMode.SQUARE_NO_TURN, throttle)" />
          <Button text="Circle drift"
            @click="sendAutoButtonData(OperationMode.BUTTONS_AUTO, AutoOperationMode.CIRCLE_DRIFT, throttle)" />
        </div>
      </div>

      <div class="flex flex-row gap-x-5 pointer-events-none" v-if="operationMode === OperationMode.JOYSTICK_MANUAL">
        <div class="pointer-events-auto">
          <Joystick :movement-type="JoystickMovementType.OMNIDIRECTIONAL"
            @movement-changed="(event) => sendManualData(event, OperationMode.JOYSTICK_MANUAL, JoystickMovementType.OMNIDIRECTIONAL)"
            @movement-ended="(event) => sendManualData(event, OperationMode.JOYSTICK_MANUAL, JoystickMovementType.OMNIDIRECTIONAL)" />
        </div>
        <div class="pointer-events-auto">
          <Joystick :movement-type="JoystickMovementType.ROTATIONAL"
            @movement-changed="(event) => sendManualData(event, OperationMode.JOYSTICK_MANUAL, JoystickMovementType.ROTATIONAL)"
            @movement-ended="(event) => sendManualData(event, OperationMode.JOYSTICK_MANUAL, JoystickMovementType.ROTATIONAL)" />
        </div>
      </div>
    </div>
  </div>
</template>