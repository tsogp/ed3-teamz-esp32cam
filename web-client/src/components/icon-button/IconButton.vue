<template>
  <button 
    class="inline-flex justify-center items-center cursor-pointer border-2 border-black hover:bg-slate-50 rounded-lg p-2" 
    @click="$emit('click')">
    <img :src="icon" :alt="altText" class="h-full w-full" :style="iconStyles" />
  </button>
</template>

<script setup>
import { computed } from 'vue';

// Define the props
const props = defineProps({
  icon: {
    type: String,
    required: true,
  },
  altText: {
    type: String,
    default: 'Icon',
  },
  width: {
    type: String,
    default: '24px',
  },
  height: {
    type: String,
    default: '24px',
  },
  iconTransform: {
    type: String,
    default: '',
  }
});

// Emit click event
const emit = defineEmits(['click']);

// Computed property for icon styles
const iconStyles = computed(() => {
  const styles = {
    width: props.width,
    height: props.height,
  };

  if (props.iconTransform) {
    styles.transform = props.iconTransform;
  }

  return styles;
});
</script>
