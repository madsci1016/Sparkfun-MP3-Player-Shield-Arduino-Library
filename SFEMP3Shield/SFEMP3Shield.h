/******************************************************************
*  Sparkfun Electronics MP3 Shield Library
*		details and example sketch: 
*			http://www.billporter.info/?p=1270
*
*		Brought to you by:
*              Bill Porter
*              www.billporter.info
*
*		See Readme for other info and version history
*	
*  
*This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
<http://www.gnu.org/licenses/>
*
*This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License. 
*To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/ or
*send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
******************************************************************/
#ifndef SFEMP3Shield_h
#define SFEMP3Shield_h

// inslude the SPI library:
#include "SPI.h"



//Not neccessary, but just in case. 
#if ARDUINO > 22
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

//Add the SdFat Libraries
#include <SdFat.h>
#include <SdFatUtil.h> 



static void refill();

void Mp3WriteRegister(uint8_t, uint8_t, uint8_t);
uint16_t Mp3ReadRegister (uint8_t);

//Create the variables to be used by SdFat Library
static Sd2Card card;
static SdVolume volume;
static SdFile root;
static SdFile track;
static uint8_t playing;

//buffer for music
static uint8_t mp3DataBuffer[32];

//MP3 Player Shield pin mapping. See the schematic
#define MP3_XCS 6 //Control Chip Select Pin (for accessing SPI Control/Status registers)
#define MP3_XDCS 7 //Data Chip Select / BSYNC Pin
#define MP3_DREQ 2 //Data Request Pin: Player asks for more data
#define MP3_DREQINT          0      //Corresponding INTx for DREQ pin
#define MP3_RESET 8 //Reset is active low
#define SD_SEL 9 //select pin for SD card

//VS10xx SCI Registers
#define SCI_MODE 0x00
#define SCI_STATUS 0x01
#define SCI_BASS 0x02
#define SCI_CLOCKF 0x03
#define SCI_DECODE_TIME 0x04
#define SCI_AUDATA 0x05
#define SCI_WRAM 0x06
#define SCI_WRAMADDR 0x07
#define SCI_HDAT0 0x08
#define SCI_HDAT1 0x09
#define SCI_AIADDR 0x0A
#define SCI_VOL 0x0B
#define SCI_AICTRL0 0x0C
#define SCI_AICTRL1 0x0D
#define SCI_AICTRL2 0x0E
#define SCI_AICTRL3 0x0F

//VS10xx SCI_MODE bitmasks
#define SM_RESET 0x04
#define SM_CANCEL 0x08

#define TRUE  1
#define FALSE  0

//tag location offsets
#define TRACK_TITLE 3
#define TRACK_ARTIST 33
#define TRACK_ALBUM 63


class SFEMP3Shield {
public:
uint8_t begin();
void SetVolume(uint8_t, uint8_t);
uint8_t playTrack(uint8_t);
uint8_t playMP3(char*);
void trackTitle(char*);
void trackArtist(char*);
void trackAlbum(char*);
void stopTrack();
uint8_t isPlaying();
bool skipTo(uint32_t);
uint32_t currentPosition();
void setBitRate(uint16_t);
void pauseDataStream();
void resumeDataStream();

private:
void getTrackInfo(uint8_t, char*);
uint8_t bitrate;
uint32_t start_of_music;
uint8_t VolL;
uint8_t VolR;
};

char* strip_nonalpha_inplace(char *s);


#endif