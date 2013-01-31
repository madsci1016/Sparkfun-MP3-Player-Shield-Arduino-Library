/**
 * \file WebPlayer.ino
 *
 * \brief Example sketch of using the MP3Shield Arduino driver to create a webserver
 * \remarks comments are implemented with Doxygen Markdown format
 *
 * \author Michael P. Flaga
 * \author AdaFruit
 *
 * This sketch is a quick merge of http://github.com/adafruit/SDWebBrowse with
 * SFEMP3Shield library demonstrating the VS1053 commanded through a web browser.
 *
 * \warning
 * Need to set the following in the SFEMP3ShieldConfig.h, in order to work.
 * \code #define USE_MP3_REFILL_MEANS USE_MP3_Polled \endcode
 * as the Ethernet library makes interrupts not possible.
 * any one with a fix, let me know.
 */

#include <SdFat.h>
#include <SdFatUtil.h>
#include <Ethernet.h>
#include <SPI.h>

// Below is not needed if interrupt driven. Safe to remove if not using.
#if defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_Timer1
  #include <TimerOne.h>
#elif defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer
  #include <SimpleTimer.h>
#endif

#include <SFEMP3Shield.h>

//create and name the library object
SFEMP3Shield MP3player;
byte result;

/************ ETHERNET STUFF ************/
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//byte ip[] = { 192, 168, 0, 79 };
byte ip[] = { 172, 17, 67, 192 };
EthernetServer server(80);

/************ SDCARD STUFF ************/
SdFat sd;
SdFile file;

//------------------------------------------------------------------------------
/**
 * \brief Setup the Arduino Chip's feature for our use.
 *
 * After Arduino's kernel has booted initialize basic features for this
 * application, such as Serial port and MP3player objects with .begin.
 */
void setup() {
  Serial.begin(115200);

  Serial.print(F("Free RAM: "));
  Serial.println(FreeRam());

  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  pinMode(10, OUTPUT);                       // set the SS pin as an output (necessary!)
  digitalWrite(10, HIGH);                    // but turn off the W5100 chip!

  pinMode(8, OUTPUT);
  digitalWrite(8, LOW); // put VS1053 into reset

  if(!sd.begin(9, SPI_HALF_SPEED)) sd.initErrorHalt();
  if (!sd.chdir("/")) sd.errorHalt("sd.chdir");

  Serial.print(F("Volume is FAT"));
  Serial.println(sd.vol()->fatType(),DEC);
  Serial.println();

  // list file in root with date and size
  Serial.println(F("Files found in root:"));
  sd.ls(LS_DATE | LS_SIZE);
  Serial.println();

//  // Recursive list of all directories
//  Serial.println(F("Files found in all dirs:"));
//  sd.vwd()->ls(LS_R);
//  Serial.println();

  Serial.println(F("Done"));

  // Debugging complete, we start the server!
  Ethernet.begin(mac, ip);
  server.begin();

  result = MP3player.begin();
  //check result, see readme for error codes.
  if(result != 0) {
    Serial.println(F("Error code: "));
    Serial.print(result);
    Serial.println(F(" when trying to start MP3 player"));
    }
  MP3player.setVolume(80,80); // commit new volume  Serial.println(FreeRam());

}

//------------------------------------------------------------------------------
/**
 * \brief dump list of files to the web
 *
 * The SdFat does not have an easyway to go down the list of files and get there
 * names for direct handeling. Hence this does that.
 */
void ListFiles(EthernetClient client, uint8_t flags) {
  // This code is just copied from SdFile.cpp in the SDFat library
  // and tweaked to print to the client output in html!
  dir_t p;

  sd.vwd()->rewind();
  client.println(F("<ul>"));
  while (sd.vwd()->readDir(&p) > 0) {
    // done if past last used entry
    if (p.name[0] == DIR_NAME_FREE) break;

    // skip deleted entry and entries for . and  ..
    if (p.name[0] == DIR_NAME_DELETED || p.name[0] == '.') continue;

    // only list subdirectories and files
    if (!DIR_IS_FILE_OR_SUBDIR(&p)) continue;

    // print any indent spaces
    client.print(F("<li><a href=\""));
    for (uint8_t i = 0; i < 11; i++) {
      if (p.name[i] == ' ') continue;
      if (i == 8) {
        client.print('.');
      }
      client.print((char)p.name[i]);
    }
    client.print(F("\">"));

    // print file name with possible blank fill
    for (uint8_t i = 0; i < 11; i++) {
      if (p.name[i] == ' ') continue;
      if (i == 8) {
        client.print('.');
      }
      client.print((char)p.name[i]);
    }

    client.print(F("</a>"));

    if (DIR_IS_SUBDIR(&p)) {
      client.print('/');
    }

    // print modify date/time if requested
    if (flags & LS_DATE) {
       sd.vwd()->printFatDate(p.lastWriteDate);
       client.print(' ');
       sd.vwd()->printFatTime(p.lastWriteTime);
    }
    // print size if requested
    if (!DIR_IS_SUBDIR(&p) && (flags & LS_SIZE)) {
      client.print(' ');
      client.print(p.fileSize);
    }
    client.println(F("</li>"));
  }
  client.println(F("</ul>"));
}

// How big our line buffer should be. 100 is plenty!
#define BUFSIZ 100

//------------------------------------------------------------------------------
/**
 * \brief Main Loop the Arduino Chip
 *
 * This is called at the end of Arduino kernel's main loop before recycling.
 * And is where the user's serial input of bytes are read and analyzed by
 * parsed_menu.
 *
 * Additionally, if the means of refilling is not interrupt based then the
 * MP3player object is serviced with the availaible function.
 */
void loop()
{
  char clientline[BUFSIZ];
  int index = 0;

// Below is only needed if not interrupt driven. Safe to remove if not using.
#if defined(USE_MP3_REFILL_MEANS) \
    && ( (USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer) \
    ||   (USE_MP3_REFILL_MEANS == USE_MP3_Polled)      )

  MP3player.available();
#endif

  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean current_line_is_blank = true;

    // reset the input buffer
    index = 0;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        // If it isn't a new line, add the character to the buffer
        if (c != '\n' && c != '\r') {
          clientline[index] = c;
          index++;
          // are we too big for the buffer? start tossing out data
          if (index >= BUFSIZ)
            index = BUFSIZ -1;

          // continue to read more data!
          continue;
        }

        // got a \n or \r new line, which means the string is done
        clientline[index] = 0;

        // Print it out for debugging
        Serial.println(clientline);

        // Look for substring such as a request to get the root file
        if (strstr(clientline, "GET / ") != 0) {
          // send a standard http response header
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: text/html"));
          client.println();

          // print all the files, use a helper to keep it clean
          client.println(F("<h2>Files:</h2>"));
          ListFiles(client, LS_SIZE);

        } else if (strstr(clientline, "GET /") != 0) {
          // this time no space after the /, so a sub-file!
          char *filename;

          filename = clientline + 5; // look after the "GET /" (5 chars)
          // a little trick, look for the " HTTP/1.1" string and
          // turn the first character of the substring into a 0 to clear it out.
          (strstr(clientline, " HTTP"))[0] = 0;

          // print the file we want
          Serial.println(filename);

          char *get_arg;
          get_arg = strstr(filename, "?");
          Serial.println(get_arg);
          if (get_arg) {
            get_arg[0] = 0;
            get_arg += 5;
            Serial.println(F("get arg invoked "));
            Serial.println(get_arg);
            Serial.println(filename);
            client.println("HTTP/1.1 204 OK");
            client.println();

            // parse through web page commands

            //PLAY
            if (strstr(get_arg, "PLAY") != 0) {
              result = MP3player.playMP3(filename);
              if(result != 0) {
                Serial.print(F("Error code: "));
                Serial.print(result);
                Serial.println(F(" when trying to play track"));
              }

            // STOP
            } else if (strstr(get_arg, "STOP") != 0) {
              MP3player.stopTrack();

            //PAUSE
            } else if (strstr(get_arg, "PAUSE") != 0) {
              if( MP3player.getState() == playback) {
                MP3player.pauseMusic();
                Serial.println(F("Pausing"));
              } else if( MP3player.getState() == paused_playback) {
                MP3player.resumeMusic();
                Serial.println(F("Resuming"));
              } else {
                Serial.println(F("Not Playing!"));
              }

            // VOLUME
            } else if(strstr(get_arg, "VOLUME") != 0) {
              int8_t val = atol(strstr(get_arg, "VAL=") + 4);
              Serial.print(F("Val="));
              Serial.println(val, DEC);

              // push byte[1] into both left and right assuming equal balance.
              MP3player.setVolume(val, val); // commit new volume
              Serial.print(F("Volume changed to -"));
              Serial.print(val>>1, 1);
              Serial.println(F("[dB]"));

            // PLAY SPEED
            } else if(strstr(get_arg, "SPEED") != 0) {
              int16_t val = atol(strstr(get_arg, "VAL=") + 4);
              Serial.print(F("Val="));
              Serial.println(val, DEC);

              MP3player.setPlaySpeed(val); // commit new playspeed
              Serial.print(F("playspeed to "));
              Serial.println(val, DEC);

            }

            //
          } else if (  strstr(strlwr(filename), ".mp3")
             || strstr(strlwr(filename), ".aac")
             || strstr(strlwr(filename), ".wma")
             || strstr(strlwr(filename), ".wav")
             || strstr(strlwr(filename), ".fla")
             || strstr(strlwr(filename), ".mid")
            ) {

            // start of web page
            client.println(F("HTTP/1.1 200 OK"));
            client.println(F("Content-Type: text/html"));
            client.println(F("<html><head></head><body>"));

            client.println();
            client.print(F("<h2>Playing "));
            client.print(filename);
            client.println(F("</h2>"));
            client.println(F("Press back up to see menu."));

            client.println(F("<table>"));
            client.println(F("<tr><td>"));
            ButtonForm(client, "CMD", "PLAY");
            client.println(F("</td><td>"));
//            ButtonForm(client, "CMD", "PAUSE");
//            client.println(F("</td><td>"));
            ButtonForm(client, "CMD", "STOP");
            client.println(F("</table>"));

            client.println(F("<form method=get>"));
            client.println(F("<input type='submit' name='CMD' value='VOLUME'>"));
            client.println(F("<input id='VAL' name='VAL' type='range' min='0' max='100' >"));
            client.println(F("</form>"));

            client.println(F("<form method=get>"));
            client.println(F("<input type='submit' name='CMD' value='SPEED'>"));
            client.println(F("<input id='VAL' name='VAL' type='range' min='1' max='4' value ='1'>"));
            client.println(F("</form>"));

            Serial.print(F("Music file:"));
            Serial.println(filename);

          } else {
            if (! file.open(filename, O_READ)) {
              client.println(F("HTTP/1.1 404 Not Found"));
              client.println(F("Content-Type: text/html"));
              client.println();
              client.println(F("<h2>File Not Found!</h2>"));
              break;
            }

            Serial.println(F("Opened!"));

            client.println(F("HTTP/1.1 200 OK"));
//            client.println(F("Content-Type: text/plain"));
            client.println(F("Content-Type: "));
            Serial.println(F("Content-Type: "));
            if (strstr(strlwr(filename), ".htm") != 0) { //Sets content type.
              client.println(F("text/html"));
              Serial.println(F("text/html"));
            } else if (strstr(strlwr(filename), ".css") != 0) {
              client.println(F("text/css"));
              Serial.println(F("text/css"));
            } else if (strstr(strlwr(filename), ".png") != 0) {
              client.println(F("image/png"));
              Serial.println(F("image/png"));
            } else if (strstr(strlwr(filename), ".jpg") != 0) {
              client.println(F("image/jpeg"));
              Serial.println(F("image/jpeg"));
            } else if (strstr(strlwr(filename), ".gif") != 0) {
              client.println(F("image/gif"));
              Serial.println(F("image/gif"));
            } else if (strstr(strlwr(filename), ".3gp") != 0) {
              client.println(F("video/mpeg"));
              Serial.println(F("video/mpeg"));
            } else if (strstr(strlwr(filename), ".pdf") != 0) {
              client.println(F("application/pdf"));
              Serial.println(F("application/pdf"));
            } else if (strstr(strlwr(filename), ".js") != 0) {
              client.println(F("application/x-javascript"));
              Serial.println(F("application/x-javascript"));
            } else if (strstr(strlwr(filename), ".xml") != 0) {
              client.println(F("application/xml"));
              Serial.println(F("application/xml"));
            } else {
              client.println(F("text"));
              Serial.println(F("text"));
            }
            client.println();

            int16_t c;
            while ((c = file.read()) > 0) {
                // uncomment the serial to debug (slow!)
                Serial.print((char)c);
                client.print((char)c);
            }
            file.close();
          }
        } else {
          // everything else is a 404
          client.println(F("HTTP/1.1 404 Not Found"));
          client.println(F("Content-Type: text/html"));
          client.println();
          client.println(F("<h2>File Not Found!</h2>"));
        }
        break;
      }
    }
    // give the web browser time to receive the data
    delay(1000);
    client.stop();
  }
}

void ButtonForm(EthernetClient client, char* name, char* value) {
  client.print("<form method=get>");
//  client.print(F("<input type='hidden' name='")); client.print(name);     client.print(F("' value='")); client.print(value);  client.print(F("'>"));
  client.print(F("<input type='submit' name='")); client.print(name); client.print(F("' value='")); client.print(value);  client.print(F("'>"));
  client.println(F("</form>"));
}