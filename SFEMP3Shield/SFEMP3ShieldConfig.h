/******************************************************************
This SFEMP3ShieldConfig.h helps configure the SFEMP3Shield library for 
various supported different base Arduino boards and shield's using the
VS1053 chip. It is possible this may support other VS10xx chips. But are
unverified at this time.

HARDWARE SUPPORT :
As the name SFEMP3Shield implies this driver was originally developed for 
Sparkfun's MP3 Player Shield. Whereas it can and has been easily adapted 
to other hardware, both base Arduino's and shield's using the VS10xx.

The default configuration of this library assumes the SFE MP3 Shield, on 
an UNO/Duemilanove, when left un-altered.
******************************************************************/
#ifndef SFEMP3ShieldConfig_h
#define SFEMP3ShieldConfig_h

//------------------------------------------------------------------------------
/**
 * MEGA:
 * Arduino Mega's REQUIRES additional jumpers. As the SPI are not on the same
 * pins as the UNO/Duemilanove.
 * When using a mega with SFE or Seeduino jump the following pings : 
 *   Mega's 51 to the MP3's D11 for MOSI,
 *   Mega's 50 to the MP3's D12 for MISO
 *   Mega's 52 to the MP3's D13 for SCK 
 * The remainder of pins may remain unchanged. Including INT0 as the Mega maps
 * INT0 to D2 as to support USE_MP3_INTx, as is. Like the Uno.
 *
 * Note: SdFat's SoftSPI.h was tried, but has problems when used twice once with
 * Sd2Card.cpp and a 2nd time with SFEMP3Shield.cpp.
 */

//------------------------------------------------------------------------------
/**
 * SEEEDUINO:
 * Seeduino MP3 Players is supported. However, its DREQ is not connected to a 
 * hard INT(x) pin, hence it MUST be polled. This can be configured below, using
 * USE_MP3_SimpleTimer.
 * When using a Seeeduino MP3 Player shield uncomment the below define of SEEEDUINO
 * Along with USE_MP3_REFILL_MEANS should not be USE_MP3_INTx, unless extra wires.
 */
//#defined SEEEDUINO  // uncomment if using the Seeeduino Music Shield

//------------------------------------------------------------------------------
/**
 * MP3 Player Shield pin mapping. See the appropiate schematic
 */
#if defined( SEEEDUINO )
#define MP3_XCS             A3 //Control Chip Select Pin (for accessing SPI Control/Status registers)
#define MP3_XDCS            A2 //Data Chip Select / BSYNC Pin
#define MP3_DREQ            A1 //Data Request Pin: Player asks for more data
//#define MP3_DREQINT        0 // There is no IRQ used on Seeduino
#define MP3_RESET           A0 //Reset is active low
#define SD_SEL              10 //select pin for SD card
#else // otherwise typical Arduino Uno/Duemilanove
#define MP3_XCS              6 //Control Chip Select Pin (for accessing SPI Control/Status registers)
#define MP3_XDCS             7 //Data Chip Select / BSYNC Pin
#define MP3_DREQ             2 //Data Request Pin: Player asks for more data
#define MP3_DREQINT          0 //Corresponding INTx for DREQ pin
#define MP3_RESET            8 //Reset is active low
#define SD_SEL               9 //select pin for SD card
#endif

//------------------------------------------------------------------------------
/**
 * The selection of DREQ and its refilling method.
 * To enable MP3 Player to use OTHER than default INT0 on D2 for refilling 
 * uncomment the below define of USE_MP3_REFILL_MEANS and set to desired method, 
 * described below. 
 * The default when left commented implements INT0 on D2.
 */
//#define USE_MP3_REFILL_MEANS USE_MP3_INTx

//------------------------------------------------------------------------------
/**
 * Configure the implemented means of Refilling the VS10xx chip
 */
#if defined(USE_MP3_REFILL_MEANS)
#define USE_MP3_INTx        0 // defacto Interrupt on INTx, from DREQ
#define USE_MP3_Polled      1 // simply polled from main loop checks DREQ on period.
#define USE_MP3_Timer1      2 // Interrupt on Timer1, timer IRQ checks DREQ on period.
#define USE_MP3_SimpleTimer 3 // Soft SimpleTimer period, main loop checks DREQ on period. Note this uses 170 more bytes.
#endif
/**
 * Note ALL base Arduino's should support timers and soft polled means of either
 * USE_MP3_Polled, USE_MP3_Timer1 or USE_MP3_SimpleTimer means. 
 * Assuming resources are not committed else where.
 *
 * Note INT(x) may not be available on some base systems depending upon design.
 * Where SFE MP3 Player can use USE_MP3_INTx as DREQ is connected to D2 or INT0.
 * Note that MP3_DREQINT vector is defined above, in pin assignments.
 *
 * The use of USE_MP3_Timer1 interrupt requires the TimerOne.h library can be 
 * downloaded from http://code.google.com/p/arduino-timerone/ for library.
 *
 * The use of USE_MP3_SimpleTimer interrupt requires the TimerOne.h library can
 * be downloaded from 
 * http://www.arduino.cc/playground/Code/SimpleTimer#GetTheCode for library.
 *
 * Remember to restart Arduino IDE for new Libraries to be available. 
 * Coping the file is not enough.
 */

//------------------------------------------------------------------------------
/**
 * When means other than Polled and INTx are used the following Libraries need to be loaded.
 */
#if defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_Timer1
  #include <TimerOne.h>
// strange if TimerOne.h is present but not selected it still consums 6 bytes, something to do with Arduino's pre-compiler and Linker.
#elif defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer
  #include <SimpleTimer.h>
#endif


//------------------------------------------------------------------------------
/**
   When means are time based, need to define the period of update.
   100ms is recommened for 192K sample rate MP3. other rates may vary.
 */
#if defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS > USE_MP3_Polled // not needed if INTx is used or polled in loop.
#define MP3_REFILL_PERIOD 100 // ms for the refill period
#endif

#endif  // SFEMP3ShieldConfig_h
