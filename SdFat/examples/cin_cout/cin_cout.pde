/*
 * Demo of ArduinoInStream and ArduinoOutStream
 */
#include <SdFat.h>

// create serial output stream
ArduinoOutStream cout(Serial);

// input buffer for line
char cinBuf[40];

// create serial input stream
ArduinoInStream cin(Serial, cinBuf, sizeof(cinBuf));
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
}
//------------------------------------------------------------------------------
void loop() {
  int32_t n;

  cout << "enter an integer\n";

  cin.readline();

  if (cin >> n) {
    cout << "The number is: " << n << endl;
  } else {
    // will fail if no digits or not in range [-2147483648, 2147483647]
    cout << "Invalid input: " << cinBuf << endl;
  }
  cout << endl;
}
