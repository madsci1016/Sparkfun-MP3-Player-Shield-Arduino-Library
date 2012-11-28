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

byte result;

char title[30];
char artist[30];
char album[30];

void setup() {
  Serial.begin(115200);

  Serial.print(F("Free RAM = ")); // available in Version 1.0 F() bases the string to into Flash, to use less SRAM.
  Serial.print(FreeRam(), DEC);  // FreeRam() is provided by SdFatUtil.h
  Serial.println(F(" Should be a base line of 1007, on ATmega328 when using INTx"));


  //boot up the MP3 Player Shield
  result = MP3player.begin();
  //check result, see readme for error codes.
  if(result != 0) {
    Serial.print(F("Error code: "));
    Serial.print(result);
    Serial.println(F(" when trying to start MP3 player"));
  }
  help();
}

void loop() {

// Below is only needed if not interrupt driven. Safe to remove if not using.
#if defined(USE_MP3_REFILL_MEANS) \
    && ( (USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer) \
    ||   (USE_MP3_REFILL_MEANS == USE_MP3_Polled)      )

  MP3player.available();
#endif

  if(Serial.available()) {
    parse_menu(Serial.read()); // get command from serial input
  }

  delay(100);
}

void parse_menu(byte key_command) {

  Serial.print(F("Received command: "));
  Serial.write(key_command);
  Serial.println(F(" "));

  //if s, stop the current track
  if (key_command == 's') {
    MP3player.stopTrack();

  //if 1-9, play corresponding track
  } else if (key_command >= '1' && key_command <= '9') {
    //convert ascii numbers to real numbers
    key_command = key_command - 48;

    //tell the MP3 Shield to play a track
    result = MP3player.playTrack(key_command);

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
  } else if ((key_command == '-') || (key_command == '+')) {
    union twobyte mp3_vol; // create key_command existing variable that can be both word and double byte of left and right.
    mp3_vol.word = MP3player.GetVolume(); // returns a double uint8_t of Left and Right packed into int16_t

    if (key_command == '-') { // note dB is negative
      // assume equal balance and use byte[1] for math
      if (mp3_vol.byte[1] >= 254) { // range check
        mp3_vol.byte[1] = 254;
      } else {
        mp3_vol.byte[1] += 2; // keep it simpler with whole dB's
      }
    } else {
      if (mp3_vol.byte[1] <= 2) { // range check
        mp3_vol.byte[1] = 2;
      } else {
        mp3_vol.byte[1] -= 2;
      }
    }
    // push byte[1] into both left and right assuming equal balance.
    MP3player.SetVolume(mp3_vol.byte[1], mp3_vol.byte[1]); // commit new volume
    Serial.print(F("Volume changed to -"));
    Serial.print(mp3_vol.byte[1]>>1, 1);
    Serial.println(F("[dB]"));

  //if < or > to change Play Speed
  } else if ((key_command == '>') || (key_command == '<')) {
    uint16_t playspeed = MP3player.GetPlaySpeed(); // create key_command existing variable
    // note playspeed of Zero is equal to ONE, normal speed.
    if (key_command == '>') { // note dB is negative
      // assume equal balance and use byte[1] for math
      if (playspeed >= 254) { // range check
        playspeed = 5;
      } else {
        playspeed += 1; // keep it simpler with whole dB's
      }
    } else {
      if (playspeed == 0) { // range check
        playspeed = 0;
      } else {
        playspeed -= 1;
      }
    }
    MP3player.SetPlaySpeed(playspeed); // commit new playspeed
    Serial.print(F("playspeed to "));
    Serial.println(playspeed, DEC);

  /* Alterativly, you could call a track by it's file name by using playMP3(filename);
  But you must stick to 8.1 filenames, only 8 characters long, and 3 for the extension */
  } else if (key_command == 'f') {
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
  } else if (key_command == 'd') {
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
  } else if (key_command == 'i') {
    MP3player.getAudioInfo();

  } else if (key_command == 'p') {
      MP3player.pauseDataStream();
      Serial.println(F("Pausing"));

  } else if (key_command == 'r') {
      MP3player.resumeDataStream();
      Serial.println(F("Resuming"));

  } else if (key_command == 't') {
      int8_t teststate = MP3player.enableTestSineWave(126);
    if (teststate == -1) {
      Serial.println(F("Un-Available while playing music."));
    } else if (teststate == 1) {
      Serial.println(F("Enabling Test Sine Wave"));
    } else if (teststate == 2) {
      MP3player.disableTestSineWave();
      Serial.println(F("Disabling Test Sine Wave"));
    }

  } else if (key_command == 'm') {
      uint16_t teststate = MP3player.memoryTest();
    if (teststate == -1) {
      Serial.println(F("Un-Available while playing music."));
    } else if (teststate == 2) {
      teststate = MP3player.disableTestSineWave();
      Serial.println(F("Un-Available while Sine Wave Test"));
    } else {
      Serial.print(F("Memory Test Results = "));
      Serial.println(teststate, HEX);
      Serial.println(F("Result should be 0x83FF."));
      Serial.println(F("Reset is needed to recover to normal operation"));
    }

  } else if (key_command == 'e') {
    uint8_t earspeaker = MP3player.GetEarSpeaker();
    if (earspeaker >= 3){
      earspeaker = 0;
    } else {
      earspeaker++;
    }
    MP3player.SetEarSpeaker(earspeaker); // commit new earspeaker
    Serial.print(F("earspeaker to "));
    Serial.println(earspeaker, DEC);

  } else if (key_command == 'h') {
    help();
  }

  // print prompt after key stroke has been processed.
  Serial.println(F("Enter 1-9,s,d,+,-,i>,<,p,r,t,m :"));
}

void help() {
  Serial.println(F("Arduino SFEMP3Shield Library Example:"));
  Serial.println(F(" courtesy of Bill Porter & Michael P. Flaga"));
  Serial.println(F("COMMANDS:"));
  Serial.println(F(" [1-9] to play a track"));
  Serial.println(F(" [s] to stop playing"));
  Serial.println(F(" [+ or -] to change volume"));
  Serial.println(F(" [> or <] to increament or decreament playspeed by 1 factor"));
  Serial.println(F(" [i] retreieve current audio information (partial list)"));
  Serial.println(F(" [e] increament Spatial EarSpeaker, default is 0, wraps after 4"));
  Serial.println(F(" [p] to pause."));
  Serial.println(F(" [r] to resume."));
  Serial.println(F(" [t] to toggle sine wave test"));
  Serial.println(F(" [m] perform memory test. reset is needed after to recover."));
  Serial.println(F(" [h] this help"));
}
