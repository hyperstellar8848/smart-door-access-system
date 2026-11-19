# Wokwi Simulation Setup and Website–ESP32 Integration Report

## 1. Activating the Wokwi VS Code Extension

To run the ESP32 simulation directly inside Visual Studio Code, we installed the **Wokwi Simulator** extension.

The extension required a license key to activate its functionality. After obtaining the license key, we entered it through the extension's activation process.

## 2. Configuring the Wokwi Project

After activation, we opened the project's `esp32/wokwi` folder in VS Code.

Initially, the extension displayed the following error:

`Wokwi: wokwi.toml configuration file not found in workspace`

To resolve this, we created a `wokwi.toml` configuration file inside the Wokwi project folder.

Its configuration was:

```toml
[wokwi]
version = 1
firmware = 'firmware.bin'
rfc2217ServerPort = 4000
```

This configuration specifies the compiled firmware file and enables the RFC2217 serial server on port `4000`. The serial connection allows the Python backend to communicate with the simulated ESP32.

We then used **Wokwi: Select Config File** in the VS Code Command Palette to select the configuration file.

## 3. Compiling the ESP32 Firmware Online

The Wokwi extension required a compiled firmware file (`.bin`) to run the simulation. It did not compile the Arduino `.ino` source file directly.

Since we were not using PlatformIO or Arduino CLI, we used an online Arduino compiler:

[Arduino Web Compiler](https://arduino-web-compiler.vercel.app/)

The steps were:

1. Open the online compiler website.
2. Import the Wokwi project ZIP file.
3. Select the ESP32 as the target board.
4. Compile the project.
5. Download the generated compilation archive.
6. Extract the firmware binary file.
7. Copy the `.bin` file into `esp32/wokwi/`.
8. Rename it to `firmware.bin`, matching the filename in `wokwi.toml`.

This produced the firmware file required by the VS Code extension.

## 4. Running the Wokwi Simulator

After placing the compiled firmware in the correct folder, we opened the Command Palette and selected:

`Wokwi: Start Simulator`

At first, the serial monitor displayed only ESP32 bootloader messages. This indicated that the simulator was not yet running the updated project firmware.

We recompiled the latest source code using the online compiler, replaced the old `firmware.bin`, and restarted the simulator.

After this correction, the serial monitor displayed:

```text
SYSTEM READY - WOKWI MODE
```

This confirmed that the intended firmware was running.

## 5. Establishing Serial Communication

The next step was connecting the simulated ESP32 to the Python backend.

The Wokwi configuration exposed an RFC2217 serial connection on port `4000`.

We tested the connection in PowerShell using:

```powershell
Test-NetConnection localhost -Port 4000
```

The result showed:

```text
TcpTestSucceeded : True
```

This confirmed that the serial server was accessible.

We also tested sending a command directly to the simulated ESP32 using Python and PySerial. The ESP32 received the command and returned a response.

For example:

- Command: `GRANTED`
- Response: `ACCESS GRANTED`

The corresponding LED and servo behavior also worked in the simulation.

## 6. Connecting the Website to the Simulated ESP32

After confirming that direct serial communication worked, we investigated why authentication through the website did not control the simulated door.

We discovered that the Flask backend was still using a hardware simulation flag:

```python
SIMULATE_HARDWARE = True
```

With this setting, the backend simulated successful hardware operations instead of sending commands to the ESP32.

We modified `website/backend/app.py` to establish actual serial communication using PySerial.

The backend now uses:

```python
SERIAL_PORT = "rfc2217://localhost:4000"
SERIAL_BAUDRATE = 115200
```

We also introduced a helper function:

```python
def send_hardware_command(command):
    try:
        with serial.serial_for_url(
            SERIAL_PORT,
            baudrate=SERIAL_BAUDRATE,
            timeout=3,
        ) as connection:
            connection.write(f"{command}\n".encode("utf-8"))
            connection.flush()
            response = connection.readline().decode(
                "utf-8", errors="replace"
            ).strip()

        if not response:
            raise RuntimeError("ESP32 did not respond to the command.")

        return response

    except (serial.SerialException, OSError) as error:
        raise RuntimeError(
            f"Could not communicate with the ESP32: {error}"
        ) from error
```

This function sends a command to the ESP32 and reads its response.

### Backend command integration

We connected the authentication and door-control operations to the hardware commands:

| Website / Backend event | ESP32 command | Expected behavior |
|---|---|---|
| Successful authentication | `GRANTED` | Green LED ON, red LED OFF, servo unlocks |
| Unauthorized access | `DENIED` | Red LED blinks, door remains locked |
| Failed authentication | `DENIED` | Red LED blinks, door remains locked |
| Lock door | `LOCK` | LEDs turn OFF, servo returns to locked position |

The backend also handles communication failures and returns an error if the ESP32 does not respond.
