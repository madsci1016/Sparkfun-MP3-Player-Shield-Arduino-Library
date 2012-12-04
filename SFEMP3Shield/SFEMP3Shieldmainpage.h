/**
\file SFEMP3mainpage.h

\brief Main Page of MarkDown Documentation
\remarks implemented with Doxygen Markdown format

\mainpage Arduino SFEMP3Shield Library

\tableofcontents

<CENTER>Copyright &copy; 2012
</CENTER>

\section Intro Introduction
The Arduino SFEMP3Shield Library is a driver for VSLI's VS10xx, 
implemented as a Slave co-processor to audio decode streams of Ogg Vorbis/MP3/AAC/WMA/FLAC/WAVMIDI formats,
across the SPI bus of the Arduino. 
Principally this library is developed for the VS1053, where it may be compatible with other VS10xx's

Initial development was implemented on an Arduino 328 UNO/Duemilanove with a SparkFun MP3 Player Shield.
Where additional support has been provided for Seeduino MP3 Player Shield 
and documention is provied as to how to implement on a Arduino Mega. 
Where this driver is modular in concept to allow ready porting to other Arduino or Wiring platforms.

The main classes in SFEMP3Shield is SFEMP3Shield.

\section  Contributers Contributers
\author  Nathan Seidle, www.sparkfun.com
\author  Bill Porter, www.billporter.info
\author  Michael Flaga, www.flaga.net
\author  ddz
\author  Wade Brainerd

The SFEMP3Shield use SdFile class 

The SdVolume class supports FAT16 and FAT32 partitions.  Most applications
will not need to call SdVolume member function.

The Sd2Card class supports access to standard SD cards and SDHC cards.  Most
applications will not need to call Sd2Card functions.  
The Sd2Card class can be used for raw access to the SD card.

\todo
speed up digitalwrites by using SdFat's atomwrite

An example is provided in the SFEMP3Shield/examples folder.  Which is 
developed to test SFEMP3Shield and illustrate its various common uses.

\todo
%SdFat was developed for high speed data recording.  %SdFat was used to
implement an audio record/play class, WaveRP, for the Adafruit Wave Shield.
This application uses special Sd2Card calls to write to contiguous files in
raw mode.  These functions reduce write latency so that audio can be
recorded with the small amount of RAM in the Arduino.

\section SDcard SD\SDHC Cards

Arduinos access SD cards using the cards SPI protocol.  PCs, Macs, and
most consumer devices use the 4-bit parallel SD protocol.  A card that
functions well on A PC or Mac may not work well on the Arduino.

Most cards have good SPI read performance but cards vary widely in SPI
write performance.  Write performance is limited by how efficiently the
card manages internal erase/remapping operations.  The Arduino cannot
optimize writes to reduce erase operations because of its limit RAM.

SanDisk cards generally have good write performance.  They seem to have
more internal RAM buffering than other cards and therefore can limit
the number of flash erase operations that the Arduino forces due to its
limited RAM.

\section Hardware Hardware Configuration

SFEMP3Shield was developed using an
<A HREF = "http://www.sparkfun.com/"> Sparkfun Electonics</A> 
<A HREF = "https://www.sparkfun.com/products/10628"> MP3 Player Shield DEV-10628</A>.

The hardware interface to the SD card should not use a resistor based level
shifter.  %SdFat sets the SPI bus frequency to 8 MHz which results in signal
rise times that are too slow for the edge detectors in many newer SD card
controllers when resistor voltage dividers are used.

The 5 to 3.3 V level shifter for 5 V Arduinos should be IC based like the
74HC4050N based circuit shown in the file SdLevel.png.  The Adafruit Wave Shield
uses a 74AHC125N.  Gravitech sells SD and MicroSD Card Adapters based on the
74LCX245.

If you are using a resistor based level shifter and are having problems try
setting the SPI bus frequency to 4 MHz.  This can be done by using 
card.init(SPI_HALF_SPEED) to initialize the SD card.

\section comment Bugs and Comments

If you wish to report bugs or have comments, send email to fat16lib@sbcglobal.net.

\section SdFatClass SdFat Usage

%SdFat uses a slightly restricted form of short names.
Only printable ASCII characters are supported. No characters with code point
values greater than 127 are allowed.  Space is not allowed even though space
was allowed in the API of early versions of DOS.

Short names are limited to 8 characters followed by an optional period (.)
and extension of up to 3 characters.  The characters may be any combination
of letters and digits.  The following special characters are also allowed:

$ % ' - _ @ ~ ` ! ( ) { } ^ # &

Short names are always converted to upper case and their original case
value is lost.

\note
  The Arduino Print class uses character
at a time writes so it was necessary to use a \link SdFile::sync() sync() \endlink
function to control when data is written to the SD card.

\par
An application which writes to a file using print(), println() or
\link SdFile::write write() \endlink must call \link SdFile::sync() sync() \endlink
at the appropriate time to force data and directory information to be written
to the SD Card.  Data and directory information are also written to the SD card
when \link SdFile::close() close() \endlink is called.

\par
Applications must use care calling \link SdFile::sync() sync() \endlink
since 2048 bytes of I/O is required to update file and
directory information.  This includes writing the current data block, reading
the block that contains the directory entry for update, writing the directory
block back and reading back the current data block.

It is possible to open a file with two or more instances of SdFile.  A file may
be corrupted if data is written to the file by more than one instance of SdFile.


\section  References References
\see

<A HREF = "http://www.vlsi.fi/en/products/vs1053.html"> VS1053 Datasheet</A>.

<A HREF = "http://www.vsdsp-forum.com/phpbb/index.php"> VLSI's DSP support Forum</A>.

<A HREF = "http://www.vlsi.fi/en/support/software.html"> VLSI's software download of Apps, Patches, Plugins and tools</A>.

<A HREF = "http://www.vlsi.fi/fileadmin/app_notes/vlsi/vs10XXan_output.pdf"> VS10XX AppNote: Connecting analog outputs</A>.

<A HREF = "http://www.sparkfun.com/"> Sparkfun Electonics</A> 

<A HREF = "https://www.sparkfun.com/products/10628"> MP3 Player Shield DEV-10628</A>.

<A HREF = "http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Dev/Arduino/Shields/MP3%20Shield-v13.pdf"> SFE's Schematic</A>.

<A HREF = "http://www.adafruit.com/"> Adafruit Industries:</A>.

<A HREF = "http://www.arduino.cc/"> The Arduino site</A>.

<A HREF = "http://www.atmel.com/dyn/resources/prod_documents/doc8161.pdf"> The ATmega328 datasheet</A>.

\section  Installation Installation
To install, unzip and place 'SFEMP3Shield', 'SdFat' and 'plugins' folder into your 'C:\Users\{user name}\Documents\Arduino\libraries' folder or '{Arduino IDE path}\hardware\libraries" or {Arduino IDE path}\libraries" directory. 
Restart the Arduino IDE, and open up the example sketch. 

Most of the uses of the library are in the example sketch. 

\section Error_Codes Error_Codes
From the 'begin' function:
<pre>
0 OK
1 SD Card Init Failure
2 SD Card File System (FAT) init failure
3 SD Card Root Directory init failure
4 MP3 Decoder mode failure
5 MP3 Decoder speed failure
</pre>

From the 'playTrack' or 'playMP3' function:
<pre>
0 OK
1 Already playing track
2 File not found
</pre>

From the 'skipTo' function
<pre>
0 OK
1 Not Playing track
2 Failed to skip to new file location
</pre>

\section Plug_Ins Plug Ins

.\vs_plg_to_bin.pl is a perl script that will read and digest the .plg files
and convert them to raw binary as to be read by SFEMP3Shield::VSLoadUserCode() from the SdCard.
Allowing updates to the VSDsp into its volatile memory after each reset.
These updates may be custom features or accumilated patches. 

By storing them on the SdCard these plug-ins do not consume the Arduino's limited Flash spaces

Below are pre-compiled binarys of corresponding provided VSLI patches/plugins. 
The filenames are kept short as SdCard only support 8.3.

<pre>
.\\pcm.053       .\\vs1053-pcm110\\vs1053pcm.plg                 
.\\admxleft.053  .\\vs1053b-admix130\\admix-left.plg             
.\\admxmono.053  .\\vs1053b-admix130\\admix-mono.plg             
.\\admxrght.053  .\\vs1053b-admix130\\admix-right.plg            
.\\admxster.053  .\\vs1053b-admix130\\admix-stereo.plg           
.\\admxswap.053  .\\vs1053b-admix130\\admix-swap.plg             
.\\patchesf.053  .\\vs1053b-patches195\\vs1053b-patches-flac.plg 
.\\patches.053   .\\vs1053b-patches195\\vs1053b-patches.plg      
.\\rtmidi.053    .\\vs1053b-rtmidistart\\rtmidistart.plg         
.\\eq5.053       .\\vs1053b-eq5-090\\vs1053b-eq5.plg        
</pre>

These plugins should be placed in the root of the SdCard.



 */  
