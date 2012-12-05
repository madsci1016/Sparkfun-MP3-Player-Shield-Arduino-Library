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

\sa SEEEDUINO as to how to configure for Seeeduino's Music Shield.
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
 *
 */
#define SEEEDUINO 0 // uncomment if using the Seeeduino Music Shield

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
 * as seen by the the Arduino */

/**
 * \def SD_SEL
 * \brief A macro to configure the SdCard Chip Select for SFEMP3SHield library
 *
 * This is the pin of the Arduino that is connected to the SdCards Chip select pin.
 * This pin should be the same pin assigned in SdFat Library.
 * as seen by the the Arduino */

// if SEEDUINO use the following pin outs
#if ( SEEEDUINO == 1 )
#define MP3_XCS             A3 //Control Chip Select Pin (for accessing SPI Control/Status registers)
#define MP3_XDCS            A2 //Data Chip Select / BSYNC Pin
#define MP3_DREQ            A1 //Data Request Pin: Player asks for more data
//#define MP3_DREQINT        0 // There is no IRQ used on Seeduino
#define MP3_RESET           A0 //Reset is active low
#define SD_SEL              10 //select pin for SD card
#else // otherwise use pinout of typical Sparkfun MP3 Player Shield.
#define MP3_XCS              6 //Control Chip Select Pin (for accessing SPI Control/Status registers)
#define MP3_XDCS             7 //Data Chip Select / BSYNC Pin
#define MP3_DREQ             2 //Data Request Pin: Player asks for more data
#define MP3_DREQINT          0 //Corresponding INTx for DREQ pin
#define MP3_RESET            8 //Reset is active low
#define SD_SEL               9 //select pin for SD card
#endif

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
 * \note MP3_DREQINT corresponds the interrupt vector associated with the pin assigned to MP3_DREQ.
 *
 * \note INT(x) may not be available on some base systems depending upon design.
 * Where SFE MP3 Player can use USE_MP3_INTx as DREQ is connected to D2 or INT0.
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

#endif  // SFEMP3ShieldConfig_h
