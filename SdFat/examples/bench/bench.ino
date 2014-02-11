/*
 * This sketch is a simple binary write/read benchmark.
 */
#include <SdFat.h>
#include <SdFatUtil.h>

// SD chip select pin
const uint8_t chipSelect = SS;

#define FILE_SIZE_MB 5
#define FILE_SIZE (1000000UL*FILE_SIZE_MB)
#define BUF_SIZE 100

uint8_t buf[BUF_SIZE];

// file system
SdFat sd;

// test file
SdFile file;

// Serial output stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial){}  // wait for Leonardo
  cout << pstr("\nUse a freshly formatted SD for best performance.\n");
}
//------------------------------------------------------------------------------
void loop() {
  uint32_t maxLatency;
  uint32_t minLatency;
  uint32_t totalLatency;

  // discard any input
  while (Serial.read() >= 0) {}

  // pstr stores strings in flash to save RAM
  cout << pstr("Type any character to start\n");
  while (Serial.read() <= 0) {}
  delay(400);  // catch Due reset problem
  
  cout << pstr("Free RAM: ") << FreeRam() << endl;

  // initialize the SD card at SPI_FULL_SPEED for best performance.
  // try SPI_HALF_SPEED if bus errors occur.
  if (!sd.begin(chipSelect, SPI_FULL_SPEED)) sd.initErrorHalt();

  cout << pstr("Type is FAT") << int(sd.vol()->fatType()) << endl;

  // open or create file - truncate existing file.
  if (!file.open("BENCH.DAT", O_CREAT | O_TRUNC | O_RDWR)) {
    error("open failed");
  }

  // fill buf with known data
  for (uint16_t i = 0; i < (BUF_SIZE-2); i++) {
    buf[i] = 'A' + (i % 26);
  }
  buf[BUF_SIZE-2] = '\r';
  buf[BUF_SIZE-1] = '\n';

  cout << pstr("File size ") << FILE_SIZE_MB << pstr("MB\n");
  cout << pstr("Buffer size ") << BUF_SIZE << pstr(" bytes\n");
  cout << pstr("Starting write test.  Please wait up to a minute\n");

  // do write test
  uint32_t n = FILE_SIZE/sizeof(buf);
  maxLatency = 0;
  minLatency = 9999999;
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
  file.sync();
  t = millis() - t;
  double s = file.fileSize();
  cout << pstr("Write ") << s/t << pstr(" KB/sec\n");
  cout << pstr("Maximum latency: ") << maxLatency;
  cout << pstr(" usec, Minimum Latency: ") << minLatency;
  cout << pstr(" usec, Avg Latency: ") << totalLatency/n << pstr(" usec\n\n");
  cout << pstr("Starting read test.  Please wait up to a minute\n");
  // do read test
  file.rewind();
  maxLatency = 0;
  minLatency = 9999999;
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
  cout << pstr("Read ") << s/t << pstr(" KB/sec\n");
  cout << pstr("Maximum latency: ") << maxLatency;
  cout << pstr(" usec, Minimum Latency: ") << minLatency;
  cout << pstr(" usec, Avg Latency: ") << totalLatency/n << pstr(" usec\n\n");
  cout << pstr("Done\n\n");
  file.close();
}