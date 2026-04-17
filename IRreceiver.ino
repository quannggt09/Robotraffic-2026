#include <IRremote.hpp>

const uint8_t IR_RECEIVE_PIN = 2;   // VS1838B OUT nối D2
const int SIGNAL_OUT = 5;

void setup() {
  Serial.begin(115200);

  // Khởi động IR receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  pinMode(SIGNAL_OUT, OUTPUT);

  Serial.println("IR Receiver Ready...");
}

void loop() {
  if (IrReceiver.decode()) {
    // In thông tin gói tin nhận được
    Serial.print("Protocol: ");
    Serial.println(IrReceiver.decodedIRData.protocol);

    Serial.print("Address: 0x");
    Serial.println(IrReceiver.decodedIRData.address, HEX);

    Serial.print("Command: 0x");
    Serial.println(IrReceiver.decodedIRData.command, HEX);

    Serial.println("---------------------");

    // Xử lý theo command (khớp với code phát của bạn)
    switch (IrReceiver.decodedIRData.command) {

      case 0:
        Serial.println("STATE: RED");
        digitalWrite(SIGNAL_OUT, LOW);
        break;

      case 1:
        Serial.println("STATE: RED_YELLOW");
        digitalWrite(SIGNAL_OUT, LOW);
        break;

      case 2:
        Serial.println("STATE: GREEN");
        digitalWrite(SIGNAL_OUT, HIGH);
        break;

      case 3:
        Serial.println("STATE: GREEN_FLASH");
        digitalWrite(SIGNAL_OUT, HIGH);
        break;

      case 4:
        Serial.println("STATE: YELLOW");
        digitalWrite(SIGNAL_OUT, HIGH);
        break;
    }

    IrReceiver.resume(); // Chuẩn bị nhận gói tiếp theo
  }
}