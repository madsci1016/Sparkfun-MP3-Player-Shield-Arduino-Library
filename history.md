Revision History
---------------

## 1.02.15
* implemented 1.0.1 into repo

## 1.02.14
* implemented sdfatlib20131225 into repo

## 1.02.13
* ifdef'ed out space for Leonardo's less program space.
* made FilePlayer.ino more equal to that of MP3Shield_Library_Demo.ino

## 1.02.12
* implemented sdfatlib20130629 into repo
* removed unused code in FilePlayer.ino

## 1.02.11
* Added support for Bass Enhancer VSBE

## 1.02.10
* implemented sdfatlib20130313 into repo
* updated VLSI Patches and Plugins.
* fixed MP3ButtonPlayer2.ino example

## 1.02.09
* added demo of button using button2.h library for debounce
  (see gisthub for improved button.h library)

## 1.02.08
* added support for BareTouch pinout in config

## 1.02.07
* added SendSingleMIDInote() that sends a MIDI beep. It will suspend current playing stream to send beep and then resume prior stream

## 1.02.06
* added PERF_MON_PIN to enable allowing measurement of the CPU utilization and description of performance document

## 1.02.05
* implemented sdfatlib20130629 into repo
  * minor corrects of SdFatUtil's FreeRam() reporting correct value.

## 1.02.04
* improved SPI handling to guard against other SPI effects and speed
* increased SPI rate for 16Mg vs 8Mg FCPU, as to read at correct speeds.

## 1.02.03
* cleared interrupt during refill, if used. As to allow others. Such as timer0 was falling behind during the SdCard track.read(). 
* Along with displaying current second at command prompt, for verifying time.

## 1.02.02
* updated SdFatLib to sdfatlib20130313
* added const to PROGMEM uint16_t bitrate_table for avr-gcc 4.7.2 compatibility
* added ASCII range check to strip off non-ASCII, such as CR or LF, on FilePlayer.ino

## 1.02.01
* added new example FilePlayer.ino, more elaborate command to play all files.
* updated ram usage prints.
* added missing MinimumSerial files from new SdFatLib

## 1.02.00
* Roll up of all below changes for release

## 1.01.01
* added getState() as to report other possible states, such as paused but playing.
  * added check to enableRefill()
* added GetDifferentialOutput() and SetDifferentialOutput() feature to change the output,
     as to create a differential left/right output with a maximum output of 3V.
* changed case of get.. and set.. functions all to lower case for consistency.
* added VU meter support
* added chdir("/") to example and fixed enableTestSineWave freq
* Added skip, pauseMusic, resumeMusic and optional time offset to playMP3 along with examples.
* corrected typo's in Differential and initialized.
* updated test for mp3
* added example WebPlayer.ino
* sdfatlib20121219 replaced sdfatlib20120719


## 1.01.00
* changed sdFat to be instanced from INO file using sd.begin() for simpler use.
* added end() feature to put VS10xx into low power mode, along with corresponding checks.

## 1.00.02
* Fixed SkipTo() feature and added menu command.

## 1.00.01
* added support for Leonardo's interrupt switching and documentation.
* added documentation Gravitech's MP3-4NANO
* added Mono Mode and menu command, because of Nano's single speaker.
* added ADMixerLoad and ADMixerVol features

## 1.00.00
* formatted comments with Doxygen markdown.
* rearranged location of functions for organizing documentation
* extracted read of MP3 files bit-rate to member function.
* cleaned up some type casting.
* added history.md and license files
* improved tolerance of bit-rate read from mp3 file header.
* moved setting Playing to true after file is opened and bitrate is read.

## 0.09.00

* Added SFEMP3ShieldConfig.h to support alternate hardware for none INT0 DREQ based
     cards and or Shields. By using Timers, or software pollings, such as with Mega and Seediunos.

## 0.08.00

* moved MP3 functions into class and cleaned up syntax
* finished bitrate_table[] table with last row, that was missed.
* added "d" command to print directory of SdCard
* added "+/-" command to change volume by 1.0 dB
* added print of FreeRam() to show amount of static RAM available.
* save 220 bytes by using F() function to put strings into Flash and not use RAM:
*   i.e. Serial.Print(F("Hello)");
* note: FreeRam() is supplied with SdFatUtil.h

## 0.07.03
* Added apply patch/plugins from SdCard file to VS1xxx.

## 0.07.02
* Added quick check if trackname is mp3 extension.

## 0.07.01
* chomp'd non ASCII characters from file names.

## 0.07.00
* added functions to read track title,artist,album
* fixed silly use of static where it shouldn't have been

## 0.06.00
* fixed for Arduino Mega use by calling SDfatlib properly.
* Blame Nathan for bad implentation of SDFatlib

## 0.05.00
* added skipTo() and related functions to skip around in track

## 0.04.00
* added functions to cancel and resume external interrupt in case something else is on the SPI bus

## 0.03.00
* added isPlaying function to query shield status

## 0.02.00
* included pre-modified SDFat Library

## 0.01.00
* Initial Release, using external interrupt driven.

