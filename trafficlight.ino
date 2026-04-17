#include <IRremote.hpp>

// ----------------- Visible LEDs -----------------
const uint8_t RED_PIN    = 11;
const uint8_t YELLOW_PIN = 12;
const uint8_t GREEN_PIN  = 13;

// ----------------- Timings (ms) -----------------
const unsigned long TIME_RED         = 7000;
const unsigned long TIME_RED_YELLOW  = 3000;
const unsigned long TIME_GREEN       = 10000;
const unsigned long TIME_GREEN_FLASH = 2000;
const unsigned long TIME_YELLOW      = 3000;

const unsigned long GREEN_FLASH_TOGGLE_MS = 300;
const unsigned long IR_SEND_PERIOD_MS     = 300; // more stable

// ----------------- IR settings -----------------
const uint8_t IR_SEND_PIN = 3;

// ----------------- State machine -----------------
enum LightState {
  S_RED,
  S_RED_YELLOW,
  S_GREEN,
  S_GREEN_FLASH,
  S_YELLOW
};

LightState state = S_RED;

unsigned long stateStartMs = 0;
unsigned long lastFlashToggleMs = 0;
bool greenFlashOn = false;

unsigned long lastIrSendMs = 0;

// ----------------- STATE → CODE -----------------
uint8_t codeForState(LightState s) {
  switch (s) {
    case S_RED:         return 0;
    case S_RED_YELLOW:  return 1;
    case S_GREEN:       return 2;
    case S_GREEN_FLASH: return 3;
    case S_YELLOW:      return 4;
    default:            return 0;
  }
}

// ----------------- LED CONTROL -----------------
void setLedsForState(LightState s, unsigned long nowMs) {
  switch (s) {
    case S_RED:
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(YELLOW_PIN, LOW);
      digitalWrite(GREEN_PIN, LOW);
      break;

    case S_RED_YELLOW:
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(YELLOW_PIN, HIGH);
      digitalWrite(GREEN_PIN, LOW);
      break;

    case S_GREEN:
      digitalWrite(RED_PIN, LOW);
      digitalWrite(YELLOW_PIN, LOW);
      digitalWrite(GREEN_PIN, HIGH);
      break;

    case S_GREEN_FLASH:
      if (nowMs - lastFlashToggleMs >= GREEN_FLASH_TOGGLE_MS) {
        lastFlashToggleMs = nowMs;
        greenFlashOn = !greenFlashOn;
      }
      digitalWrite(RED_PIN, LOW);
      digitalWrite(YELLOW_PIN, LOW);
      digitalWrite(GREEN_PIN, greenFlashOn ? HIGH : LOW);
      break;

    case S_YELLOW:
      digitalWrite(RED_PIN, LOW);
      digitalWrite(YELLOW_PIN, HIGH);
      digitalWrite(GREEN_PIN, LOW);
      break;
  }
}

// ----------------- STATE TRANSITION -----------------
void advanceStateIfNeeded(unsigned long nowMs) {
  unsigned long elapsed = nowMs - stateStartMs;

  switch (state) {
    case S_RED:
      if (elapsed >= TIME_RED) {
        state = S_RED_YELLOW;
        stateStartMs = nowMs;
      }
      break;

    case S_RED_YELLOW:
      if (elapsed >= TIME_RED_YELLOW) {
        state = S_GREEN;
        stateStartMs = nowMs;
      }
      break;

    case S_GREEN:
      if (elapsed >= TIME_GREEN) {
        state = S_GREEN_FLASH;
        stateStartMs = nowMs;
        lastFlashToggleMs = nowMs;
        greenFlashOn = false;
      }
      break;

    case S_GREEN_FLASH:
      if (elapsed >= TIME_GREEN_FLASH) {
        state = S_YELLOW;
        stateStartMs = nowMs;
      }
      break;

    case S_YELLOW:
      if (elapsed >= TIME_YELLOW) {
        state = S_RED;
        stateStartMs = nowMs;
      }
      break;
  }
}

// ----------------- IR SENDING -----------------
void sendIrStateIfNeeded(unsigned long nowMs) {
  if (nowMs - lastIrSendMs >= IR_SEND_PERIOD_MS) {
    lastIrSendMs = nowMs;

    uint16_t address = 0x1234;
    uint8_t command  = codeForState(state);

    IrSender.sendNEC(address, command, 0);
  }
}

// ----------------- SETUP -----------------
void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);

  IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK);

  state = S_RED;
  stateStartMs = millis();
}

// ----------------- LOOP -----------------
void loop() {
  unsigned long nowMs = millis();

  advanceStateIfNeeded(nowMs);
  setLedsForState(state, nowMs);
  sendIrStateIfNeeded(nowMs);
}
