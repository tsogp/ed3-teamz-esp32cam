<template>
  <Joystick ref="joystick" :size="JOYSTICK_SIZE" :stick-size="JOYSTICK_SIZE / 2" :base-color="BASE_COLOR"
    :stick-color="STICK_COLOR" :throttle="100"
    @stop="$emit('movementEnded', { x: Number(($event?.x ?? 0).toFixed(3)), y: Number(($event?.y ?? 0).toFixed(3)) })"
    @move="$emit('movementChanged', { x: Number(($event?.x ?? 0).toFixed(3)), y: Number(($event?.y ?? 0).toFixed(3)) })" />
</template>

<script setup>
import { ref, onMounted, onBeforeUnmount, useTemplateRef, watch } from 'vue';
import { Joystick } from 'vue-joystick-component';
import { JoystickMovementType, JOYSTICK_SIZE, BASE_COLOR, STICK_COLOR, AVAILABLE_KEYS } from './types';

const props = defineProps({
  movementType: JoystickMovementType
});

const emit = defineEmits([
  "movementChanged",
  "movementEnded"
])

const keysPressed = ref(new Set());

const movementType = ref(props.movementType);
const joystickRef = useTemplateRef("joystick");
const joystickEl = ref(null);

const handleKeyDown = (event) => {
  if (!AVAILABLE_KEYS[props.movementType].includes(event.key) || keysPressed.value.has(event.key)) {
    return;
  }

  keysPressed.value.add(event.key);
};

const normalizeKeyboardDirection = (x, y) => {
  const magnitude = Math.sqrt(x * x + y * y);

  if (magnitude === 0) {
    return { x: 0, y: 0 };
  }

  const normalizedX = x / magnitude;
  const normalizedY = y / magnitude;

  return { x: normalizedX.toFixed(3), y: normalizedY.toFixed(3) };
}

const handleKeyUp = (event) => {
  if (!keysPressed.value.has(event.key)) {
    return;
  }

  keysPressed.value.delete(event.key);
  if (keysPressed.value.size === 0) {
    joystickEl.value.style.transform = "";
    emit("movementEnded", { x: 0, y: 0 });
  }
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

watch(keysPressed.value, (newValue) => {
  if (movementType.value === JoystickMovementType.OMNIDIRECTIONAL) {
    if (keysPressed.value.has('w') && keysPressed.value.has('d')) {
      updateButtonPosition((JOYSTICK_SIZE / 2), -(JOYSTICK_SIZE / 2));
    } else if (keysPressed.value.has('w') && keysPressed.value.has('a')) {
      updateButtonPosition(-(JOYSTICK_SIZE / 2), -(JOYSTICK_SIZE / 2));
    } else if (keysPressed.value.has('s') && keysPressed.value.has('d')) {
      updateButtonPosition((JOYSTICK_SIZE / 2), (JOYSTICK_SIZE / 2));
    } else if (keysPressed.value.has('s') && keysPressed.value.has('a')) {
      updateButtonPosition(-(JOYSTICK_SIZE / 2), (JOYSTICK_SIZE / 2));
    } else {
      if (keysPressed.value.has('w')) updateButtonPosition(0, -(JOYSTICK_SIZE / 2));
      else if (keysPressed.value.has('s')) updateButtonPosition(0, (JOYSTICK_SIZE / 2));
      else if (keysPressed.value.has('a')) updateButtonPosition(-(JOYSTICK_SIZE / 2), 0);
      else if (keysPressed.value.has('d')) updateButtonPosition((JOYSTICK_SIZE / 2), 0);
    }
  } else if (movementType.value === JoystickMovementType.ROTATIONAL) {
    if (keysPressed.value.has('ArrowUp') && keysPressed.value.has('ArrowRight')) {
      updateButtonPosition((JOYSTICK_SIZE / 2), -(JOYSTICK_SIZE / 2));
    } else if (keysPressed.value.has('ArrowUp') && keysPressed.value.has('ArrowLeft')) {
      updateButtonPosition(-(JOYSTICK_SIZE / 2), -(JOYSTICK_SIZE / 2));
    } else if (keysPressed.value.has('ArrowDown') && keysPressed.value.has('ArrowRight')) {
      updateButtonPosition((JOYSTICK_SIZE / 2), (JOYSTICK_SIZE / 2));
    } else if (keysPressed.value.has('ArrowDown') && keysPressed.value.has('ArrowLeft')) {
      updateButtonPosition(-(JOYSTICK_SIZE / 2), (JOYSTICK_SIZE / 2));
    } else {
      if (keysPressed.value.has('ArrowUp')) updateButtonPosition(0, -(JOYSTICK_SIZE / 2));
      else if (keysPressed.value.has('ArrowDown')) updateButtonPosition(0, (JOYSTICK_SIZE / 2));
      else if (keysPressed.value.has('ArrowLeft')) updateButtonPosition(-(JOYSTICK_SIZE / 2), 0);
      else if (keysPressed.value.has('ArrowRight')) updateButtonPosition((JOYSTICK_SIZE / 2), 0);
    }
  }
})

onMounted(() => {
  window.addEventListener('keydown', handleKeyDown);
  window.addEventListener('keyup', handleKeyUp);
});

onBeforeUnmount(() => {
  window.removeEventListener('keydown', handleKeyDown);
  window.removeEventListener('keyup', handleKeyUp);
});

</script>