/**
 * \file MP3ButtonPlayer2.ino
 *
 * \brief Example sketch of using the MP3Shield Arduino driver using buttons,
 * with arduino recommended(simpler) debounce library
 * \remarks comments are implemented with Doxygen Markdown format
 *
 * \author Michael P. Flaga
 *
 * This sketch demonstrates the use of digital input pins used as buttons as 
 * NEXT, PLAY and STOP to control the tracks that are to be played.
 * Where PLAY or STOP will begin or cancel the stream of track000.mp3 through 
 * track999.mp3, as indexed by NEXT, begining with 0.

 * \note Use this example uses the bounce2 library to provide debouncing fuctions. Advocated by Arduino's website at http://playground.arduino.cc/code/bounce
 */

// libraries
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <SFEMP3Shield.h>
#include <Bounce2.h> 

/**
 * \breif Macro for the debounced NEXT pin, with pull-up
 */
#define B_NEXT  A0

/**
 * \breif Macro for the debounced STOP pin, with pull-up
 */
#define B_STOP  A1

/**
 * \breif Macro for the debounced PLAY pin, with pull-up
 */
#define B_PLAY  A2

/**
 * \breif Macro for the Debounce Period [milliseconds]
 */
#define BUTTON_DEBOUNCE_PERIOD 20 //ms

/**
 * \brief Object instancing the SdFat library.
 *
 * principal object for handling all SdCard functions.
 */
SdFat sd;

/**
 * \brief Object instancing the SFEMP3Shield library.
 *
 * principal object for handling all the attributes, members and functions for the library.
 */
SFEMP3Shield MP3player;

/**
 * \brief Object instancing the Next Button.
 */
Bounce b_Next  = Bounce();

/**
 * \brief Object instancing the Stop Button library.
 */
Bounce b_Stop  = Bounce();

/**
 * \brief Object instancing the Play Button library.
 */
Bounce b_Play  = Bounce();

/**
 * \brief Index of the current track playing.
 *
 * Value indicates current playing track, used to populate "x" for playing the 
 * filename of "track00x.mp3" for track000.mp3 through track254.mp3
 */
int8_t current_track = 0;

//------------------------------------------------------------------------------
/**
 * \brief Setup the Arduino Chip's feature for our use.
 *
 * After Arduino's kernel has booted initialize basic features for this
 * application, such as Serial port and MP3player objects with .begin.
 */
void setup() {
  Serial.begin(115200);

  pinMode(B_NEXT, INPUT_PULLUP);
  pinMode(B_STOP, INPUT_PULLUP);
  pinMode(B_PLAY, INPUT_PULLUP);

  b_Next.attach(B_NEXT);
  b_Next.interval(BUTTON_DEBOUNCE_PERIOD);
  b_Stop.attach(B_STOP);
  b_Stop.interval(BUTTON_DEBOUNCE_PERIOD);
  b_Play.attach(B_PLAY);
  b_Play.interval(BUTTON_DEBOUNCE_PERIOD);

  if(!sd.begin(9, SPI_HALF_SPEED)) sd.initErrorHalt();
  if (!sd.chdir("/")) sd.errorHalt("sd.chdir");

  MP3player.begin();
  MP3player.setVolume(10,10);
  
  Serial.println(F("Looking for Buttons to be depressed..."));
}


//------------------------------------------------------------------------------
/**
 * \brief Main Loop the Arduino Chip
 *
 * This is called at the end of Arduino kernel's main loop before recycling.
 * And is where the user's is executed.
 *
 * \note If the means of refilling is not interrupt based then the
 * MP3player object is serviced with the availaible function.
 */
void loop() {

// Below is only needed if not interrupt driven. Safe to remove if not using.
#if defined(USE_MP3_REFILL_MEANS) \
    && ( (USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer) \
    ||   (USE_MP3_REFILL_MEANS == USE_MP3_Polled)      )

  MP3player.available();
#endif

  if (b_Play.update()) {
    if (b_Play.read() == LOW)	{
      Serial.print(F("B_PLAY pressed, Start Playing Track # "));
      Serial.println(current_track);
      MP3player.playTrack(current_track);
    }
  }

  if (b_Stop.update()) {
    if (b_Stop.read() == LOW)	{
      Serial.print(F("B_STOP pressed, Stopping Track #"));
      Serial.println(current_track);
      MP3player.stopTrack();
    }
  }

  if (b_Next.update()) {
    if (b_Next.read() == LOW)	{
      Serial.print(F("B_NEXT pressed, Start Playing Next Track #"));
      Serial.println(++current_track);
      MP3player.stopTrack();
      MP3player.playTrack(current_track);
    }
  }

   //Do something. Have fun with it.

}
