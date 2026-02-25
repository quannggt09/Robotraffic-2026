#define sen0Pin 9
#define sen1Pin 10
#define sen2Pin 11
#define sen3Pin 12
#define sen4Pin 13

int sensor[5];

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

int angle = 0;
int last_angle = 0;
const int MAX_ANGLE = 20;

void setup() {
  pinMode(sen0Pin, INPUT);
  pinMode(sen1Pin, INPUT);
  pinMode(sen2Pin, INPUT);
  pinMode(sen3Pin, INPUT);
  pinMode(sen4Pin, INPUT);   
}

void loop() {
  read_sensor_values();
  calculate_pid();
  steering_angle_calculator();
}

void read_sensor_values() {
  sensor[0] = digitalRead(sen0Pin);
  sensor[1] = digitalRead(sen1Pin);
  sensor[2] = digitalRead(sen2Pin);
  sensor[3] = digitalRead(sen3Pin);
  sensor[4] = digitalRead(sen4Pin);

  int weights[5] = {-4, -2, 0, 2, 4};

  int sum = 0;
  int active = 0;

  for (int i = 0; i < 5; i++) {
    if (sensor[i] == 1) {
      sum += weights[i];
      active++;
    }
  }

  if (active != 0) {
    error = sum / active;
  }
  else {
    if (previous_error < 0)
      error = -5;
    else
      error = 5;
  }
}

void calculate_pid() {
  P = error;
  I = I + error;
  I = constrain(I, -50, 50);
  D = error - previous_error;

  PID_value = (Kp * P) + (Ki * I) + (Kd * D);

  previous_error = error;
}

void steering_angle_calculator() {
  angle = PID_value;

  angle = constrain(angle, -MAX_ANGLE, MAX_ANGLE);
}
