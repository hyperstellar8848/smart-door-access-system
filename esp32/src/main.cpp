
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// ==========================================
// HARDWARE MODE
// ==========================================

// 0 = Wokwi: 2 LEDs + Servo
// 1 = Physical: 1 LED

#define HARDWARE_MODE_WOKWI 0
#define HARDWARE_MODE_PHYSICAL 1

// Select PHYSICAL mode:
#define HARDWARE_MODE HARDWARE_MODE_PHYSICAL

// For Wokwi, use this instead:
// #define HARDWARE_MODE HARDWARE_MODE_WOKWI


// ==========================================
// WIFI CONFIGURATION
// ==========================================

// Physical ESP32 Wi-Fi credentials
const char* WIFI_SSID = "YOUR_WIFI_NAME"; /////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; /////////////////////////////////////////////////////////////////////////////////////////////

// Wokwi Wi-Fi:
// WIFI_SSID = "Wokwi-GUEST"
// WIFI_PASSWORD = ""


// ==========================================
// NETWORK CONFIGURATION
// ==========================================

WebServer server(80);

const char* MDNS_HOSTNAME = "smartdoor";

bool serverStarted = false;
bool mdnsStarted = false;

unsigned long lastWiFiAttempt = 0;
const unsigned long WIFI_RETRY_INTERVAL = 10000;


// ==========================================
// PIN CONFIGURATION
// ==========================================

#define GREEN_LED 25
#define RED_LED 26
#define SERVO_PIN 18

#define STATUS_LED 25


// ==========================================
// SERVO CONFIGURATION
// ==========================================

#define SERVO_LOCKED 0
#define SERVO_UNLOCKED 90


// ==========================================
// LED BLINKING CONFIGURATION
// ==========================================

const unsigned long BLINK_INTERVAL = 200;

bool deniedBlinkActive = false;
bool deniedBlinkState = false;

unsigned long lastBlinkTime = 0;
int blinkTransitions = 0;


// ==========================================
// HARDWARE CONTROL
// ==========================================

void setDoorPosition(int angle) {

#if HARDWARE_MODE == HARDWARE_MODE_WOKWI

    int pulseWidth = map(angle, 0, 180, 500, 2500);
    uint32_t duty = (uint32_t)pulseWidth * 65535 / 20000;

    ledcWrite(SERVO_PIN, duty);

#endif
}


void stopDeniedBlink() {

    deniedBlinkActive = false;
    deniedBlinkState = false;
    blinkTransitions = 0;
}


void setLockedState() {

    stopDeniedBlink();

#if HARDWARE_MODE == HARDWARE_MODE_WOKWI

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);

    setDoorPosition(SERVO_LOCKED);

#else

    digitalWrite(STATUS_LED, LOW);

#endif
}


void setGrantedState() {

    stopDeniedBlink();

#if HARDWARE_MODE == HARDWARE_MODE_WOKWI

    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);

    setDoorPosition(SERVO_UNLOCKED);

#else

    digitalWrite(STATUS_LED, HIGH);

#endif
}


void setDeniedState() {

    stopDeniedBlink();

#if HARDWARE_MODE == HARDWARE_MODE_WOKWI

    digitalWrite(GREEN_LED, LOW);

    setDoorPosition(SERVO_LOCKED);

#endif

    deniedBlinkActive = true;
    deniedBlinkState = true;

    lastBlinkTime = millis();
    blinkTransitions = 0;

#if HARDWARE_MODE == HARDWARE_MODE_WOKWI

    digitalWrite(RED_LED, HIGH);

#else

    digitalWrite(STATUS_LED, HIGH);

#endif
}


// ==========================================
// DENIED ACCESS BLINKING
// ==========================================

void updateDeniedBlink() {

    if (!deniedBlinkActive) {
        return;
    }

    if (millis() - lastBlinkTime >= BLINK_INTERVAL) {

        lastBlinkTime = millis();

        deniedBlinkState = !deniedBlinkState;
        blinkTransitions++;

#if HARDWARE_MODE == HARDWARE_MODE_WOKWI

        digitalWrite(RED_LED, deniedBlinkState);

#else

        digitalWrite(STATUS_LED, deniedBlinkState);

#endif

        if (blinkTransitions >= 10) {

            stopDeniedBlink();

#if HARDWARE_MODE == HARDWARE_MODE_WOKWI

            digitalWrite(RED_LED, LOW);

#else

            digitalWrite(STATUS_LED, LOW);

#endif
        }
    }
}


// ==========================================
// HTTP RESPONSE
// ==========================================

void sendResponse(int statusCode, const String& message) {

    server.sendHeader("Access-Control-Allow-Origin", "*");

    server.sendHeader(
        "Access-Control-Allow-Methods",
        "GET, OPTIONS"
    );

    server.sendHeader(
        "Access-Control-Allow-Headers",
        "Content-Type"
    );

    server.send(statusCode, "text/plain", message);
}


// ==========================================
// HTTP ENDPOINTS
// ==========================================

void handleRoot() {

    sendResponse(200, "SMART DOOR SYSTEM ONLINE");
}


void handleDiscovery() {

    String response = "{";

    response += "\"device\":\"SMART_DOOR\",";
    response += "\"status\":\"ONLINE\",";
    response += "\"ip\":\"";
    response += WiFi.localIP().toString();
    response += "\",";
    response += "\"hostname\":\"smartdoor.local\"";

    response += "}";

    server.sendHeader("Access-Control-Allow-Origin", "*");

    server.send(
        200,
        "application/json",
        response
    );
}


void handleGranted() {

    setGrantedState();

    sendResponse(200, "ACCESS GRANTED");
}


void handleDenied() {

    setDeniedState();

    sendResponse(200, "ACCESS DENIED");
}


void handleLock() {

    setLockedState();

    sendResponse(200, "DOOR LOCKED");
}


// ==========================================
// START NETWORK SERVICES
// ==========================================

void startNetworkServices() {

    if (!serverStarted) {

        server.on("/", HTTP_GET, handleRoot);

        server.on(
            "/discover",
            HTTP_GET,
            handleDiscovery
        );

        server.on(
            "/GRANTED",
            HTTP_GET,
            handleGranted
        );

        server.on(
            "/DENIED",
            HTTP_GET,
            handleDenied
        );

        server.on(
            "/LOCK",
            HTTP_GET,
            handleLock
        );

        server.on(
            "/OFF",
            HTTP_GET,
            handleLock
        );

        server.onNotFound([]() {
            sendResponse(404, "ENDPOINT NOT FOUND");
        });

        server.begin();

        serverStarted = true;

        Serial.println("HTTP SERVER STARTED");
    }

    if (!mdnsStarted) {

        if (MDNS.begin(MDNS_HOSTNAME)) {

            MDNS.addService("http", "tcp", 80);

            mdnsStarted = true;

            Serial.println("MDNS STARTED");

            Serial.println(
                "ADDRESS: http://smartdoor.local"
            );

        } else {

            Serial.println("MDNS FAILED");

            Serial.println(
                "USE ESP32 IP ADDRESS INSTEAD"
            );
        }
    }
}


// ==========================================
// WIFI CONNECTION
// ==========================================

void connectWiFi() {

    Serial.println();
    Serial.println("CONNECTING TO WIFI...");

    WiFi.mode(WIFI_STA);

    WiFi.setSleep(false);

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
    );

    unsigned long startAttempt = millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startAttempt < 20000
    ) {

        delay(500);

        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {

        Serial.println("WIFI CONNECTED");

        Serial.print("ESP32 IP ADDRESS: ");

        Serial.println(WiFi.localIP());

        startNetworkServices();

    } else {

        Serial.println("WIFI CONNECTION FAILED");

        Serial.println("WILL RETRY AUTOMATICALLY");
    }

    lastWiFiAttempt = millis();
}


// ==========================================
// SETUP
// ==========================================

void setup() {

    Serial.begin(115200);

    delay(500);

#if HARDWARE_MODE == HARDWARE_MODE_WOKWI

    pinMode(GREEN_LED, OUTPUT);

    pinMode(RED_LED, OUTPUT);

    digitalWrite(GREEN_LED, LOW);

    digitalWrite(RED_LED, LOW);

    ledcAttach(SERVO_PIN, 50, 16);

    setDoorPosition(SERVO_LOCKED);

    Serial.println("SYSTEM READY - WOKWI MODE");

#else

    pinMode(STATUS_LED, OUTPUT);

    digitalWrite(STATUS_LED, LOW);

    Serial.println("SYSTEM READY - PHYSICAL MODE");

#endif

    connectWiFi();
}


// ==========================================
// MAIN LOOP
// ==========================================

void loop() {

    updateDeniedBlink();

    if (serverStarted) {

        server.handleClient();
    }

    // Reconnect Wi-Fi if disconnected

    if (WiFi.status() != WL_CONNECTED) {

        if (serverStarted) {

            server.stop();

            serverStarted = false;

            Serial.println("WIFI DISCONNECTED");
        }

        if (mdnsStarted) {

            MDNS.end();

            mdnsStarted = false;
        }

        if (
            millis() - lastWiFiAttempt >=
            WIFI_RETRY_INTERVAL
        ) {

            connectWiFi();
        }
    }

    // Serial Monitor commands

    if (Serial.available()) {

        String command = Serial.readStringUntil('\n');

        command.trim();

        if (
            command == "GRANTED" ||
            command == "UNLOCK"
        ) {

            setGrantedState();

            Serial.println("ACCESS GRANTED");
        }

        else if (command == "DENIED") {

            setDeniedState();

            Serial.println("ACCESS DENIED");
        }

        else if (
            command == "LOCK" ||
            command == "OFF"
        ) {

            setLockedState();

            Serial.println("DOOR LOCKED");
        }

        else {

            Serial.println("UNKNOWN COMMAND");
        }
    }
}
