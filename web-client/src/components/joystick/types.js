const JoystickMovementType = Object.freeze({
  OMNIDIRECTIONAL: 0,
  ROTATIONAL: 1,
});

const AVAILABLE_KEYS = [
  ['w', 'a', 's', 'd'],
  ['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight']
]

const JOYSTICK_SIZE = 200;

const BASE_COLOR = "black";
const STICK_COLOR = "purple"; 

export { JoystickMovementType, JOYSTICK_SIZE, BASE_COLOR, STICK_COLOR, AVAILABLE_KEYS };
