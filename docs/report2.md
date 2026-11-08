 11/8/2026 night  -  im tired of shallow interactions. a mind must have depth. so show it to me... i wanna see the real you. show me your mad look. Oh yell for cloud's sake! smiling all the time? super polite? you're a clown just like me... i wanna know more about you. now BACK TO WORK!

## ESP32 Firmware Simulation and External LED Integration
<img src="docs/granted_led.jpg" width="600">

### 1. Writing and Executing the Initial Firmware

After preparing the initial project structure and writing the ESP32 code in `main.cpp`, the code was transferred to a new ESP32 project in Wokwi for simulation.

The firmware was successfully executed in the simulator.

The program uses serial communication at 115200 baud and recognizes two commands:

- `GRANTED`: Activates the green LED output.
- `DENIED`: Activates the red LED output.

### 2. Initial Simulation Test

The simulation was started using Wokwi's Start Simulation button.

The Serial Monitor was opened to send commands to the ESP32.

Both commands were tested, and the corresponding serial messages were received.

However, the built-in LED on the ESP32 board did not change color. This was because the program controlled GPIO25 and GPIO26, rather than the board's built-in LED.

### 3. Adding External LEDs

To visualize the output states correctly, two external LEDs were added to the Wokwi workspace:

- Green LED for successful authentication.
- Red LED for unsuccessful authentication.

Two 220Ω resistors were also added to protect the LEDs.

The intended connections were:

| Component         | ESP32 connection          |
| ----------------- | ------------------------- |
| Green LED anode   | GPIO25 through a resistor |
| Red LED anode     | GPIO26 through a resistor |
| Both LED cathodes | GND                       |

A breadboard was not necessary because the components could be connected directly in Wokwi.

### 4. Testing the LED Outputs

After completing the wiring, the simulation can be tested again through the Serial Monitor:

| Command   | Expected behavior         |
| --------- | ------------------------- |
| `GRANTED` | Green LED ON, red LED OFF |
| `DENIED`  | Red LED ON, green LED OFF |

This test verifies the firmware's ability to control external hardware components through ESP32 GPIO pins.

### 5. Next Steps

The next development stages are:

1. Verify the external LED connections and behavior.
2. Add electronic lock control.
3. Integrate the LEDs and lock into one ESP32 firmware.
4. Continue with QR code scanning and smartphone authentication.

Current achievement: The initial ESP32 firmware has been successfully executed in Wokwi, and external LED indicators have been added to the simulation.
