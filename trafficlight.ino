
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

const unsigned long GREEN_FLASH_TOGGLE_MS = 300;   // blink speed
const unsigned long IR_SEND_PERIOD_MS     = 120;   // how often to broadcast state

// ----------------- IR settings -----------------
const uint8_t IR_SEND_PIN = 3; // Uno recommended with IRremote

// Simple "message" codes (pick any distinct values you like)
const uint32_t IR_CODE_RED         = 0xA1B2C301;
const uint32_t IR_CODE_RED_YELLOW  = 0xA1B2C302;
const uint32_t IR_CODE_GREEN       = 0xA1B2C303;
const uint32_t IR_CODE_GREEN_FLASH = 0xA1B2C304;
const uint32_t IR_CODE_YELLOW      = 0xA1B2C305;

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

void allOff() {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(YELLOW_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
}

uint32_t codeForState(LightState s) {
  switch (s) {
    case S_RED:         return IR_CODE_RED;
    case S_RED_YELLOW:  return IR_CODE_RED_YELLOW;
    case S_GREEN:       return IR_CODE_GREEN;
    case S_GREEN_FLASH: return IR_CODE_GREEN_FLASH;
    case S_YELLOW:      return IR_CODE_YELLOW;
    default:            return 0;
  }
}

void setLedsForState(LightState s, unsigned long nowMs) {
  // Default: ensure deterministic LED outputs
  allOff();

  switch (s) {
    case S_RED:
      digitalWrite(RED_PIN, HIGH);
      break;

    case S_RED_YELLOW:
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(YELLOW_PIN, HIGH);
      break;

    case S_GREEN:
      digitalWrite(GREEN_PIN, HIGH);
      break;

    case S_GREEN_FLASH:
      // Non-blocking blinking
      if (nowMs - lastFlashToggleMs >= GREEN_FLASH_TOGGLE_MS) {
        lastFlashToggleMs = nowMs;
        greenFlashOn = !greenFlashOn;
      }
      digitalWrite(GREEN_PIN, greenFlashOn ? HIGH : LOW);
      break;

    case S_YELLOW:
      digitalWrite(YELLOW_PIN, HIGH);
      break;
  }
}

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

void sendIrStateIfNeeded(unsigned long nowMs) {
  if (nowMs - lastIrSendMs >= IR_SEND_PERIOD_MS) {
    lastIrSendMs = nowMs;

    // NEC: common + easy to decode. This call is short; no delay().
    // Address can be any 16-bit value; command is 8-bit (we'll hash the state).
    uint32_t code = codeForState(state);

    uint16_t address = 0x1234;
    uint8_t command  = (uint8_t)(code & 0xFF);

    IrSender.sendNEC(address, command, 0); // repeats=0
  }
}

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  allOff();

  // IRremote setup
  IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK); // LED feedback optional

  state = S_RED;
  stateStartMs = millis();
}

void loop() {
  unsigned long nowMs = millis();

  advanceStateIfNeeded(nowMs);
  setLedsForState(state, nowMs);

  // Broadcast the current light state over IR
  sendIrStateIfNeeded(nowMs);

  // No delay() anywhere; loop runs fast.
}
