// TODO: implement loading current resolution from NVS
const AVAILABLE_RESOLUTIONS = [
  { name: "96X96", value: 0 },
  { name: "QQVGA (160x120)", value: 1 },
  { name: "128X128", value: 2 },
  { name: "QCIF (176x144)", value: 3 },
  { name: "HQVGA (240x176)", value: 4 },
  { name: "240X240", value: 5 },
  { name: "QVGA (320x240)", value: 6 },
  { name: "320X320", value: 7 },
  { name: "CIF (400x296)", value: 8 },
  { name: "HVGA (480x320)", value: 9 },
  { name: "VGA (640x480)", value: 10 },
  { name: "SVGA (800x600)", value: 11 },
  { name: "XGA (1024x768)", value: 12 },
  { name: "HD (1280x720)", value: 13 },
  { name: "SXGA (1280x1024)", value: 14 },
  { name: "UXGA (1600x1200)", value: 15 },
];

const AVAILABLE_JPEG_QUALITY = Array.from({ length: 54 }, (_, i) => {
  const quality = i + 10;
  return { name: quality, value: quality };
});

const OperationMode = Object.freeze({
  BUTTONS_MANUAL: 1,
  JOYSTICK_MANUAL: 0,
  BUTTONS_AUTO: 2,
});

const AutoOperationMode = Object.freeze({
  SQUARE_WITH_TURN: 0,
  CIRCLE_WITH_TURN: 1,
  SQUARE_NO_TURN: 2,
  CIRCLE_DRIFT: 3,
});

const OPERATION_MODES = [
  { name: "Joystick Manual", value: OperationMode.JOYSTICK_MANUAL },
  { name: "Buttons Manual", value: OperationMode.BUTTONS_MANUAL },
  { name: "Buttons Auto", value: OperationMode.BUTTONS_AUTO },
];

export {
  AVAILABLE_RESOLUTIONS,
  AVAILABLE_JPEG_QUALITY,
  OPERATION_MODES,
  OperationMode,
  AutoOperationMode,
};
