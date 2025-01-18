const JoystickMovementType = Object.freeze({
  OMNIDIRECTIONAL: 0,
  ROTATIONAL: 1,
});

const JoystickMovementStatus = Object.freeze({
  MOVING: 0,
  STOPPED: 1,
});

const AVAILABLE_KEYS = [
  ["w", "a", "s", "d"],
  ["ArrowUp", "ArrowDown", "ArrowLeft", "ArrowRight"],
];

const JOYSTICK_SIZE = 200;

const BASE_COLOR = "#007ee3";
const STICK_COLOR = "#111827";

export {
  JoystickMovementType,
  JoystickMovementStatus,
  JOYSTICK_SIZE,
  BASE_COLOR,
  STICK_COLOR,
  AVAILABLE_KEYS,
};
