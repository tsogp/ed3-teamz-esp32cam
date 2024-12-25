<template>
  <button
    class="inline-flex justify-center items-center cursor-pointer border-2 border-black text-black hover:bg-slate-50 rounded-lg p-2"
    @click="$emit('click')">
    <span>{{ text }}</span>
  </button>
</template>

<script setup>
import { defineProps, defineEmits } from 'vue';

const props = defineProps({
  text: {
    type: String,
    required: true,
  },
});

const emit = defineEmits(['click']);
</script>
