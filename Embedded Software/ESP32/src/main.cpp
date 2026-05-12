#include <Arduino.h>
#include "communication/protocol.h"
#include "communication/commands.h"

void setup() {
    Serial.begin(115200); // Debug to PC
    Serial1.begin(115200, SERIAL_8N1, 17, 18); // Comms to STM32
    //delay(2000);
    Serial.println("Running...");
  }

void loop() {
    // This handles all receiving, CRC checking, and auto-replying in the background
    Protocol_PollSerial(&Serial1);
    delay(50);
    //Send_LCD("Hello!");
    //delay(100);
    //Send_FanUp();
    //delay(1000);
    //Climate_SendAmbientToggle();
    //delay(1000);
}
