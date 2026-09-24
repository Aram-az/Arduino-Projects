# Anxiety Support Prototype and Live Dashboard

**ELEC 290 · Queen's University · Team 07 · 2025**

A team-built, Arduino-based proof of concept that reads a galvanic skin response (GSR) sensor and an MPU6050 motion sensor, assigns a simple signal severity level, activates a vibration motor and LED breathing sequence when motion crosses a threshold, and displays incoming measurements in a browser dashboard.

> This is a classroom prototype. Its thresholds are heuristic and have not been clinically validated to detect or diagnose anxiety. It is not a medical device or an emergency response system.

## Demo

Add a photograph of the breadboard prototype and a screenshot of the dashboard here when available. The dashboard's **Trigger Demo Episode** button creates a simulated event in the browser; it does not trigger the physical motor or represent a sensor measurement.

## System overview

```text
GSR sensor (A0) ─┐
                 ├─> Arduino sketch ── JSON over USB serial ──> Node.js ── WebSocket ──> browser dashboard
MPU6050 (I2C) ──┘        │
                         ├─> vibration motor through relay
                         └─> three LEDs for guided breathing
```

- **Arduino:** Samples GSR, applies fast and slow exponential moving averages, and uses their difference as a scaled GSR change signal. It reads acceleration from the MPU6050, removes an approximate gravity magnitude and a 20-sample moving average, then smooths the remaining movement signal.
- **Severity:** The sketch sets level `2` when a strong GSR change and motion threshold occur together; level `1` when either a strong GSR change, a moderate GSR change, or the motion threshold occurs; otherwise level `0`. The current thresholds are hard-coded and require calibration for a particular setup.
- **Physical response:** When the motion threshold is exceeded, the sketch switches the relay on for three seconds and runs up to ten LED breathing cycles. A button can stop the sequence early after a cycle. These blocking delays can interrupt sampling and dashboard updates during a response.
- **Dashboard:** A Node.js bridge parses newline-delimited JSON from the serial port and broadcasts it on a local WebSocket. The browser shows the GSR trace, latest reported severity, and recent samples. A high-severity sample opens a confirmation prompt with breathing instructions. The dashboard's “Events” count currently counts received samples, not distinct episodes.

The serial message format is:

```json
{"t":12345,"gsr":18.42,"tremor":0,"severity":1}
```

`t` is milliseconds since the Arduino started; `gsr` is a scaled change signal, not a calibrated skin-conductance unit.

## Repository layout

```text
Anxiety-Dashboard/
├── README.md
├── main/
│   └── main.ino
├── Node-Server/
│   ├── package.json
│   ├── package-lock.json
│   └── server.js
└── Web/
    ├── index.html
    ├── app.js
    └── styles.css
```

## Hardware

The sketch uses an Arduino-compatible board with `Wire` support, an MPU6050/GY-521 sensor at I2C address `0x68`, a GSR sensor connected to analog input `A0`, three LEDs with suitable current-limiting resistors, a push button, and a relay-driven vibration motor. Check the relay and motor power requirements before connecting them to the board.

| Connection | Sketch pin |
| --- | --- |
| Green LED | Digital 2 |
| Yellow LED | Digital 3 |
| Red LED | Digital 4 |
| Motor relay control | Digital 5 |
| Button (`INPUT_PULLUP`; connect to ground when pressed) | Digital 6 |
| GSR output | Analog A0 |
| MPU6050 SDA/SCL | Board's I2C SDA/SCL pins |

The sketch selects `analogReference(INTERNAL)` with a 1.1 V reference comment. **Verify the actual reference and GSR signal voltage for your specific board before wiring and uploading.** The archive does not provide a complete wiring schematic or identify the exact board model.

## Run with hardware

1. Install the Arduino IDE and select the correct board and port. Open `main/main.ino` and upload it. The sketch uses the IDE's built-in `Wire` library and prints serial data at **115200 baud**.
2. Close the Arduino Serial Monitor and any other program using the serial port.
3. Install a current Node.js version. In `Node-Server/server.js`, set `ARDUINO_PORT` to your board's port (the checked-in value is `COM3`). On macOS/Linux, use the appropriate `/dev/...` path. Leave `FAKE_MODE = false`.
4. In a terminal, run:

   ```bash
   cd Anxiety-Dashboard/Node-Server
   npm ci
   node server.js
   ```

   The bridge listens at `ws://localhost:8080`. Confirm the terminal reports that the serial port opened.
5. Open `Anxiety-Dashboard/Web/index.html` in a browser. The dashboard connects to the WebSocket on the same computer. It loads Chart.js from a CDN, so the chart requires an internet connection unless that dependency is made local.

## Run without hardware

1. Change `FAKE_MODE` at the top of `Node-Server/server.js` to `true`.
2. From `Anxiety-Dashboard/Node-Server`, run `npm ci` followed by `node server.js`.
3. Open `Anxiety-Dashboard/Web/index.html`. The server will broadcast simulated values. You can also click **Trigger Demo Episode** to show the confirmation overlay. Neither mode validates sensor performance.

## Team and contributions

This was an ELEC 290 Team 07 project. **Before publication, add teammate credits and specify which parts you personally designed, implemented, tested, or integrated.** The repository contains team code; the author's individual contribution cannot be established reliably from the archive alone.

## Limitations and possible improvements

- Validate and calibrate sensor thresholds with appropriate data; motion and GSR changes can have many causes besides anxiety.
- Replace the blocking motor/LED delays with a timed state machine so sampling and serial output remain responsive.
- Check MPU6050 I2C read availability and handle disconnected or malformed sensor data.
- Distinguish dashboard sample count from actual episodes; validate incoming WebSocket messages before rendering them.
- Make the serial port and demo mode configurable without editing source, and add a complete board-specific wiring diagram.

No personal physiological recordings are included in this repository.
