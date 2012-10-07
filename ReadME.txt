/******************************************************************
*  Sparkfun Electronics MP3 Shield Library v0.7
*		details and example sketch: 
*			http://www.billporter.info/?p=1270
*
*		Brought to you by:
*              Bill Porter
*              www.billporter.info
*
*	 Contributers:
*		Some original code from SFE head guru Nathan
*			But for making this pretty I think he owes me a beer now. 
*		Michael Flaga, doing most to the new dev work now. A better programer then I.
*		ddz
*		Wade Brainerd
*
*  Lib version history
*    0.1 made into library, external interrupt driven.
*    0.2 included pre-modified SDFat Library
*    0.3 added isPlaying function to query shield status
*    0.4 added functions to cancel and resume external interrupt
*        in case something else is on the SPI bus	
*    0.5 added skipTo() and related functions to skip around in track
*    0.6 fixed for Arduino Mega use by calling SDfatlib properly. 
*           Blame Nathan for bad implentation of SDFatlib
*    0.7 added functions to read track title,artist,album
*         fixed silly use of static where it shouldn't have been
*    0.7.1 chomp'd non ASCII characters from file names.
*    0.7.2 Added quick check if trackname is mp3 extension.
*    0.7.3 Added apply patch/plugins from SdCard file to VS1xxx.
*	
*
*
*This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
<http://www.gnu.org/licenses/>
*  
*
*This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License. 
*To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/ or
*send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
******************************************************************/


To install, unzip and place 'SFEMP3Shield' and 'SdFat' folder into your 'C:\Users\{user name}\Documents\Arduino\libraries' folder or '{Arduino IDE path}\hardware\libraries" or {Arduino IDE path}\libraries" directory. 
Restart the Arduino IDE, and open up the example sketch. 

Most of the uses of the library are in the example sketch. 


****************IF YOU HAVE PROBLEMS***********************

Check over your code very well, then drop me a comment at 
http://www.billporter.info/?p=1270 and I'll try to support.


****************Library Functions***************************

byte begin()  Starts library. Plass this in setup. Returns an error code if there's trouble

void SetVolume(byte Left, byte Right)  Sets volume in MP3 player, starts off at 40 for both. 

byte playTrack(byte TrackNumber)  Starts playback of an MP3 file named 'trackx.mp3' where x is 0-255 and passed in by you as 'TrackNumber'. Returns an error if there's trouble,

byte playMP3(char*)   Starts playback of a track with a name you pass in. Must follow 8.1 format. Returns an error if there's trouble. 

void stopTrack()  Stops the current track that's playing, if there is one.

byte isPlaying()  Return 1 if something is currently being played. 0 if nothing is playing. 

void skipTo(unsigned long timecode_in_ms)   Skips playback to certain time in ms in MP3 file. Not 100% accurate,
but decent. If it's way off, it means autodetect of bitrate failed and you need set it manually with setBitRate()

unsigned long currentPosition()    returns current timecode in ms of playing track. Not 100% accurate,
but decent. If it's way off, it means autodetect of bitrate failed and you need set it manually with setBitRate()

void setBitRate(unsigned int bitrate)    forces bitrate when autodetect failed. Call right after a play function, 
but before any function related to timecode. 

void trackTitle(char* buffer); 
void trackArtist(char* buffer);
void trackAlbum(char* buffer);       These functions read the MP3 track info from file and save it in buffer.
									See example. 

*****************Advanced Functions**********************

*****If you want to use the SPI bus for something else as well****
****** You need to wrap your SPI code in these functios:***********

void pauseDataStream();
 //Your code for accessing something else on the SPI bus
void resumeDataStream();
 
This avoid collisions that might occur if the MP3 chip triggers the interrupt while you are doing somethign else
on the SPI bus. If you take too long, the decoder will run out of data and stop. 

***********Advanced Functions********************

Write and read to direct MP3 decoder registers like this:

void Mp3WriteRegister(unsigned char addressbyte, unsigned char highbyte, unsigned char lowbyt)

uint16_t Mp3ReadRegister (unsigned char addressbyte);


****************Error Codes*****************************

From the 'begin' function:

0 OK
1 SD Card Init Failure
2 SD Card File System (FAT) init failure
3 SD Card Root Directory init failure
4 MP3 Decoder mode failure
5 MP3 Decoder speed failure

From the 'playTrack' or 'playMP3' function:

0 OK
1 Already playing track
2 File not found

From the 'skipTo' function

0 OK
1 Not Playing track
2 Failed to skip to new file location



****************Plug Ins*****************************
.\vs_plg_to_bin.pl is a perl script that will read the below plg files
and convert them to raw binary to be read by VSLoadUserCode().
This allows the larger accumilated patch to be loaded, without consuming AVR resources

Below are pre-compiled binarys of corresponding provided VSLI patches/plugins. 
The filenames are kept short as SdCard only support 8.3.

.\pcm.053       vs1053-pcm110\vs1053pcm.plg                 
.\admxleft.053  vs1053b-admix130\admix-left.plg             
.\admxmono.053  vs1053b-admix130\admix-mono.plg             
.\admxrght.053  vs1053b-admix130\admix-right.plg            
.\admxster.053  vs1053b-admix130\admix-stereo.plg           
.\admxswap.053  vs1053b-admix130\admix-swap.plg             
.\patchesf.053  vs1053b-patches195\vs1053b-patches-flac.plg 
.\patches.053   vs1053b-patches195\vs1053b-patches.plg      
.\rtmidi.053    vs1053b-rtmidistart\rtmidistart.plg         
.\eq5.053       vs1053b-eq5-090\vs1053b-eq5.plg             

