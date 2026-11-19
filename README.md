# Smart Door Access Control System
## Final Project Report

# 1. Introduction

This project focuses on designing and implementing a smart door access control system using the ESP32 microcontroller. The main objective is to connect digital user authentication with physical door control through a web-based interface.

Users can access the system through a web URL. After authenticating with a Passkey using the WebAuthn standard, the backend verifies their identity and checks their access permissions.

If authentication and authorization are successful, the backend sends a command to the ESP32, which changes the simulated door lock state.

The hardware component was developed and tested using the Wokwi simulator, with an ESP32, two LEDs, and a servo motor representing the door locking mechanism.

# 2. Project Objectives

The main objectives of this project are:

- Design a smart door access control system based on ESP32.
- Implement secure user authentication using Passkeys and WebAuthn.
- Verify user permissions before granting access.
- Develop a web-based interface for interacting with the system.
- Establish communication between a Python backend and the ESP32.
- Control the simulated door lock through serial commands.
- Store user information, permissions, and access logs.
- Simulate and test the hardware using Wokwi.
- Provide access to the web interface through a public URL.

# 3. System Architecture

The system consists of three main components:

## 3.1. Frontend

The frontend is the user-facing part of the application. It was developed using:

- **HTML:** Defines the structure of the web pages.
- **CSS:** Handles the visual design and layout.
- **JavaScript:** Manages user interactions and communication with the backend.

Users access the interface through a web browser, where they can initiate authentication and control the door.

## 3.2. Backend

The backend was developed using Python and the Flask framework.

Its main responsibilities include:

- Handling requests from the frontend.
- Managing user registration and authentication.
- Verifying WebAuthn credentials.
- Checking user access permissions.
- Managing user sessions.
- Sending commands to the ESP32.
- Receiving responses from the microcontroller.
- Recording access events.

## 3.3. Hardware

The hardware component is based on the ESP32 microcontroller.

For the Wokwi simulation, the following components were used:

- ESP32 development board.
- Green LED to indicate authorized access.
- Red LED to indicate denied access.
- Servo motor to represent the door locking mechanism.

The servo motor simulates the physical lock. It moves to the unlocked position when an authorized command is received and returns to the locked position when the door is locked.

# 4. Technologies and Development Tools

| Technology / Tool | Purpose |
|---|---|
| ESP32 | Main microcontroller |
| Arduino Cloud IDE | Compiling firmware and generating the binary file |
| Visual Studio Code | Development environment |
| Wokwi Simulator Extension | Hardware simulation inside VS Code |
| Wokwi License Key | Activating the Wokwi extension |
| Python | Backend programming |
| Flask | Web server and API development |
| PySerial | Serial communication with ESP32 |
| HTML | Frontend structure |
| CSS | Frontend styling |
| JavaScript | Frontend interaction |
| WebAuthn | Authentication standard |
| Passkeys | Passwordless authentication |
| JSON | Data storage |
| RFC2217 | Serial communication through the Wokwi simulator |

# 5. Authentication and Access Management

One of the most important parts of the project is the implementation of secure user authentication using WebAuthn.

WebAuthn is a web authentication standard based on public-key cryptography. It enables passwordless authentication through Passkeys.

Instead of relying on traditional passwords, users authenticate using a supported security mechanism on their device.

## 5.1. User Registration

During registration:

1. The user initiates the registration process.
2. The backend generates the required registration information.
3. The browser invokes the WebAuthn API.
4. The user creates a Passkey using the device's supported authentication mechanism.
5. The resulting credential information is returned to the backend.
6. The backend stores the credential information.

## 5.2. User Authentication

During authentication:

1. The user initiates a login request.
2. The backend generates the required authentication challenge.
3. The browser invokes the WebAuthn API.
4. The user verifies their identity using the device's authentication mechanism, such as a PIN.
5. The authentication response is sent to the backend.
6. The backend verifies the cryptographic response.

If authentication succeeds, the system proceeds to the authorization stage.

## 5.3. Access Authorization

Authentication alone is not sufficient to unlock the door. The system must also verify whether the authenticated user has permission to access the requested door.

The backend checks the user's permissions before sending any hardware command.

- If the user is authorized, the backend sends `GRANTED`.
- If the user is not authorized, the backend sends `DENIED`.

This separates identity verification from access authorization.

# 6. Data Storage

The project uses JSON files to store application data.

The main data files are:

| File | Purpose |
|---|---|
| `user.json` | Stores user information |
| `passkeys.json` | Stores authentication credential information |
| `permissions.json` | Stores user access permissions |
| `access_logs.json` | Stores access events and activity records |

The `storage.py` module handles reading and writing these JSON files.

This approach provides a simple data management solution suitable for the current prototype.

# 7. Web Interface and Backend Communication

The frontend communicates with the Flask backend through API endpoints.

JavaScript sends requests to the backend for authentication and door-control operations.

The backend acts as the central controller between the web application and the hardware.

The browser does not communicate directly with the ESP32. Instead, the backend processes the request, verifies the user's identity and permissions, and then sends the appropriate command to the microcontroller.

A public URL was configured to make the web interface accessible through a browser without requiring users to open the local development environment directly.

# 8. ESP32 Firmware Development

The ESP32 firmware was developed using Arduino-compatible C++ code.

Its main responsibilities are:

- Receiving serial commands.
- Processing the received commands.
- Controlling the LEDs.
- Controlling the servo motor.
- Returning responses to the backend.

The firmware supports two hardware configurations.

## 8.1. Wokwi Mode

The simulation mode uses two LEDs and one servo motor.

| Command | Expected Behavior |
|---|---|
| `GRANTED` | Green LED ON, red LED OFF, servo moves to 90° |
| `UNLOCK` | Green LED ON, red LED OFF, servo moves to 90° |
| `DENIED` | Red LED blinks five times; door remains locked |
| `LOCK` | Both LEDs turn OFF; servo returns to 0° |

The servo represents the door lock:

- **0°:** Locked position.
- **90°:** Unlocked position.

When access is denied, the red LED blinks five times while the door remains locked.

## 8.2. Physical Mode

A separate physical hardware mode is also defined in the firmware.

This mode uses a single status LED instead of the two LEDs and servo used in Wokwi.

Its behavior is:

- LED ON: Access granted / unlocked status.
- LED OFF: Locked status.
- LED blinking: Access denied.

The firmware can switch between Wokwi and physical configurations through its hardware mode definition.

**Note:** The final end-to-end test described in this report was performed using Wokwi simulation. The physical hardware mode was prepared in the firmware but was not validated with a real ESP32 board and physical lock.

# 9. Wokwi Setup in Visual Studio Code

The Wokwi Simulator extension was used to run the ESP32 simulation directly inside Visual Studio Code.

## 9.1. Installing and Activating the Extension

First, the Wokwi Simulator extension was installed in Visual Studio Code.

The extension required a license key to activate its functionality. After obtaining the key, it was entered through the extension's activation process.

## 9.2. Creating the Wokwi Configuration File

Initially, the extension displayed the following error:

```text
Wokwi: wokwi.toml configuration file not found in workspace
```

To solve this issue, a `wokwi.toml` file was created inside the `esp32/wokwi/` directory.

Its configuration was:

```toml
[wokwi]
version = 1
firmware = 'firmware.bin'
rfc2217ServerPort = 4000
```

This configuration specifies the firmware file and enables the RFC2217 serial server on port `4000`.

The configuration file was then selected using the VS Code Command Palette:

`Wokwi: Select Config File`

## 9.3. Compiling Firmware with Arduino Cloud IDE

The Wokwi extension required a compiled firmware binary (`.bin`) to run the simulation.

Instead of using PlatformIO or Arduino CLI, Arduino Cloud IDE was used to compile the ESP32 source code.

The process was:

1. Open Arduino Cloud IDE.
2. Import the Arduino project.
3. Select the ESP32 board.
4. Compile the source code.
5. Download the generated compilation archive.
6. Extract the firmware binary.
7. Copy the `.bin` file into `esp32/wokwi/`.
8. Rename the file to `firmware.bin`.

This generated the firmware required by the Wokwi extension.

## 9.4. Running the Simulator

The simulator was started using:

`Wokwi: Start Simulator`

During the first attempt, only ESP32 bootloader messages appeared in the serial monitor. This indicated that the expected project firmware was not running.

The source code was compiled again using Arduino Cloud IDE, and the old `firmware.bin` was replaced with the updated binary.

After restarting the simulator, the following message appeared:

```text
SYSTEM READY - WOKWI MODE
```

This confirmed that the intended firmware was running.

# 10. Serial Communication Between Flask and ESP32

To connect the Python backend to the simulated ESP32, serial communication was configured through RFC2217.

The Wokwi configuration exposed the serial connection on port `4000`.

The connection was tested using PowerShell:

```powershell
Test-NetConnection localhost -Port 4000
```

The result was:

```text
TcpTestSucceeded : True
```

This confirmed that the serial server was accessible.

Direct communication was also tested using Python and PySerial.

For example:

- Command: `GRANTED`
- ESP32 response: `ACCESS GRANTED`

The corresponding LED and servo behavior was observed in the simulation.

## 10.1. Backend Serial Integration

Initially, the Flask backend used a simulation flag:

```python
SIMULATE_HARDWARE = True
```

This meant that hardware operations were simulated inside the backend instead of being sent to the ESP32.

To establish real communication with the simulator, `website/backend/app.py` was modified.

The serial connection was configured as follows:

```python
SERIAL_PORT = "rfc2217://localhost:4000"
SERIAL_BAUDRATE = 115200
```

A helper function named `send_hardware_command()` was added to send commands and read responses using PySerial.

The function:

1. Opens the RFC2217 serial connection.
2. Sends the requested command.
3. Flushes the connection.
4. Reads the ESP32 response.
5. Handles communication errors and missing responses.

## 10.2. Hardware Command Integration

The backend connects authentication and door-control operations to the ESP32 commands.

| Backend Event | Command | Hardware Behavior |
|---|---|---|
| Successful authentication | `GRANTED` | Green LED ON and servo unlocks |
| Unauthorized access | `DENIED` | Red LED blinks; door remains locked |
| Failed authentication | `DENIED` | Red LED blinks; door remains locked |
| Lock door | `LOCK` | LEDs turn OFF and servo returns to locked position |

The backend also handles communication failures and reports an error if the ESP32 does not respond.

## 10.3. Resolving RFC2217 Compatibility

During integration, the following error appeared:

```text
write_timeout is currently not supported in site
```

The issue was caused by using a serial connection parameter that was not supported by the RFC2217 implementation.

The `write_timeout` parameter was removed from the connection configuration.

After this correction, the backend syntax was checked using:

```powershell
python -m py_compile app.py
```

The command completed successfully.

# 11. Complete System Workflow

The final system operates through the following sequence:

1. The user opens the public website URL in a browser.
2. The web interface is displayed.
3. The user initiates authentication using a Passkey.
4. WebAuthn performs the authentication process.
5. The authentication response is sent to the Flask backend.
6. The backend verifies the user's identity.
7. The backend checks the user's access permissions.
8. If access is authorized, the backend sends `GRANTED` to the ESP32.
9. The ESP32 receives the command and activates the green LED.
10. The servo moves to the unlocked position.
11. The backend returns the operation result to the frontend.
12. When the user requests the door to be locked, the backend sends `LOCK`.
13. The ESP32 turns off the LEDs and returns the servo to the locked position.

If authentication or authorization fails, the backend sends `DENIED`, and the ESP32 activates the red LED warning pattern.

## System Communication Flow

```text
             User / Browser
                    |
                    v
              Web Interface
              HTML / CSS / JS
                    |
                    v
              Flask Backend
                    |
          +---------+---------+
          |                   |
          v                   v
      WebAuthn             Permission
      Verification          Checking
          |                   |
          +---------+---------+
                    |
                    v
             PySerial / RFC2217
                    |
                    v
                 Wokwi
                    |
                    v
                  ESP32
                    |
             +------+------+
             |             |
             v             v
            LEDs          Servo
             |             |
             +------+------+
                    |
                Door State
```

# 12. Testing and Final Results

The system was tested in several stages.

## 12.1. Firmware Testing

The ESP32 firmware was compiled and executed in Wokwi.

The serial monitor displayed:

```text
SYSTEM READY - WOKWI MODE
```

This confirmed that the expected firmware was running.

## 12.2. Serial Communication Testing

The RFC2217 connection was checked on port `4000`.

The successful `TcpTestSucceeded : True` result confirmed that the serial server was accessible.

Direct Python-to-ESP32 communication was also tested successfully.

## 12.3. Hardware Command Testing

The main hardware commands were tested:

| Command | Expected Result |
|---|---|
| `GRANTED` | Green LED ON and servo unlocks |
| `DENIED` | Red LED blinks five times |
| `LOCK` | LEDs OFF and servo returns to locked position |

## 12.4. Website-to-Hardware Integration Testing

After connecting PySerial to the Flask backend, the complete website-to-hardware communication path was tested.

The web interface was opened through its public URL.

Authentication and door-control operations were performed through the website, and the simulated ESP32 responded to the backend commands.

The door could be unlocked and locked through the web interface.

This confirmed that the integration between the frontend, Flask backend, serial communication layer, and Wokwi ESP32 simulation was working.

# 13. Project File Structure

The project was organized into the following directories:

```text
smart-door-access-system/
│
├── docs/
│
├── esp32/
│   ├── src/
│   │   └── main.cpp
│   │
│   └── wokwi/
│       ├── diagram.json
│       ├── sketch.ino
│       ├── wokwi.toml
│       └── firmware.bin
│
├── pc/
│
├── website/
│   ├── backend/
│   │   ├── app.py
│   │   ├── auth.py
│   │   ├── storage.py
│   │   └── data/
│   │       ├── user.json
│   │       ├── passkeys.json
│   │       ├── permissions.json
│   │       └── access_logs.json
│   │
│   └── ...
│
└── README.md
```

### Main File Responsibilities

| File | Responsibility |
|---|---|
| `main.cpp` | Main ESP32 firmware |
| `sketch.ino` | Arduino firmware used for Wokwi |
| `diagram.json` | Wokwi circuit components and connections |
| `wokwi.toml` | Wokwi simulator configuration |
| `firmware.bin` | Compiled firmware binary |
| `app.py` | Flask backend and API routes |
| `auth.py` | WebAuthn authentication functions |
| `storage.py` | JSON data storage operations |
| JSON files | User data, credentials, permissions, and access logs |

# 14. Challenges and Solutions

Several technical challenges were encountered during development.

| Challenge | Solution |
|---|---|
| Activating the Wokwi extension | Obtained and entered a Wokwi license key |
| Missing Wokwi configuration | Created `wokwi.toml` |
| Simulator requiring compiled firmware | Used Arduino Cloud IDE |
| Old firmware being executed | Recompiled the source and replaced `firmware.bin` |
| Serial connection unavailable | Enabled RFC2217 on port `4000` |
| Website not controlling the simulator | Modified Flask backend to use PySerial |
| RFC2217 not supporting `write_timeout` | Removed the unsupported parameter |

# 15. Limitations and Future Improvements

Although the main authentication and hardware-control workflow was successfully implemented and tested in simulation, several improvements are possible.

Future development could include:

- Connecting a real ESP32 board to a physical electronic door lock.
- Replacing the simulated servo mechanism with a real locking mechanism.
- Adding a door-position sensor to detect whether the door is physically open or closed.
- Implementing actual QR code scanning if required.
- Developing an administration dashboard for managing users and permissions.
- Improving error handling and hardware disconnection management.
- Migrating from JSON files to a database for larger deployments.
- Using HTTPS and appropriate security configurations for production deployment.
- Improving session security and protection of sensitive authentication data.

**Important implementation notes:**

- The current URL-based interface does not mean that a QR code scanner has been implemented. Actual QR scanning remains a possible future feature.
- The firmware includes a physical hardware mode, but the final end-to-end test was performed using Wokwi simulation.
- The public URL provides access to the web interface. The local backend and Wokwi simulator must remain available on the host system for the simulated hardware-control workflow to operate.
