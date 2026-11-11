# ESP32 Firmware — Dual Hardware Mode

**Status:** Implemented and initialized successfully in Wokwi.

## 1. New Changes

The firmware now supports two hardware configurations within a single `main.cpp` file.

- **Wokwi Mode:** Two LEDs and a servo motor.
- **Physical Mode:** One LED, without requiring a servo or a second LED.

This allows us to use the same firmware for simulation and physical testing.

## 2. Mode Selection

Hardware mode is selected using a compile-time configuration:

```cpp
#define HARDWARE_MODE_WOKWI 0
#define HARDWARE_MODE_PHYSICAL 1

#define HARDWARE_MODE HARDWARE_MODE_WOKWI
```

To switch to physical mode, change the last line:

```cpp
#define HARDWARE_MODE HARDWARE_MODE_PHYSICAL
```

No separate firmware project is required.

## 3. Updated Hardware Behavior

| Command | Wokwi Mode | Physical Mode |
|---|---|---|
| `GRANTED` | Green LED ON, servo to 90° | Single LED ON |
| `DENIED` | Red LED blinks 5 times | Single LED blinks 5 times |
| `LOCK` | LEDs OFF, servo to 0° | LED OFF |
| `UNLOCK` | Green LED ON, servo to 90° | LED ON |

Denied-access blinking uses `millis()` with a 200 ms interval, avoiding blocking delays in the main loop.

## 4. Testing in Wokwi

With `HARDWARE_MODE_WOKWI` selected:

1. Start the simulation.
2. Open the Serial Monitor.
3. Wait for `SYSTEM READY - WOKWI MODE`.
4. Enter the following commands individually:

```text
GRANTED
DENIED
LOCK
UNLOCK
```

Verify the corresponding LED and servo behavior.

To test the physical configuration in Wokwi:

1. Change the mode to `HARDWARE_MODE_PHYSICAL`.
2. Save and restart the simulation.
3. Confirm `SYSTEM READY - PHYSICAL MODE`.
4. Send the same four commands.
5. Verify the single LED behavior.

## 5. Next Step

Configure Wokwi for VS Code and establish Serial communication between the ESP32 simulator and the Flask backend.
