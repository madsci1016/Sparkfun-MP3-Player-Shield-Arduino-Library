/**************************************
*
*  Example for Sparkfun MP3 Shield Library
*      By: Bill Porter
*      www.billporter.info
*
*   Function:
*      This sketch listens for commands from a serial terminal (like the Serial Monitor in
*      the Arduino IDE). If it sees 1-9 it will try to play an MP3 file named track00x.mp3
*      where x is a number from 1 to 9. For eaxmple, pressing 2 will play 'track002.mp3'.
*      A lowe case 's' will stop playing the mp3.
*      'f' will play an MP3 by calling it by it's filename as opposed to a track number.
*
*      Sketch assumes you have MP3 files with filenames like
*      "track001.mp3", "track002.mp3", etc on an SD card loaded into the shield.
*
***************************************/

#include <SPI.h>

//Add the SdFat Libraries
#include <SdFat.h>
#include <SdFatUtil.h>

//and the MP3 Shield Library
#include <SFEMP3Shield.h>

// Below is not needed if interrupt driven. Safe to remove if not using.
#if defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_Timer1
  #include <TimerOne.h>
#elif defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer
  #include <SimpleTimer.h>
#endif

//create and name the library object
SFEMP3Shield MP3player;

byte temp;
byte result;

char title[30];
char artist[30];
char album[30];
uint8_t mp3_vol = 40;

void setup() {
  Serial.begin(115200);

  Serial.print(F("Free RAM = ")); // available in Version 1.0 F() bases the string to into Flash, to use less SRAM.
  Serial.print(FreeRam(), DEC);  // FreeRam() is provided by SdFatUtil.h
  Serial.println(F(" Should be a base line of 1006, on ATmega328 when using INTx"));


  //boot up the MP3 Player Shield
  result = MP3player.begin();
  //check result, see readme for error codes.
  if(result != 0) {
    Serial.print(F("Error code: "));
    Serial.print(result);
    Serial.println(F(" when trying to start MP3 player"));
  }

  Serial.println(F("Enter 1-9 to play a track, s to stop playing, +/- to change volume or d for directory"));
}

void loop() {

// Below is only needed if not interrupt driven. Safe to remove if not using.
#if defined(USE_MP3_REFILL_MEANS) \
    && ( (USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer) \
    ||   (USE_MP3_REFILL_MEANS == USE_MP3_Polled)      )

  MP3player.available();
#endif

  if(Serial.available()) {
    temp = Serial.read();

    Serial.print(F("Received command: "));
    Serial.write(temp);
    Serial.println(F(" "));

    //if s, stop the current track
    if (temp == 's') {
      MP3player.stopTrack();

    //if 1-9, play corresponding track
    } else if (temp >= '1' && temp <= '9') {
      //convert ascii numbers to real numbers
      temp = temp - 48;

      //tell the MP3 Shield to play a track
      result = MP3player.playTrack(temp);

      //check result, see readme for error codes.
      if(result != 0) {
        Serial.print(F("Error code: "));
        Serial.print(result);
        Serial.println(F(" when trying to play track"));
      }

      Serial.println(F("Playing:"));

      //we can get track info by using the following functions and arguments
      //the functions will extract the requested information, and put it in the array we pass in
      MP3player.trackTitle((char*)&title);
      MP3player.trackArtist((char*)&artist);
      MP3player.trackAlbum((char*)&album);

      //print out the arrays of track information
      Serial.write((byte*)&title, 30);
      Serial.println();
      Serial.print(F("by:  "));
      Serial.write((byte*)&artist, 30);
      Serial.println();
      Serial.print(F("Album:  "));
      Serial.write((byte*)&album, 30);
      Serial.println();

    //if +/- to change volume
    } else if ((temp == '-') || (temp == '+')) {
      if (temp == '-') { // note dB is negative
        if (mp3_vol >= 254) { // range check
          mp3_vol = 254;
        } else {
          mp3_vol += 2; // keep it simpler with whole dB's
        }
      } else {
        if (mp3_vol <= 2) { // range check
          mp3_vol = 2;
        } else {
          mp3_vol -= 2;
        }
      }
      MP3player.SetVolume(mp3_vol, mp3_vol); // commit new volume
      Serial.print(F("Volume changed to -"));
      Serial.print(mp3_vol>>1, 1);
      Serial.println(F("[dB]"));

    /* Alterativly, you could call a track by it's file name by using playMP3(filename);
    But you must stick to 8.1 filenames, only 8 characters long, and 3 for the extension */
    } else if (temp == 'f') {
      //create a string with the filename
      char trackName[] = "track001.mp3";
      
      //tell the MP3 Shield to play that file
      result = MP3player.playMP3(trackName);
      //check result, see readme for error codes.
      if(result != 0) {
        Serial.print(F("Error code: "));
        Serial.print(result);
        Serial.println(F(" when trying to play track"));
      }

    /* Display the file on the SdCard */
    } else if (temp == 'd') {
      if(!MP3player.isPlaying()) {
        // prevent root.ls when playing, something locks the dump. but keeps playing.
        // yes, I have tried another unique instance with same results.
        // something about SdFat and its 500byte cache.
        Serial.println(F("Files found (name date time size):"));
        SFEMP3Shield::root.ls(LS_R | LS_DATE | LS_SIZE);
      } else {
        Serial.println(F("Busy Playing Files, try again later."));
      }

    /* Get and Display the Audio Information */
    } else if (temp == 'g') {
    	MP3player.getAudioInfo();
    }

    // print prompt after key stroke has been processed.
    Serial.println(F("Enter 1-9,s,d,+,- :"));
  }

  delay(100);
}
