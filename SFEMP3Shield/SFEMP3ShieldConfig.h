/**
\file SFEMP3ShieldConfig.h

\brief Hardware dependent configuration definitions
\remarks comments are implemented with Doxygen Markdown format

This SFEMP3ShieldConfig.h helps configure the SFEMP3Shield library for
various supported different base Arduino boards and shield's using the
VS10xx chip. It is possible this may support other VS10xx chips. But are
unverified at this time.

As the name SFEMP3Shield implies this driver was originally developed from
Sparkfun's MP3 Player Shield. Whereas it can and has been easily adapted
to other hardware, both base Arduino's and shield's using the VS10xx.

The default configuration of this library assumes the SFE MP3 Shield, on
an UNO/Duemilanove, when left un-altered.

\note
Support forArduino Mega's REQUIRES additional jumpers. As the SPI are not on the same
pins as the UNO/Duemilanove.
When using a mega with SFE compatible shields jump the following pings :
<tt>
\n Mega's 51 to the MP3's D11 for MOSI
\n Mega's 50 to the MP3's D12 for MISO
\n Mega's 52 to the MP3's D13 for SCK
</tt>
\n The remainder of pins may remain unchanged. Including INT0 as the Mega maps
INT0 to D2 as to support USE_MP3_INTx, as is. Like the Uno.
\n Where the default SFEMP3ShieldConfig.h should not need changing.
\n Yes, SdFat's SoftSPI.h was tried, but has problems when used twice once with
Sd2Card.cpp and a 2nd time with SFEMP3Shield.cpp.

\n

\note
Support for Arduino Leonardo is afflicted by having the SPI pins not routing the same pins as the UNO. This is similar to the Arduino Mega. Where as it appears it should simply work with additional jumpers, from the Leonardo's ICSP port, which has the SPI pins to the MP3 shields equivalent SPI pins.
<tt>
\n Leo's ICSP4 to the MP3's D11 for MOSI
\n Leo's ICSP1 to the MP3's D12 for MISO
\n Leo's ICSP3 to the MP3's D13 for SCK
</tt>
\n and remember to \b NOT use D10 as an input. It must be left as output.

\todo Please let us know if this works? I think it should.

\sa SEEEDUINO as to how to configure for Seeeduino's Music Shield.
\sa GRAVITECH as to how to configure for Gravitech's MP3-4NANO Shield.
 */

#ifndef SFEMP3ShieldConfig_h
#define SFEMP3ShieldConfig_h

//------------------------------------------------------------------------------

/**
 * \def SEEEDUINO
 * \brief A macro to configure use on a Seeeduino MP3 player shield
 *
 * Seeduino MP3 Players is supported. However, its DREQ is not connected to a
 * hard INT(x) pin, hence it MUST be polled. This can be configured, using
 * USE_MP3_SimpleTimer.
 * When using a Seeeduino MP3 Player shield set the below define of SEEEDUINO
 * to 1. As so the correct IO pins are configured MP3_XCS, MP3_XDCS and MP3_DREQ
 *
 * Along with USE_MP3_REFILL_MEANS should not be USE_MP3_INTx, unless extra
 * jumper wires are used.
 *
 * Set \c SEEEDUINO to \c 0 to use on a SparkFun MP3 player shield
 *
 * Set \c SEEEDUINO to \c 1 to use on a Seeeduino MP3 player shield
 */
#define SEEEDUINO 0 // set to 1 if using the Seeeduino Music Shield

/**
 * \def GRAVITECH
 * \brief A macro to configure use on a Gravitech's MP3-4NANO shield
 *
 * Gravitech's MP3-4NANO shield is supported. However, its chip select of the
 * SdCard connected to D4. This can be configured, simply by setting the below
 * define of GRAVITECH to 1.
 *
 * Set \c GRAVITECH to \c 0 to use on a Gravitech's MP3-4NANO shield
 *
 * Set \c GRAVITECH to \c 1 to use on a Gravitech's MP3-4NANO
 */
#define GRAVITECH 0 // set to 1 if using the Gravitech's MP3-4NANO shield

/**
 * \def TEENSY2
 * \brief Macro to configure pins for connecting the Sparkfun shield to a Teensy 2
 *
 * You can connect the Sparkfun Mp3 shield to a Teensy 2 with jumper cables on a
 * breadboard. Teensy SDI pins are CS=0, SCK=1, MOSI=2, MISO=3. If you are using
 * a Teensy 2 then set TEENSY2 to 1 below and scroll down for pin assignments.
 *
 * Set \c TEENSY2 to \c 0 to use on a Gravitech's MP3-4NANO shield
 *
 * Set \c TEENSY2 to \c 1 to use on a Gravitech's MP3-4NANO
 */
#define TEENSY2 0 // set to 1 if using the Sparkfun Mp3 shield with Teensy 2

/**
 * \def BARETOUCH
 * \brief A macro to explicitly configure use with the Bare Conductive Touch Board
 *
 * Bare Conductive's Touch Board is supported. However, its pin mapping is 
 * significantly different to the SparkFun MP3 player shield.
 *
 * If you are using Arduino 1.5.0+ then automatic pin remapping can be enabled
 * as follows:
 *
 * 1. Download the Bare Conductive board definitions file (boards.txt) from
 *    their Github (https://github.com/bareconductive).
 * 2. Extract the Bare Conductive folder into your Documents/Arduino/Hardware
 *    folder (My Documents\Arduino\Hardware on Windows). If the folder does
 *    not already exist, create it.
 * 3. Restart Arduino if it is currently running.
 * 4. In the Arduino menu, select Tools -> Board -> Bare Conductive Touch Board
 * 
 * This will automatically set up this library when the board is selected, and
 * revert back to the setting for the Sparkfun MP3 shield when it is not. If you
 * would like to override this, set the BARETOUCH value below: 0 to use the
 * Sparkfun MP3 shield, 1 to use the Bare Conductive Touch Board.
 *
 * If you are using an earlier version of Arduino, you will have to manage the
 * pin remapping manually. Setting BARETOUCH below to 0 will leave the pin map
 * as normal - i.e. for the Sparkfun MP3 shield. Setting it to 1 will map the 
 * pins correctly for the Bare Conductive Touch Board. If you decide to then
 * use a different board, you'll have to remember to come back here and adjust 
 * the settings accordingly.
 * 
 */
#define BARETOUCH 0 // set to 1 to force Bare Conductive Touch Board settings on

//------------------------------------------------------------------------------
/*
 * MP3 Player Shield pin mapping. See the appropiate schematic
 */

/**
 * \def MP3_XCS
 * \brief A macro to configure the XCS pin
 *
 * VS10xx's Control Chip Select Pin (for accessing SPI Control/Status registers)
 * as seen by the the Arduino
 *
 */

/**
 * \def MP3_XDCS
 * \brief A macro to configure the XDCS pin
 *
 * VS10xx's Data Chip Select Pin (for streaming data back and forth)
 * as seen by the the Arduino
 */

/**
 * \def MP3_DREQ
 * \brief A macro to configure the DREQ pin
 *
 * VS10xx's DREQ pin that indicates when it is clear to send more data.
 * aka Data REQuest.
 * as seen by the the Arduino
 */

/**
 * \def MP3_DREQINT
 * \brief A macro to configure the DREQINT pin
 *
 * The associated INT(X) pin name for the associated pin of DREQ, if used.
 * as seen by the the Arduino
 *
 * This may not be needed when USE_MP3_REFILL_MEANS is not equal to USE_MP3_REFILL_MEANS
 *
 * \sa USE_MP3_REFILL_MEANS
 */

/**
 * \def MP3_RESET
 * \brief A macro to configure the RESET pin
 *
 * VS10xx's RESET Pin
 * as seen by the the Arduino
 */

/**
 * \def SD_SEL
 * \brief A macro to configure the SdCard Chip Select for SFEMP3SHield library
 *
 * This is the pin of the Arduino that is connected to the SdCards Chip select pin.
 * This pin should be the same pin assigned in SdFat Library.
 * as seen by the the Arduino
 */

/**
 * \def PERF_MON_PIN
 * \brief A macro to configure a Pin to analyze performance
 *
 * The output of this pin will be low, during the refill of the VSdsp, allowing measurement of the CPU utilization, required to sustain playing.
 *
 * Set value to any available digital output, including A0-5...
 *
 * Set value to negative to disable.
 */
#define PERF_MON_PIN          -1 //  example of A5

#include <pins_arduino.h>

#if defined(__BIOFEEDBACK_MEGA__)
  #define MP3_XCS             67      //PK5 Output, Active Low,  Control Chip Select Pin (for accessing SPI Control/Status registers)
  #define MP3_XDCS            68      //PK6 Output, Active Low,  Data Chip Select / BSYNC Pin
  #define MP3_DREQ            66      //PK4 Input , Active High, Data Request Pin: Player asks for more data
  #define MP3_RESET           65      //PK3 Output, Active Low,  Reset is active low
  #define SD_SEL              76      //PJ6 Output, Active Low
  #define MP3_DREQINT          5 //Corresponding INTx for DREQ pin
#elif ( SEEEDUINO == 1 ) // if SEEDUINO use the following pin outs
  #define MP3_XCS             A3 //Control Chip Select Pin (for accessing SPI Control/Status registers)
  #define MP3_XDCS            A2 //Data Chip Select / BSYNC Pin
  #define MP3_DREQ            A1 //Data Request Pin: Player asks for more data
  //#define MP3_DREQINT        0 // There is no IRQ used on Seeduino
  #define MP3_RESET           A0 //Reset is active low
  #define SD_SEL              10 //select pin for SD card
#elif ( TEENSY2 == 1 )
  #define MP3_XCS              7
  #define MP3_XDCS             8
  #define MP3_DREQ             4
  #define MP3_DREQINT          1
  #define MP3_RESET            9
  #define SD_SEL               0 // Teensy SDI CS on pin 0
  // Connect SDI pins as follows:
  // Sparkfun shield 11 -> Teensy 2 (mosi)
  // Sparkfun shield 12 -> Teensy 3 (miso)
  // Sparkfun shield 13 -> Teensy 1 (sck)
// if BARETOUCH or ARDUINO_AVR_BARETOUCH use the following pin map
#elif (( BARETOUCH == 1 ) || ( ARDUINO_AVR_BARETOUCH == 1 )) 	
  #define MP3_XCS             9  //Control Chip Select Pin (for accessing SPI Control/Status registers)
  #define MP3_XDCS            6  //Data Chip Select / BSYNC Pin
  #define MP3_DREQ            7  //Data Request Pin: Player asks for more data
  #define MP3_DREQINT         4  //Corresponding INTx for DREQ pin
  #define MP3_RESET           8  //Reset is active low
  #define SD_SEL              5  //select pin for SD card	
// otherwise use pinout of typical Sparkfun MP3 Player Shield.
#else // otherwise use pinout of typical Sparkfun MP3 Player Shield.
  #define MP3_XCS              6 //Control Chip Select Pin (for accessing SPI Control/Status registers)
  #define MP3_XDCS             7 //Data Chip Select / BSYNC Pin
  #define MP3_DREQ             2 //Data Request Pin: Player asks for more data
  #if defined(__AVR_ATmega32U4__)
    #define MP3_DREQINT          1 //Corresponding INTx for DREQ pin
  #else // swapped between Uno and Leonardo.
    #define MP3_DREQINT          0 //Corresponding INTx for DREQ pin
  #endif
  #define MP3_RESET            8 //Reset is active low
  #if ( GRAVITECH == 1 )
    #define SD_SEL               4 //select pin for SD card
  #else
    #define SD_SEL               9 //select pin for SD card
  #endif // GRAVITECH
#endif // none SEEEDUINO

//------------------------------------------------------------------------------
/**
 * \def USE_MP3_REFILL_MEANS
 * \brief The selection of DREQ'ss refilling method.
 *
 * The value is that of an enumerated list of possible methods, aka means.
 *
 * The VS10xx's DREQ requests more data from the host micro (the Arduino)
 * Where it can either be an input to an interrupt and corresponding ISR or
 * be polled by software.
 *
 * To enable MP3 Player to use OTHER than default INT0 on D2 for refilling
 * uncomment or change the define of USE_MP3_REFILL_MEANS and set to desired method,
 *
 * The default when left commented implements INT0 on D2.
 *
 * \note ALL base Arduino's should support timers and soft polled means of either
 * \n USE_MP3_Polled, USE_MP3_Timer1 or USE_MP3_SimpleTimer means.
 * \n Assuming resources are not committed else where.
 *
 * \warning Remember to restart Arduino IDE for new Libraries to be available.
 * Coping the file is not enough.
 */
#define USE_MP3_REFILL_MEANS USE_MP3_INTx

/*
 * Configure the implemented means of Refilling the VS10xx chip
 */
#if defined(USE_MP3_REFILL_MEANS)

/**
 *\brief defacto Interrupt on INTx, from DREQ
 * \brief A macro of the enumerated value used to select hard interrupt INTx as the means to refill the VS10xx
 *
 * Where the Interrupt Service Routine attached to INTx as per attachInterrupt(MP3_DREQINT, refill, RISING)
 * causes execution of SFEMP3Shield::refill() as per the VS10xx need per DREQ.
 *
 * \note MP3_DREQINT corresponds the interrupt vector associated with the pin assigned to MP3_DREQ. As defined in WIterrupts.c
 *
 * \note INT(x) may be relocated or not be available on some base systems depending upon design.
 * Such as with the Lenoardo, which have pins D2/D3 and there corresponding INT0/INT1 swapped, versus the UNO.
 * See <a href="http://arduino.cc/en/Reference/AttachInterrupt"> attachInterrupt() </a>
 * Where SFE MP3 Player can use USE_MP3_INTx as DREQ is connected to D2 aka INT0.
 * Noting that MP3_DREQINT vector is defined above, in pin assignments.
 */
#define USE_MP3_INTx        0

/**
 * \brief A macro of the enumerated value used to select Software polling as the means to refill the VS10xx
 *
 * Where Main loop uses SFEMP3Shield::available() to check if DREQ needs refilling on a periodic basis.
 *
 * \note In this means SFEMP3Shield::available() is simply SFEMP3Shield::refill()
 * and MP3_REFILL_PERIOD is \em NOT used with this means.
 */
#define USE_MP3_Polled      1

/**
 * \brief A macro of the enumerated value used to select Timer1's interrupt as the means to refill the VS10xx
 *
 * Where the Interrupt Service Route attached to Timer1 cause periodic execution of SFEMP3Shield::refill()
 *
 * \note MP3_REFILL_PERIOD is required when using this means.
 *
 * \sa The use of USE_MP3_Timer1 interrupt requires the TimerOne.h library can be
 * downloaded from http://code.google.com/p/arduino-timerone/ for library.
 */
#define USE_MP3_Timer1      2

/**
 * \brief A macro of the enumerated value used to select Soft SimpleTimer period as the means to refill the VS10xx
 *
 * Where Main loop uses SFEMP3Shield::available() to check if DREQ needs refilling on a periodic basis.
 *
 * \note In this means SFEMP3Shield::available() is gating the excecution of SFEMP3Shield::refill()
 * based on MP3_REFILL_PERIOD. Where MP3_REFILL_PERIOD is required when using this means.
 *
 * \note The associated <SimpleTimer.h> library is required and utilizes 170 more bytes.
 *
 * \sa The use of USE_MP3_SimpleTimer interrupt requires the TimerOne.h library can
 * be downloaded from
 * http://www.arduino.cc/playground/Code/SimpleTimer#GetTheCode for library.
 */
#define USE_MP3_SimpleTimer 3

#endif

//------------------------------------------------------------------------------
/*
 * When means other than Polled and INTx are used the following Libraries need to be loaded.
 */
#if defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_Timer1
  #include <TimerOne.h>
// strange if TimerOne.h is present but not selected it still consums 6 bytes, something to do with Arduino's pre-compiler and Linker.
#elif defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer
  #include <SimpleTimer.h>
#endif


//------------------------------------------------------------------------------
/*
   When means are time based, need to define the period of update.
   100ms is recommened for 192K sample rate MP3. other rates may vary.
 */
#if defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS > USE_MP3_Polled // not needed if INTx is used or polled in loop.

/**
 * \brief A macro used to determine the number of milliseconds between software polls of the DREQ.
 */
#define MP3_REFILL_PERIOD 100
#endif

//------------------------------------------------------------------------------
/**
 * \def MIDI_CHANNEL
 * \brief A macro used to specify the MIDI channel
 *
 * Where used in the SingleMIDInoteFile array for sending quick beeps with function SFEMP3Shield::SendSingleMIDInote()
 *
 * \note Where Ch9 is reserved for Percussion Instruments with single note
 */
#define MIDI_CHANNEL             9 

/**
 * \def MIDI_NOTE_NUMBER
 * \brief A macro used to specify the MIDI note
 *
 * Where used in the SingleMIDInoteFile array for sending quick beeps with function SFEMP3Shield::SendSingleMIDInote()
 *
 * \note So for Ch9's the note is GM Bank Percussion Instrument, not actual note. e.g 56 is cowbell. This removes the necassasity to send other commands.
 */
#define MIDI_NOTE_NUMBER        56

/**
 * \def MIDI_NOTE_DURATION
 * \brief A macro used to specify the duration of the MIDI note
 *
 * Where used in the SingleMIDInoteFile array for sending quick beeps with function SFEMP3Shield::SendSingleMIDInote()
 *
 * \warning format is variable length, must keep it small. As not to break hardcoded header format
 */
#define MIDI_NOTE_DURATION     100


/**
 * \def MIDI_INTENSITY
 * \brief A macro used to specify the intensity of the MIDI note
 *
 * Value ranges from 0 to 127(full scale). Where used in the SingleMIDInoteFile array for sending both the ON and off of the quick beep with function SFEMP3Shield::SendSingleMIDInote()
 */
#define MIDI_INTENSITY         127 // Full scale.




#endif  // SFEMP3ShieldConfig_h
