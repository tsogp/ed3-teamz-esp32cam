<template>
  <Joystick ref="joystick" :size="JOYSTICK_SIZE" :stick-size="JOYSTICK_SIZE / 2" :base-color="BASE_COLOR"
    :stick-color="STICK_COLOR" :throttle="100"
    @stop="$emit('movementChanged', { x: event?.x, y: event?.y })"
    @move="$emit('movementEnded', { x: event?.x, y: event?.y })" />
</template>

<script setup>
import { ref, onMounted, onBeforeUnmount, useTemplateRef, watch } from 'vue';
import { Joystick } from 'vue-joystick-component';
import { JoystickMovementType, JOYSTICK_SIZE, BASE_COLOR, STICK_COLOR } from './types';

const props = defineProps({
  movementType: JoystickMovementType
});

const emit = defineEmits([
  "movementChanged",
  "movementEnded"
])

const keysPressed = new Set();

const movementType = ref(props.movementType);
const joystickRef = useTemplateRef("joystick");
const joystickEl = ref(null);

const handleKeyDown = (event) => {
  keysPressed.add(event.key);

  if (movementType.value === JoystickMovementType.OMNIDIRECTIONAL) {
    if (keysPressed.has('w') && keysPressed.has('d')) {
      updateButtonPosition((JOYSTICK_SIZE / 2), -(JOYSTICK_SIZE / 2));
    } else if (keysPressed.has('w') && keysPressed.has('a')) {
      updateButtonPosition(-(JOYSTICK_SIZE / 2), -(JOYSTICK_SIZE / 2));
    } else if (keysPressed.has('s') && keysPressed.has('d')) {
      updateButtonPosition((JOYSTICK_SIZE / 2), (JOYSTICK_SIZE / 2));
    } else if (keysPressed.has('s') && keysPressed.has('a')) {
      updateButtonPosition(-(JOYSTICK_SIZE / 2), (JOYSTICK_SIZE / 2));
    } else {
      if (keysPressed.has('w')) updateButtonPosition(0, -(JOYSTICK_SIZE / 2));
      else if (keysPressed.has('s')) updateButtonPosition(0, (JOYSTICK_SIZE / 2));
      else if (keysPressed.has('a')) updateButtonPosition(-(JOYSTICK_SIZE / 2), 0);
      else if (keysPressed.has('d')) updateButtonPosition((JOYSTICK_SIZE / 2), 0);
    }
  } else if (movementType.value === JoystickMovementType.ROTATIONAL) {
    if (keysPressed.has('ArrowUp') && keysPressed.has('ArrowRight')) {
      updateButtonPosition((JOYSTICK_SIZE / 2), -(JOYSTICK_SIZE / 2));
    } else if (keysPressed.has('ArrowUp') && keysPressed.has('ArrowLeft')) {
      updateButtonPosition(-(JOYSTICK_SIZE / 2), -(JOYSTICK_SIZE / 2));
    } else if (keysPressed.has('ArrowDown') && keysPressed.has('ArrowRight')) {
      updateButtonPosition((JOYSTICK_SIZE / 2), (JOYSTICK_SIZE / 2));
    } else if (keysPressed.has('ArrowDown') && keysPressed.has('ArrowLeft')) {
      updateButtonPosition(-(JOYSTICK_SIZE / 2), (JOYSTICK_SIZE / 2));
    } else {
      if (keysPressed.has('ArrowUp')) updateButtonPosition(0, -(JOYSTICK_SIZE / 2));
      else if (keysPressed.has('ArrowDown')) updateButtonPosition(0, (JOYSTICK_SIZE / 2));
      else if (keysPressed.has('ArrowLeft')) updateButtonPosition(-(JOYSTICK_SIZE / 2), 0);
      else if (keysPressed.has('ArrowRight')) updateButtonPosition((JOYSTICK_SIZE / 2), 0);
    }
  }
};

const normalizeKeyboardDirection = (x, y) => {
  return { x: x / (JOYSTICK_SIZE / 2), y: y / (JOYSTICK_SIZE / 2) };
}

const handleKeyUp = (event) => {
  keysPressed.delete(event.key);

  if (keysPressed.size === 0) {
    joystickEl.value.style.transform = "";
  }

  emit("movementEnded", { x: 0, y: 0 });
}

const updateButtonPosition = (deltaX, deltaY) => {
  if (!joystickEl.value) {
    return;
  }

  joystickEl.value.style.transform = `translate(${deltaX}px, ${deltaY}px)`;

  emit("movementChanged", normalizeKeyboardDirection(deltaX, deltaY));
};

watch(joystickRef, (newValue) => {
  joystickEl.value = newValue.$el.querySelector('button')
});

onMounted(() => {
  window.addEventListener('keydown', handleKeyDown);
  window.addEventListener('keyup', handleKeyUp);
});

onBeforeUnmount(() => {
  window.removeEventListener('keydown', handleKeyDown);
  window.removeEventListener('keyup', handleKeyUp);
});

</script>