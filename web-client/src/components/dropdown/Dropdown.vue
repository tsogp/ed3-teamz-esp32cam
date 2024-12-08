<template>
  <div class="flex flex-col gap-y-2">
    <span class="text-sm text-gray-600">{{ title + ":" }}</span>
    <div class="relative inline-block">
      <button
        @click="toggleDropdown"
        class="border border-gray-300 px-4 py-2 hover:bg-gray-100 focus:outline-none w-48 text-left flex justify-between items-center"
      >
        <span>{{ chosenItem?.name ?? "Choose an item" }}</span>
        <svg
          xmlns="http://www.w3.org/2000/svg"
          class="h-4 w-4 text-gray-500"
          fill="none"
          viewBox="0 0 24 24"
          stroke="currentColor"
          stroke-width="2"
        >
          <path
            stroke-linecap="round"
            stroke-linejoin="round"
            d="M19 9l-7 7-7-7"
          />
        </svg>
      </button>
  
      <ul
        v-if="isOpen"
        class="absolute bg-white border border-gray-300 mt-2 shadow-lg w-48 z-10"
      >
        <li
          v-for="(item, index) in items"
          :key="index"
          class="px-4 py-2 hover:bg-gray-100 cursor-pointer"
          @click="selectItem(item)"
        >
          {{ item.name }}
        </li>
      </ul>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue';

const props = defineProps({
  items: {
    type: Array,
    required: true,
  },
  buttonLabel: {
    type: String,
    default: 'Select',
  },
  title: {
    type: String,
    required: true,
  }
});

const emit = defineEmits(['select']);
const chosenItem = ref(props.items[0]);
const isOpen = ref(false);

const toggleDropdown = () => {
  isOpen.value = !isOpen.value;
};

const selectItem = (item) => {
  isOpen.value = false;
  chosenItem.value = item;
  emit('select', item.value);
};
</script>
