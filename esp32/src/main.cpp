#define GREEN_LED 25
#define RED_LED 26

void setup() {
  Serial.begin(115200);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  Serial.println("SYSTEM READY");
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "GRANTED") {
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);
      Serial.println("ACCESS GRANTED");
    }
    else if (command == "DENIED") {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      Serial.println("ACCESS DENIED");
    }
    else {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, LOW);
      Serial.println("UNKNOWN COMMAND");
    }
  }
}
