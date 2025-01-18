<script setup>
import CameraView from './views/camera-view/CameraView.vue'
import Dropdown from './components/dropdown/Dropdown.vue';
import { AVAILABLE_RESOLUTIONS, AVAILABLE_JPEG_QUALITY, OPERATION_MODES, OperationMode, AutoOperationMode } from './types';
import api from './services/generic-api';
import { ref, onMounted, onUnmounted, onBeforeUnmount } from 'vue';
import Slider from './components/slider/Slider.vue';
import IconButton from './components/icon-button/IconButton.vue';
import Button from './components/button/Button.vue';
import Joystick from './components/joystick/Joystick.vue';
import { JoystickMovementType } from './components/joystick/types';
import pathGen from './services/path-gen2';
import HeaderComponent from './components/header/HeaderComponent.vue';

const fps = ref(0)
const operationMode = ref(OperationMode.JOYSTICK_MANUAL)
const wsConnected = ref(false);
const throttle = ref(0);
const _sqrt = ref(0.707);

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
  // rotate 90 degrees for car coordinate
  const _temp = event.x;
  event.x = event.y;
  event.y = -_temp;

  // const data = { t: type, m: mode, x: event.x, y: event.y };
  const data = (type == JoystickMovementType.ROTATIONAL) ?
    { t: type, m: mode, z: event.z } :
    { t: type, m: mode, x: event.x, y: event.y };
  console.log(data);

  // return
  if (wsConnected.value) {
    api.sendWebSocketMessage(data);
  }
}

const sendButtonData = async (event, mode, type, throttle) => {
  // console.log({x: event.x, y: event.y});
  // rotate 90 degrees for car coordinate
  const _temp = event.x;
  event.x = event.y;
  event.y = -_temp;

  // error zero
  if (event.x === 0) {
    event.x = 0;
  }
  if (event.y === 0) {
    event.y = 0;
  }

  // send data
  const data = (type == JoystickMovementType.ROTATIONAL) ?
    { t: type, m: mode, z: event.z } :
    { t: type, m: mode, x: event.x, y: event.y };
  console.log(data);

  if (wsConnected.value) {
    api.sendWebSocketMessage(data);
  }
}

const sendAutoButtonData = async (mode, type) => {
  const distance = 0.5;
  const cooldown = 1;
  const step = 4;


  let { phi1_history, phi2_history, phi3_history, phi4_history } =
    pathGen.selectPath(type, distance, step);

  // console.log('before');
  // console.log(phi1_history);
  // console.log(phi2_history);
  // console.log(phi3_history);
  // console.log(phi4_history);

  phi1_history = phi1_history.map(i => i * 330 / (2 * Math.PI))
  phi2_history = phi2_history.map(i => i * 330 / (2 * Math.PI) * -1)
  phi3_history = phi3_history.map(i => i * 330 / (2 * Math.PI))
  phi4_history = phi4_history.map(i => i * 330 / (2 * Math.PI) * -1)

  for (let i = phi1_history.length - 1; i >= 0; i--) {
    phi1_history[i] = (phi1_history[i] - phi1_history[i - 1]).toFixed(0);
    phi2_history[i] = (phi2_history[i] - phi2_history[i - 1]).toFixed(0);
    phi3_history[i] = (phi3_history[i] - phi3_history[i - 1]).toFixed(0);
    phi4_history[i] = (phi4_history[i] - phi4_history[i - 1]).toFixed(0);
  }

  phi1_history.shift();
  phi2_history.shift();
  phi3_history.shift();
  phi4_history.shift();

  console.log('after');
  console.log(phi1_history);
  console.log(phi2_history);
  console.log(phi3_history);
  console.log(phi4_history);

  const cb = (iter) => {
    if (iter >= phi1_history.length) return;

    const data = JSON.stringify({
      m: mode,
      // t: type,
      // step: iter,
      p1: phi1_history[iter],
      p2: phi2_history[iter],
      p3: phi3_history[iter],
      p4: phi4_history[iter],
    });

    console.log(data);
    if (wsConnected.value) {
      api.sendWebSocketMessage(data);
    }

    setTimeout(() => cb(iter + 1), cooldown * 1000);
  }

  cb(1);
}

onMounted(async () => {
  api.connectWebSocket("/controls", () => {
    const handleOptions = async () => {
      try {
        const response = await api.options("/");
        console.log("CORS handled ok");
      } catch (error) {
        console.log("Error handling CORS: ", error);
      }
    }

    handleOptions();
    wsConnected.value = true;
  });
})

const closeWS = () => {
  api.closeWebSocket();
  wsConnected.value = false;
}

onBeforeUnmount(() => {
  if (wsConnected.value) {
    closeWS();
    wsConnected.value = false;
  }
  // <div class="pointer-events-auto">
  //   <Joystick :movement-type="JoystickMovementType.ROTATIONAL"
  //     @movement-changed="(event) => sendManualData(event, OperationMode.JOYSTICK_MANUAL, JoystickMovementType.ROTATIONAL)"
  //     @movement-ended="(event) => sendManualData(event, OperationMode.JOYSTICK_MANUAL, JoystickMovementType.ROTATIONAL)" />
  // </div>
})

</script>

<template>
  <div class="w-full h-screen flex flex-col">
    <HeaderComponent />
    <div class=" p-5 flex-grow flex flex-col gap-5 max-h-[50%]">
      <CameraView />
    </div>

    <div class="p-5 flex flex-row gap-x-2 w-full justify-center">
      <Dropdown button-label="Video Resolution" :items="AVAILABLE_RESOLUTIONS" title="Video Resolution"
        @select="(event) => changeResolution(event)" />
      <Dropdown button-label="JPEG Quality" :items="AVAILABLE_JPEG_QUALITY" title="Video Quality"
        @select="(event) => changeQuality(event)" />
      <Dropdown button-label="Operation Mode" :items="OPERATION_MODES" title="Operation mode"
        @select="(value) => operationMode = value" />
    </div>

    <div class="flex flex-row gap-x-5 w-full justify-center">
      <!-- <Slider :min="0" :max="3" :initial-value="0" name="Throttle"
        v-if="operationMode === OperationMode.BUTTONS_MANUAL || operationMode === OperationMode.BUTTONS_AUTO"
        @update:value="(event) => throttle = event" /> -->

      <div class="flex flex-row gap-x-16 pointer-events-none" v-if="operationMode === OperationMode.JOYSTICK_MANUAL">
        <div class="pointer-events-auto">
          <Joystick :movement-type="JoystickMovementType.OMNIDIRECTIONAL"
            @movement-changed="(event) => sendManualData(event, OperationMode.JOYSTICK_MANUAL, JoystickMovementType.OMNIDIRECTIONAL)"
            @movement-ended="(event) => sendManualData(event, OperationMode.JOYSTICK_MANUAL, JoystickMovementType.OMNIDIRECTIONAL)" />
          <IconButton icon="spin-left.svg" altText="Example Icon" icon-transform=""
            @click="sendManualData({ y: 0, x: 0, z: 1 }, OperationMode.JOYSTICK_MANUAL, JoystickMovementType.ROTATIONAL, throttle)" />
          <IconButton icon="spin-right.svg" altText="Example Icon" icon-transform=""
            @click="sendManualData({ y: 0, x: 0, z: -1 }, OperationMode.JOYSTICK_MANUAL, JoystickMovementType.ROTATIONAL, throttle)" />
        </div>
      </div>

      <div class="grid grid-cols-3 grid-rows-3 gap-4 w-52 h-52" v-if="operationMode === OperationMode.BUTTONS_MANUAL">
        <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(-45deg)"
          @click="sendButtonData({ x: -_sqrt, y: _sqrt }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />
        <IconButton icon="arrow.svg" altText="Example Icon"
          @click="sendButtonData({ x: 0, y: 1 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />
        <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(45deg)"
          @click="sendButtonData({ x: _sqrt, y: _sqrt }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />

        <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(-90deg)"
          @click="sendButtonData({ x: -1, y: 0 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />
        <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(90deg)" class="col-start-3 col-end-4"
          @click="sendButtonData({ x: 1, y: 0 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />

        <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(-135deg)"
          @click="sendButtonData({ x: -_sqrt, y: -_sqrt }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />
        <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(180deg)"
          @click="sendButtonData({ x: 0, y: -1 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />
        <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(135deg)"
          @click="sendButtonData({ x: _sqrt, y: -_sqrt }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)" />

        <IconButton icon="spin-left.svg" altText="Example Icon" icon-transform=""
          @click="sendButtonData({ y: 0, x: 0, z: 1 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.ROTATIONAL, throttle)" />
        <IconButton icon="spin-right.svg" altText="Example Icon" icon-transform=""
          @click="sendButtonData({ y: 0, x: 0, z: -1 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.ROTATIONAL, throttle)" />

        <button
          @click="sendButtonData({ x: 0, y: 0 }, OperationMode.BUTTONS_MANUAL, JoystickMovementType.OMNIDIRECTIONAL, throttle)">stop</button>
      </div>

      <div class="flex flex-col gap-y-3" v-if="operationMode === OperationMode.BUTTONS_AUTO">
        <Button text="Square with turn"
          @click="sendAutoButtonData(OperationMode.BUTTONS_AUTO, AutoOperationMode.SQUARE_WITH_TURN)" />
        <Button text="Square no turn"
          @click="sendAutoButtonData(OperationMode.BUTTONS_AUTO, AutoOperationMode.SQUARE_NO_TURN)" />
        <Button text="Circle with turn"
          @click="sendAutoButtonData(OperationMode.BUTTONS_AUTO, AutoOperationMode.CIRCLE_WITH_TURN)" />
        <Button text="Circle drift"
          @click="sendAutoButtonData(OperationMode.BUTTONS_AUTO, AutoOperationMode.CIRCLE_DRIFT)" />
        <Button text="Circle no turn"
          @click="sendAutoButtonData(OperationMode.BUTTONS_AUTO, AutoOperationMode.CIRCLE_NO_TURN)" />
      </div>

    </div>
  </div>
</template>