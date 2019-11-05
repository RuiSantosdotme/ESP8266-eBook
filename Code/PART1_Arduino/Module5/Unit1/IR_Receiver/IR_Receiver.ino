/*********
  https://github.com/markszabo/IRremoteESP8266/blob/master/examples/IRrecvDemo/IRrecvDemo.ino
  IRremoteESP8266: IRrecvDemo - demonstrates receiving IR codes with IRrecv
  An IR detector/demodulator must be connected to the input. Copyright 2009 Ken Shirriff, http://arcfn.com
  Version 0.2 June, 2017
  Complete project details at https://RandomNerdTutorials.com/home-automation-using-esp8266/
*********/

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board).
uint16_t RECV_PIN = 14;

IRrecv irrecv(RECV_PIN);

decode_results results;

void setup() {
  Serial.begin(115200);
  irrecv.enableIRIn();  // Start the receiver
}

void loop() {
  if (irrecv.decode(&results)) {
    // print() & println() can't handle printing long longs. (uint64_t)
    serialPrintUint64(results.value, HEX);
    Serial.println("");
    irrecv.resume();  // Receive the next value
  }
  delay(100);
}
