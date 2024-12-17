<template>
  <div class="flex flex-col items-center h-64 mb-4">
    <!-- Slider Container -->
    <div ref="slider" class="relative w-12 h-full bg-gray-200 rounded-lg flex items-end overflow-hidden p-2"
      @mousedown="startDrag" @touchstart.prevent="startDrag">
      <!-- Slider Handle -->
      <div class="absolute left-1/2 transform -translate-x-1/2 bg-blue-500 rounded-lg w-8 h-8 cursor-pointer shadow-lg"
        :style="{ bottom: sliderPosition + '%' }"></div>
    </div>

    <!-- Value Display -->
    <div class="mt-4 text-lg font-semibold">{{ name }}: {{ value }}</div>
  </div>
</template>

<script setup>
import { ref, defineProps, defineEmits, watch } from "vue";

const props = defineProps({
  initialValue: { type: Number, default: 0 }, // Starting value
  name: { type: String, default: "Value" },
  step: { type: Number, default: 1 }, // Step increment
  max: { type: Number, default: 100 }, // Maximum value
  min: { type: Number, default: 0 }, // Minimum value
});

const emit = defineEmits(["update:value"]);

const slider = ref(null);
const value = ref(props.initialValue); // Current slider value
const sliderPosition = ref(((props.initialValue - props.min) / (props.max - props.min)) * 74 + 5);
let isDragging = false;

// Watch for external updates to `value`
watch(
  () => props.initialValue,
  (newVal) => {
    value.value = newVal;
    sliderPosition.value = ((newVal - props.min) / (props.max - props.min)) * 74 + 5;
  }
);

// Emit value change
const updateValue = () => {
  emit("update:value", value.value);
};

// Start dragging
const startDrag = (e) => {
  isDragging = true;
  document.addEventListener("mousemove", onDrag);
  document.addEventListener("mouseup", stopDrag);
  document.addEventListener("touchmove", onDrag);
  document.addEventListener("touchend", stopDrag);
  onDrag(e);
};

// Drag handler with stepped increments
const onDrag = (e) => {
  if (!isDragging) return;

  const sliderRect = slider.value.getBoundingClientRect();
  const clientY = e.clientY || e.touches[0]?.clientY;

  let position = ((sliderRect.bottom - clientY) / (sliderRect.height - 16)) * 100; // Adjust for padding
  position = Math.min(80, Math.max(5, position)); // Clamp position between 5 and 80

  // Calculate stepped value
  const stepCount = Math.round(((position - 5) / 74) * ((props.max - props.min) / props.step));
  const rangeValue = stepCount * props.step + props.min;

  // Update value and slider position
  value.value = Math.min(props.max, Math.max(props.min, rangeValue));
  sliderPosition.value = ((value.value - props.min) / (props.max - props.min)) * 74 + 5;
  updateValue();
};

// Stop dragging
const stopDrag = () => {
  isDragging = false;
  document.removeEventListener("mousemove", onDrag);
  document.removeEventListener("mouseup", stopDrag);
  document.removeEventListener("touchmove", onDrag);
  document.removeEventListener("touchend", stopDrag);
};
</script>

<style scoped>
.relative {
  padding: 0.5rem;
  /* Add padding inside the slider */
}
</style>