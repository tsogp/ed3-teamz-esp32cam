<script setup>
import CameraView from './views/camera-view/CameraView.vue'
import Dropdown from './components/dropdown/Dropdown.vue';
import { AVAILABLE_RESOLUTIONS, AVAILABLE_JPEG_QUALITY } from './types';
import api from './services/generic-api';
import { onMounted } from 'vue';
import Slider from './components/slider/Slider.vue';
import IconButton from './components/icon-button/IconButton.vue';
import Button from './components/button/Button.vue';

const changeResolution = async (resId) => {
  try {
    const response = await api.post("/reload-camera-resolution", { rs: resId });
    console.log("Changing camera resolution OK");
  } catch (error) {
    console.log("Error changing camera resolution: ", error);
  }
}

const changeQuality = async (qId) => {
  try {
    const response = await api.post("/reload-camera-quality", { q: qId });
    console.log("Changing camera resolution OK");
  } catch (error) {
    console.log("Error changing camera resolution: ", error);
  }
}

onMounted(async () => {
  try {
    const response = await api.options("");
    console.log("CORS handled ok");
  } catch (error) {
    console.log("Error handling CORS: ", error);
  }
})

</script>

<template>
  <div class="flex flex-row p-5 gap-x-2">
    <div class="flex flex-row gap-x-2">
      <Dropdown button-label="Video Resolution" :items="AVAILABLE_RESOLUTIONS" title="Choose the resolution"
        @select="(event) => changeResolution(event)" />
      <Dropdown button-label="JPEG Quality" :items="AVAILABLE_JPEG_QUALITY" title="Choose the quality"
        @select="(event) => changeQuality(event)" />
    </div>
    <CameraView class="" />
    <Slider :min="0" :max="3" :initial-value="0" name="Throttle" />
    <div class="grid grid-cols-3 grid-rows-3 gap-4 w-52 h-52">
      <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(-45deg)" />
      <IconButton icon="arrow.svg" altText="Example Icon" />
      <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(45deg)" />
      <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(-90deg)" />
      <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(90deg)"
        class="col-start-3 col-end-4" />
      <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(-135deg)" />
      <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(180deg)" />
      <IconButton icon="arrow.svg" altText="Example Icon" icon-transform="rotate(135deg)" />
    </div>
    <div class="flex flex-col gap-y-3">
      <Button text="Square with turn" />
      <Button text="Circle with turn" />
      <Button text="Square no turn" />
      <Button text="Circle drift" />
    </div>
  </div>
</template>