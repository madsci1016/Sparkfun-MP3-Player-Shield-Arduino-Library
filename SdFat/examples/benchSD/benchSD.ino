/*
 * This sketch is a simple binary write/read benchmark
 * for the standard Arduino SD.h library.
 */
#include <SPI.h>
#include <SD.h>


// SD chip select pin
const uint8_t chipSelect = SS;

#define FILE_SIZE_MB 5
#define FILE_SIZE (1000000UL*FILE_SIZE_MB)
#define BUF_SIZE 100

uint8_t buf[BUF_SIZE];


// test file
File file;

//------------------------------------------------------------------------------
// store error strings in flash to save RAM
void error(char* s) {
  Serial.println(s);
  while(1);
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial){}  // wait for Leonardo
}
//------------------------------------------------------------------------------
void loop() {
  uint32_t maxLatency;
  uint32_t minLatency;
  uint32_t totalLatency;

  // discard any input
  while (Serial.read() >= 0) {}

  // pstr stores strings in flash to save RAM
  Serial.println("Type any character to start");
  while (Serial.read() <= 0) {}
  delay(400);  // catch Due reset problem
  
  if (!SD.begin(chipSelect)) error("begin");

  // open or create file - truncate existing file.
  file = SD.open("BENCH.DAT", FILE_WRITE | O_TRUNC);
//  file = SD.open("BENCH.DAT", O_CREAT | O_TRUNC | O_CREAT);
  if (!file) {
    error("open failed");
  }

  // fill buf with known data
  for (uint16_t i = 0; i < (BUF_SIZE-2); i++) {
    buf[i] = 'A' + (i % 26);
  }
  buf[BUF_SIZE-2] = '\r';
  buf[BUF_SIZE-1] = '\n';

  Serial.print("File size ");
  Serial.print(FILE_SIZE_MB);
  Serial.println("MB");
  Serial.print("Buffer size ");
  Serial.print(BUF_SIZE);
  Serial.println(" bytes");
  Serial.println("Starting write test.  Please wait up to a minute");

  // do write test
  uint32_t n = FILE_SIZE/sizeof(buf);
  maxLatency = 0;
  minLatency = 999999;
  totalLatency = 0;
  uint32_t t = millis();
  for (uint32_t i = 0; i < n; i++) {
    uint32_t m = micros();
    if (file.write(buf, sizeof(buf)) != sizeof(buf)) {
      error("write failed");
    }
    m = micros() - m;
    if (maxLatency < m) maxLatency = m;
    if (minLatency > m) minLatency = m;
    totalLatency += m;
  }
  file.flush();
  t = millis() - t;
  double s = file.size();
  Serial.print("Write ");
  Serial.print(s/t);
  Serial.print(" KB/sec\n");
  Serial.print("Maximum latency: ");
  Serial.print(maxLatency);
  Serial.print(" usec, Minimum Latency: ");
  Serial.print(minLatency);
  Serial.print(" usec, Avg Latency: ");
  Serial.print(totalLatency/n);
  Serial.print(" usec\n\n");
  Serial.print("Starting read test.  Please wait up to a minute\n");
  // do read test
  file.seek(0);
  maxLatency = 0;
  minLatency = 99999;
  totalLatency = 0;
  t = millis();
  for (uint32_t i = 0; i < n; i++) {
    buf[BUF_SIZE-1] = 0;
    uint32_t m = micros();
    if (file.read(buf, sizeof(buf)) != sizeof(buf)) {
      error("read failed");
    }
    m = micros() - m;
    if (maxLatency < m) maxLatency = m;
    if (minLatency > m) minLatency = m;
    totalLatency += m;
    if (buf[BUF_SIZE-1] != '\n') {
      error("data check");
    }
  }
  t = millis() - t;
  Serial.print("Read ");
  Serial.print(s/t);
  Serial.print(" KB/sec\n");
  Serial.print("Maximum latency: ");
  Serial.print(maxLatency);;
  Serial.print(" usec, Minimum Latency: ");
  Serial.print(minLatency);;
  Serial.print(" usec, Avg Latency: ");
  Serial.print(totalLatency/n);
  Serial.print(" usec\n\n");
  Serial.print("Done\n\n");
  file.close();
}