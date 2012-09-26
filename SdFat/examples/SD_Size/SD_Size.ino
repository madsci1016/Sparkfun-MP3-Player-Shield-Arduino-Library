/*
 * Sketch to compare size of Arduino SD library with SdFat V2.
 * See SdFatSize.pde for SdFat sketch.
 */
#include <SD.h>

File file;
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  if (!SD.begin()) {
    Serial.println("begin failed");
    return;
  }
  file = SD.open("TEST_SD.TXT", FILE_WRITE);

  file.println("Hello");

  file.close();
}
//------------------------------------------------------------------------------
void loop() {}
