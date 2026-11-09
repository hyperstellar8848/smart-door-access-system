
#define GREEN_LED 25
#define RED_LED 26
#define SERVO_PIN 18

void setDoorPosition(int angle) {
  int pulseWidth = map(angle, 0, 180, 500, 2500);
  uint32_t duty = (uint32_t)pulseWidth * 65535 / 20000;

  ledcWrite(SERVO_PIN, duty);
}

void setup() {
  Serial.begin(115200);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  ledcAttach(SERVO_PIN, 50, 16);
  setDoorPosition(0);

  Serial.println("SYSTEM READY");
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "GRANTED") {
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);

      setDoorPosition(90);

      Serial.println("ACCESS GRANTED");
    }
    else if (command == "DENIED") {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);

      setDoorPosition(0);

      Serial.println("ACCESS DENIED");
    }
    else {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, LOW);

      setDoorPosition(0);

      Serial.println("UNKNOWN COMMAND");
    }
  }
}
