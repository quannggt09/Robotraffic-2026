#include <ServoTimer2.h>

const int IR_RECV = 7;
const int LINE_OUTS[5] = {2, 3, 4, 5, 6};

// Servo
const int SERVO_PIN = A2;
ServoTimer2 servo;
int servoPos = 90;
int servoStep = 2; // Reduced step for smoother motion
unsigned long lastMoveTime = 0; 
const int moveInterval = 15;
const int minAngle = 40;
const int maxAngle = 140;

// Motors
const int motorA1 = 10;
const int motorA2 = 11;
const int motorB1 = 12;
const int motorB2 = 13;
const int enA = 9;

// Motor Timing & Logic
unsigned long lastMotorTime = 0;
const int motorInterval = 300; // How fast the speed ramps up
int currentSpeed = 100;
bool goingForward = true;

void setup() {
  Serial.begin(9600);

  pinMode(IR_RECV, INPUT);
  servo.attach(SERVO_PIN);
  servo.write(1500);

  for (int i = 0; i < 5; i++) {
    pinMode(LINE_OUTS[i], INPUT);
  }

  // Initialize Motor Pins
  pinMode(motorA1, OUTPUT);
  pinMode(motorA2, OUTPUT);
  pinMode(motorB1, OUTPUT);
  pinMode(motorB2, OUTPUT);
  pinMode(enA, OUTPUT);

  Serial.println("System Initialized. Starting Motor Test...");
  delay(2000);
}

void loop() {
  unsigned long currentTime = millis();

  // --- 1. SENSOR LOGGING ---
  // (Optional: Wrap in a timer if it scrolls too fast)
  int irStatus = digitalRead(IR_RECV);
  Serial.print("IR: "); Serial.print(irStatus);
  Serial.print(" | Line: ");
  for (int i = 0; i < 5; i++) {
    Serial.print(digitalRead(LINE_OUTS[i]));
    if (i < 4) Serial.print(",");
  }
  Serial.print(" | Speed: "); Serial.println(currentSpeed);

  // --- 2. SERVO SWEEP (Non-blocking) ---
  if (currentTime - lastMoveTime >= moveInterval) {
    lastMoveTime = currentTime;
    servoPos += servoStep;
    if (servoPos >= maxAngle || servoPos <= minAngle) {
      servoStep = -servoStep; 
    }
    servo.write(map(servoPos, 0, 180, 750, 2250));
  }

  // --- 3. MOTOR TEST (Slow to Fast, Forward then Backward) ---
  if (currentTime - lastMotorTime >= motorInterval) {
    lastMotorTime = currentTime;

    // Increment speed
    currentSpeed += 5;

    // When max speed reached, flip direction and reset
    if (currentSpeed > 255) {
      delay(500);
      currentSpeed = 100;
      goingForward = !goingForward;
      Serial.println(goingForward ? "--- SWITCHING FORWARD ---" : "--- SWITCHING BACKWARD ---");
      digitalWrite(motorA1, LOW);
      digitalWrite(motorA2, LOW);
      // Motor B Forward
      digitalWrite(motorB1, LOW);
      digitalWrite(motorB2, LOW);
    }

    if (goingForward) {
      // Motor A Forward
      digitalWrite(motorA1, HIGH);
      digitalWrite(motorA2, LOW);
      // Motor B Forward
      digitalWrite(motorB1, HIGH);
      digitalWrite(motorB2, LOW);
    } else {
      // Motor A Backward
      digitalWrite(motorA1, LOW);
      digitalWrite(motorA2, HIGH);
      // Motor B Backward
      digitalWrite(motorB1, LOW);
      digitalWrite(motorB2, HIGH);
    }

    // Write speed to Enable pins
    analogWrite(enA, currentSpeed);
  }
}