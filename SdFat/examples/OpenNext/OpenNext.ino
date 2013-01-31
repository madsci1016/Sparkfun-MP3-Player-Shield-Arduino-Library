/*
 * Open all files in the root dir and print their filename and modify date/time
 */
#include <SdFat.h>

// SD chip select pin
const uint8_t chipSelect = SS;

// file system object
SdFat sd;

SdFile file;

// define a serial output stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {} // wait for Leonardo
  delay(1000);
  
  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();

  // open next file in root.  The volume working directory, vwd, is root
  while (file.openNext(sd.vwd(), O_READ)) {
    file.printName(&Serial);
    cout << ' ';
    file.printModifyDateTime(&Serial);
    cout << endl;
    file.close();
  }
  cout << "\nDone!" << endl;
}
//------------------------------------------------------------------------------
void loop() {}
