
// ==========================================
// SMART DOOR ACCESS CONTROL - ESP32
// ==========================================

// Select hardware mode:
// 0 = Wokwi: 2 LEDs + Servo
// 1 = Physical: 1 LED only

#define HARDWARE_MODE_WOKWI 0
#define HARDWARE_MODE_PHYSICAL 1

// choose what mode you want, by commenting the other mode <3
#define HARDWARE_MODE HARDWARE_MODE_WOKWI
// #define HARDWARE_MODE HARDWARE_MODE_PHYSICAL

// Pin configuration
#define GREEN_LED 25
#define RED_LED 26
#define SERVO_PIN 18

#define STATUS_LED 25

// Servo configuration
#define SERVO_LOCKED 0
#define SERVO_UNLOCKED 90

// LED blinking configuration
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

#else

    // Single LED represents denied access.

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
// SETUP
// ==========================================

void setup() {

    Serial.begin(115200);

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
}

// ==========================================
// MAIN LOOP
// ==========================================

void loop() {

    updateDeniedBlink();

    if (Serial.available()) {

        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "GRANTED" || command == "UNLOCK") {

            setGrantedState();

            Serial.println("ACCESS GRANTED");
        }

        else if (command == "DENIED") {

            setDeniedState();

            Serial.println("ACCESS DENIED");
        }

        else if (command == "LOCK") {

            setLockedState();

            Serial.println("DOOR LOCKED");
        }

        else {

            Serial.println("UNKNOWN COMMAND");
        }
    }
}
