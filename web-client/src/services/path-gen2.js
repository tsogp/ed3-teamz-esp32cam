// Constants
const l = 0.5; // Distance between wheel pairs (m)
const d = 0.5; // Distance between wheels along the axis (m)
const wheelR = 0.097;

// Inverse kinematics matrix
const J_inv = [
  [1, -1, -(l + d)],
  [1, 1, (l + d)],
  [1, 1, -(l + d)],
  [1, -1, (l + d)]
];

// Histories
let time_history = [0];
let x_history = [0];
let y_history = [0];
let phi_history = [0];

let v1_history = [0];
let v2_history = [0];
let v3_history = [0];
let v4_history = [0];
let vCarX_history = [0];
let vCarY_history = [0];
let vCarPhi_history = [0];

let phi1_history = [0];
let phi2_history = [0];
let phi3_history = [0];
let phi4_history = [0];

class PathGen {
  constructor() {
  }

  start_move(distance, angleMove, angleSpin, time, step) {
    const dt = time / step;
    const time_array = Array.from({ length: step }, (_, i) => i * dt);
    const time_history_last = time_history[time_history.length - 1];
    time_history = time_history.concat(time_array.slice(1).map(t => +(t + time_history_last).toFixed(2)));

    angleMove = (Math.PI / 180) * angleMove;
    angleSpin = (Math.PI / 180) * angleSpin;

    const vCarX = (distance * Math.cos(angleMove)) / time;
    const vCarY = (distance * Math.sin(angleMove)) / time;
    const vCarPhi = angleSpin / time;

    const v1 = (1 / wheelR) * (vCarX - vCarY - (l + d) * vCarPhi);
    const v2 = (1 / wheelR) * (vCarX + vCarY + (l + d) * vCarPhi);
    const v3 = (1 / wheelR) * (vCarX + vCarY - (l + d) * vCarPhi);
    const v4 = (1 / wheelR) * (vCarX - vCarY + (l + d) * vCarPhi);

    console.log(`vCarX: ${vCarX}, vCarY: ${vCarY}, vCarPhi: ${vCarPhi}`);

    for (let i = 0; i < step; i++) {
      const dPhi = vCarPhi * dt;
      const phi = phi_history[phi_history.length - 1] + dPhi;

      const x_pos = x_history[x_history.length - 1] + (vCarX * Math.cos(phi) - vCarY * Math.sin(phi)) * dt;
      const y_pos = y_history[y_history.length - 1] + (vCarX * Math.sin(phi) + vCarY * Math.cos(phi)) * dt;

      const phi1_pos = phi1_history[phi1_history.length - 1] + v1 * dt;
      const phi2_pos = phi2_history[phi2_history.length - 1] + v2 * dt;
      const phi3_pos = phi3_history[phi3_history.length - 1] + v3 * dt;
      const phi4_pos = phi4_history[phi4_history.length - 1] + v4 * dt;

      v1_history.push(v1);
      v2_history.push(v2);
      v3_history.push(v3);
      v4_history.push(v4);

      phi1_history.push(phi1_pos);
      phi2_history.push(phi2_pos);
      phi3_history.push(phi3_pos);
      phi4_history.push(phi4_pos);

      vCarX_history.push(vCarX);
      vCarY_history.push(vCarY);
      vCarPhi_history.push(vCarPhi);

      x_history.push(x_pos);
      y_history.push(y_pos);
      phi_history.push(phi);
    }
  }

  square_with_turn(distance, timePerOp, stepPerOp) {
    // start_move(distance, 0, 0, timePerOp, stepPerOp);
    for (let i = 0; i < 4; i++) {
      // Move forward
      this.start_move(distance, 60, 0, timePerOp, stepPerOp);
      // Turn
      this.start_move(distance, 0, 90, timePerOp, stepPerOp);
    }

  }

  square_no_turn(distance, timePerOp, stepPerOp) {
    for (let i = 0; i < 4; i++) {
      this.start_move(distance, 90 * i, 0, timePerOp, stepPerOp);
    }
  }

  circle_with_turn(radius, timePerOp, stepPerOp) {
    this.start_move(2 * Math.PI * radius, 0, 360, timePerOp, stepPerOp);
  }

  circle_no_turn(radius, timePerOp, stepPerOp) {
    this.start_move(2 * Math.PI * radius, 0, 0, timePerOp, stepPerOp);
  }

  circle_center_turn(radius, timePerOp, stepPerOp) {
    this.start_move(2 * Math.PI * radius, 0, 360, timePerOp, stepPerOp);
  }

  printPositions(step, signalDelay) {
    if (step < phi1_history.length) {
      console.log(`Step ${step}:`);

      const test = this.convertHistoryToJSON(phi1_history[step], phi2_history[step], phi3_history[step], phi4_history[step]);
      console.log(test);

      setTimeout(() => this.printPositions(step + 1, signalDelay), signalDelay * 1000);
    }
  }

  convertHistoryToJSON(phi1, phi2, phi3, phi4) {
    phi1 = phi1.toFixed(2);
    phi2 = phi2.toFixed(2);
    phi3 = phi3.toFixed(2);
    phi4 = phi4.toFixed(2);
    // Construct an object with the provided values
    const historyObject = {
      phi1: phi1,
      phi2: phi2,
      phi3: phi3,
      phi4: phi4,
    };

    // Convert the object into a JSON string
    return JSON.stringify(historyObject, null, 2); // Pretty format with 2 spaces
  }

  selectPath(distance, signalDelay, stepPerOp, option) {
    time_history = [0];
    x_history = [0];
    y_history = [0];
    phi_history = [0];

    v1_history = [0];
    v2_history = [0];
    v3_history = [0];
    v4_history = [0];
    vCarX_history = [0];
    vCarY_history = [0];
    vCarPhi_history = [0];

    phi1_history = [0];
    phi2_history = [0];
    phi3_history = [0];
    phi4_history = [0];

    switch (option) {
      case 0:
        this.square_with_turn(distance, 5, stepPerOp);
        this.printPositions(1, signalDelay);
        break;
      case 1:
        this.square_no_turn(distance, 5, stepPerOp);
        this.printPositions(1, signalDelay);
        break;
      case 2:
        this.circle_with_turn(distance, 5, stepPerOp);
        this.printPositions(1, signalDelay);
        break;
      case 3:
        this.circle_no_turn(distance, 5, stepPerOp);
        this.printPositions(1, signalDelay);
        break;
      case 4:
        this.circle_center_turn(distance, 5, stepPerOp);
        this.printPositions(1, signalDelay);
        break;
      default:
        console.log('Invalid option');
    }
  }
}

const pathGen = new PathGen();

export default pathGen;