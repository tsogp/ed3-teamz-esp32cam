<script setup>
import CameraView from './views/camera-view/CameraView.vue'
import Dropdown from './components/dropdown/Dropdown.vue';
import { AVAILABLE_RESOLUTIONS } from './types';
import api from './services/generic-api';

const changeResolution = async (resId) => {
  try {
    const response = await api.post("/reload-camera-config", {rs: resId});
    console.log("Changing camera resolution OK");
  } catch (error) {
    console.log("Error changing camera resolution: ", error);
  }
}

</script>

<template>
  <div class="flex flex-col">
    <Dropdown button-label="Video Quality" :items="AVAILABLE_RESOLUTIONS" title="Choose the resolution" @select="(event) => changeResolution(event.value)" />
    <CameraView class="" />
  </div>
</template>