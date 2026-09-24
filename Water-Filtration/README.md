# Automated Water Filtration Prototype

**Queen's University · Five-person team project · Arduino Mega**

An Arduino prototype that coordinates pumps, a servo mechanism, a liquid-level input, and two analog turbidity sensors for a water treatment demonstration. The sketch provides start and stop buttons, prints sensor readings to the serial monitor, and runs timed actuator behaviours.

> **Prototype status:** This code demonstrates component integration and basic sequencing. It has not been verified here with the original hardware. It does not calculate water quality, dose alum based on turbidity, or establish that the treated water is safe to drink.

## What the sketch currently does

- After the start button on pin 35 is pressed, reads two analog turbidity inputs (`A0`, `A1`) and prints their estimated voltages approximately once per `tur()` update.
- Reads a digital liquid-level input on pin 52. A `HIGH` reading runs the pump on motor-shield channel M2. A `LOW` reading stops M2, cycles a servo between 40° and 125° up to four times, and runs the pump on channel M4.
- Runs a motor on channel M1 when the sketch's `seconds` counter is greater than 105 and less than 220.
- On a rising press of the stop button on pin 31, releases all three DC motors, moves the servo to 40°, and halts further operation until reset.

The `seconds` value counts one-second intervals **when `tur()` is reached**; long `delay()` calls in the level-sensor and servo routines can make it diverge from actual elapsed time. The stop button cannot interrupt those blocking routines immediately.

## Hardware and connections

The sketch uses an **Arduino Mega**, an Adafruit Motor Shield compatible with the `AFMotor` library, a servo, two analog turbidity sensor outputs, and a digital liquid-level sensor. Verify the shield version and motor power wiring before connecting pumps.

| Component          | Connection in sketch                          |
| ------------------ | --------------------------------------------- |
| Turbidity sensor 1 | `A0`                                          |
| Turbidity sensor 2 | `A1`                                          |
| Liquid-level input | Digital `52`, `INPUT_PULLUP`                  |
| Start button       | Digital `35`, `INPUT_PULLUP`; pressed = `LOW` |
| Stop button        | Digital `31`, `INPUT_PULLUP`; pressed = `LOW` |
| Servo              | Digital `9`                                   |
| Pump 1             | Motor shield M2 (`cfpump`)                    |
| Pump 2             | Motor shield M4 (`pump`)                      |
| Additional motor   | Motor shield M1 (`motor`)                     |

The original wiring diagram, exact sensor models, power supply, and mechanical design are not included with this sketch. Check that pin 9 is available for a servo on your particular motor-shield setup.

## Build and inspect

1. Save the Arduino source as `Water-Filtration/Water-Filtration.ino` so the sketch filename matches its containing folder.
2. Install the Arduino IDE and the **AFMotor** library compatible with the team's motor shield. The `Servo` library is also required.
3. Select **Arduino Mega or Mega 2560** and the appropriate port, then compile the sketch. Confirm the shield and board combination works before connecting pumps or a motor.
4. Set the Serial Monitor to **9600 baud**. Press the start button to begin; use the stop button to halt after the current blocking operation finishes.

Compilation and operation on the original hardware have not been independently verified for this published version.

## Known limitations and next steps

- The turbidity values are displayed as voltages only. The sketch does not use them in any actuator decision, and there is no calibration from voltage to turbidity units.
- No adaptive alum dosing algorithm is implemented. The M1 motor runs at a fixed speed during a counter window; it is not stopped explicitly when that window ends.
- Repeated `Serial.begin(9600)` calls inside motor functions should be removed; initialize serial once in `setup()`.
- The `seconds > 45` stop commands execute after other branches can start motors again. Replace this with explicit treatment stages and defined motor states.
- Blocking `delay()` calls prevent responsive stop handling and regular sensing. A future revision could use `millis()` and a state machine for each stage.
- Add dry-run tests, a wiring schematic, calibrated measurements, and a documented stop/reset procedure before treating the prototype as a complete water treatment controller.
