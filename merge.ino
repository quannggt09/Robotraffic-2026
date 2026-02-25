#include <ServoTimer2.h>

// IR Sensors
const int IR_RECV = 7; // để làm j v?
const int LINE_OUTS[5] = {2, 3, 4, 5, 6};
int sensor_values[5];

// Servo
const int SERVO_PIN = A2;
ServoTimer2 servo;
int servoPos = 90;
int servoStep = 2; // Reduced step for smoother motion
unsigned long lastMoveTime = 0; 
const int moveInterval = 15;
const int minServoAngle = 40;
const int maxServoAngle = 140;
//duong trai am phai

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

//PID
float error = 0;
float previous_error = 0;
float P = 0;
float I = 0;
float D = 0;
// Tuning by chatgpt =))))))
float Kp = 1.5;
float Ki = 0.0;
float Kd = 8.0;
float PID_value = 0;

float angle = 0;
int last_angle = 0;
const int maxSteeringAngle = 20;

void setup() {
  Serial.begin(9600);

  pinMode(IR_RECV, INPUT); 
  servo.attach(SERVO_PIN);
  servo.write(1500); // xung 1500ums = góc 90 độ

  for (int i = 0; i < 5; i++) {
    pinMode(LINE_OUTS[i], INPUT); //khai báo các chân IR
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

  int irStatus = digitalRead(IR_RECV);
  Serial.print("IR: ");
  Serial.print(irStatus);
  Serial.print(" | Line: ");
  for (int i = 0; i < 5; i++) {
    Serial.print(digitalRead(LINE_OUTS[i]));
    if (i < 4) Serial.print(",");
  }
  Serial.print(" | Speed: ");
  Serial.println(currentSpeed);

  // Thiếu Điều khiển Motor
  line_outs_values();
  calculate_pid();
  int pulse = map(SteeringAngle(), -20, 20, 2000, 1000);
  servo.write(pulse); // đổi từ steering angle qua xung // Đổi 2250, 750 -> 2000, 1000

  Serial.print("Error: ");
  Serial.print(error);
  Serial.print(" | PID: ");
  Serial.print(PID_value);
  Serial.print(" | Pulse: ");
  Serial.println(pulse);
}

void line_outs_values() {
  for(int i=0; i<5; i++){
    sensor_values[i] = digitalRead(LINE_OUTS[i]);
}

  int weights[5] = {-4, -2, 0, 2, 4};
  int sum = 0;
  int active = 0;

  for (int i = 0; i < 5; i++) {
    if (sensor_values[i] == 1) {
      sum += weights[i];
      active++;
    }
  }

  if (active != 0) {
    error = (float)sum / active;
  } else {
    if (previous_error < 0)
      error = -5;
    else
      error = 5;
  }
}

void calculate_pid() {
  P = error;
  // I = I + error;
  // I = constrain(I, -50, 50);
  D = error - previous_error;
  PID_value = (Kp * P) + (Kd * D);
  previous_error = error;
}

float SteeringAngle() {
  angle = PID_value;
  angle = constrain(angle, -maxSteeringAngle, maxSteeringAngle);
  return angle;
}
