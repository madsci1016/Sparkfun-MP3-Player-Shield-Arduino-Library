#include <SdFat.h>

//  create a serial output stream
ArduinoOutStream cout(Serial);

void setup() {
  Serial.begin(9600);
  cout << "Hello, World!\n";
}

void loop() {}
