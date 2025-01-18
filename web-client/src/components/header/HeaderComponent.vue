<template>
  <header class="flex justify-between items-center p-4 bg-[#007ee3]">
    <div class="logo cursor-pointer">
      <div class="text-white font-bold">Library Mecanum Car</div>
    </div>
    <button class="px-4 py-2 text-[#007ee3] bg-white rounded-lg hover:bg-[#ebf3fa]" @click="handleClick">
      About Us
    </button>
  </header>
</template>

<script setup>
const handleClick = () => {
  alert('Button clicked!');
};
</script>
