/**
\file SFEMP3mainpage.h

\brief Main Page of MarkDown Documentation
\remarks comments are implemented with Doxygen Markdown format

\mainpage Arduino SFEMP3Shield Library

\tableofcontents

<CENTER>Copyright &copy; 2012
</CENTER>

\section Intro Introduction
The Arduino SFEMP3Shield Library is a driver for VSLI's VS10xx, implemented as a Slave co-processor to audio decode streams of Ogg Vorbis/MP3/AAC/WMA/FLAC/WAVMIDI formats, across the SPI bus of the Arduino. Principally this library is developed for the VS1053, where it may be compatible with other VS10xx's

Initial development was implemented on an Arduino 328 UNO/Duemilanove with a SparkFun MP3 Player Shield. Where additional support has been provided for Seeduino MP3 Player Shield. \ref Hardware and documentation is provided as to how to implement this and an Arduino Mega. Where this driver is modular in concept to allow ready porting to other Arduino or Wiring platforms.

\section  Contributors Contributors
\author  Nathan Seidle, www.sparkfun.com
\author  Bill Porter, www.billporter.info
\author  Michael Flaga, www.flaga.net
\author  ddz
\author  Wade Brainerd

\section Requirements  Requirements
- Software
  - Arduino 1.0 IDE or greater.
  - The most current <A HREF = "https://github.com/mpflaga/Sparkfun-MP3-Player-Shield-Arduino-Library.git" > SFEMP3Shield Driver </A>
  - SdFat Driver, while included a newer version may retrieved from <A HREF = "http://code.google.com/p/sdfatlib/" > SdFatLib' repository </A>.
  - Optionally, used if Hardware Interrupts are not supported:
    - <A HREF = "http://www.arduino.cc/playground/Code/SimpleTimer#GetTheCode" > SimpleTimer.h </A>. library can be downloaded from for library.
    - <A HREF = "http://code.google.com/p/arduino-timerone/" > TimerOne.h </A>. library can be downloaded from for library.
- Hardware
  - Arduino 328 UNO/Duemilanove or better.
  - Shield or break out with appropiate pins wired up.
  - Shield or break out for SdCard
  - SdCard FAT formatted with valid <A HREF = "http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Dev/Arduino/Shields/MP3_Player_Files.zip"> Audio files</A> and filenames

\note This library was originally developed on IDE 0.2x and later ported to 1.x. Compatibility was lost with use of the Serial.print(F("...")) and can be restored by replacing it with SdFat's PgmPrint function

\section  Installation Installation
 To install 1, 2, 3 !
 -# unzip and place './SFEMP3Shield' and './SdFat' folder into your 'C:/Users/{user name}/Documents/Arduino/libraries' folder or '{Arduino IDE path}/hardware/libraries" or {Arduino IDE path}/libraries" directory.
   - Along with SimpleTimer and or TimerOne if needed. Restart the Arduino IDE, and open up the example sketch.
 -# Copy the contents of this projects the './plugins' folder onto the root of the SdCard to be used. Along with desired audio files named appropriately; track001.mp3 through track009.mp3.
   - The plugins and audio files are \b not needed to be placed into the Arduino directories as they are not directly a part of the compiled project.
 -# Start IDE, load Example MP3Shield_Library_Demo.ino, select appropriate Board, Serial Port, Compile, load, run Serial Monitor at 115200 baud rate.
  - May need to Reset target Arduino and wait for Menu.

An example is provided in the SFEMP3Shield/examples folder.  Which is developed to test SFEMP3Shield and illustrate its various common uses.


\section Hardware Hardware

As mentioned the initial and principal support of this library is with Arduino 328 UNO/Duemilanove with a SparkFun MP3 Player Shield. Although various other boards and shields may be implemented by customing the \ref SFEMP3ShieldConfig.h file.

\subsection ArduinoMega Arduino Mega Board
Support for Arduino Mega's is documented \ref SFEMP3ShieldConfig.h, which simply REQUIRES additional jumpers. As the SPI are not on the same pins as the UNO/Duemilanove. \

\subsection Arduino_Leonardo Arduino Leonardo Board
Support for Arduino Leonardo's are afflicted by having the SPI and INT0 pins not routed to the same pins as the UNO. This is similar to the Arduino Mega. Where as it appears it could simply work with additional jumpers. Or by defining USE_MP3_REFILL_MEANS to USE_MP3_Polled. See <a href="http://arduino.cc/en/Reference/AttachInterrupt"> attachInterrupt() </a>.
\todo This is yet to be verified.

\subsection SparkFunMP3Player SparkFun MP3 Player Shield
SparkFun MP3 Player Shield should just work out of the box (bag) with a Arduino 328 UNO/Duemilanove, with Interrupts.

\subsection Seeeduino Seeduino MP3 Player Shield
Support for Seeduino MP3 Player Shield please see \ref SEEEDUINO and may require additional libraries, as per \ref Requirements

\subsection limitation Limitations.

- <b>The SPI Bus:</b>
The configuration of the VS10xx chip as a Slave on the SPI bus, along with the SdCard on that same bus master hosted by the Arduino. Understanding that every byte streamed to the VS10xx needs also to be read from the SdCard over the same shared SPI bus, results in the SPI bus being less than half as efficient. Along with overhead. This may impact the performance of high bit-rate audio files being streamed. Additionally the Play Speed Multiplier feature can be exhausted quickly.

- <b>Non-Blocking:</b>
The controlling sketch needs to enquire via SFEMP3Shield::isPlaying as to determine if the current audio stream is finished or still playing. This is actually good and a result of the library being non-blocking, allowing the calling sketch to simply initiate the play of a desired audio stream from SdCard by simply calling playTrack or playMP3, of the desired file, and move on with other RealTime issues.

- <b>Multi-Chip VS10xx support:</b>
Not at this time. There are too many issues with member functions of the SFEMP3Shield class requiring to be static.

\todo
There is a way to speed up digitalwrites, a principal causing delay, by either directly writing the I/O or perferably. using SdFat's atomwrite

\section Plug_Ins Plug Ins
vs_plg_to_bin.pl is a perl script that will read and digest the .plg files
and convert them to raw binary as to be read by SFEMP3Shield::VSLoadUserCode() from the SdCard.
Allowing updates to the VSDsp into its volatile memory after each reset.
These updates may be custom features or accumulated patches.

By storing them on the SdCard these plug-ins do not consume the Arduino's limited Flash spaces

Below are pre-compiled binary's of corresponding provided VSLI patches/plugins.
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

\note These plugins should be placed in the root of the SdCard.

\section Troubleshooting Troubleshooting

The below is a list of basic questions to ask when attempting to determine the problem.

- Did it initially \b PRINT the available RAM and Full Help Menu?
  - The MP3Shield_Library_Demo.ino example should initially provide a opening print indicating the amount of available SRAM and full menu help. If you don't see this the problem is between your Target and IDE. And likely not this library
  - Is Serial Monitor set to the correct tty or com port and 115200 baud rate? Did you change the baud rate?
  - Reset the Arduino after Serial Monitor is open or send any key. It may have printed these prior to the Serial Monitor being started.

- \b WHAT is the Error reported?
  - Is the Error Code is indicating a file problem. 
  - Are the filenames 8.3 format? See below warning.
  - See also \ref Error_Codes

- Did the SdCard \b LOAD?
  - Try reseating your SdCard.

- Is it \b FAT (FAT16 or FAT32)?
  - If the Error Code is indicating problems with the INIT, VOLUME or Track not being successful. It is recommend to use SdFat Example Library's QuickStart.ino as to see if it can access the card. Additionaly, SdInfo.ino may indicate if it can mount the card. Which may then need to formatted in FAT16 or FAT32. Where SdFormatter.ino can do this for you.

- Are the needed files on the \b root?
  - Remember to put patch and audio track files on the SdCard after formatting.
  - Are the filenames 8.3 format? See below warning.

- <pre>"Error code: 1 when \b trying to play track"</pre>
  - See the above \ref limitation about Non-Blocking.
  - Remember to check your audio cables and volume.

- Why do I only \b hear 1 second of music?
  - This symptom is typical of the interrupt not triggering the SFEMP3Shield::refill(). I bet repeatidly sendnig a track number will advance the play about one second at a time, then stop.
  - What board is it? Check Hardware \ref limitation about Interrupts.

\note This library makes extensive use of SdFat Library as to retrieve the stream of audio data from the SdCard. Notably this is where most failures occur. Where some SdCard types and manufacturers are not supported by SdFat. Though SdFat Lib is at this time, supporting most known cards.

\warning SdFatLib only supports 8.3 filenames. Long file names will not work.
Use the \c 'd' menu command to display directory contents of the SdCard. 
\c "longfilename.mp3" will be converted to \c "longfi~1\.mp3" . Where one can not predict the value of the 1. The DOS command of \c "dir \c /x" will list a cross reference, so that you know exactly, what is what.

\section Error_Codes Error Codes
Error Codes typically are returned from this Library's object's in place of Serial.print messages. As to both save Flash space and Serial devices may not always be present. Where it becomes the responsibility of the calling sketch of the library's object to appropiately react or display corresponding messages.

\subsection beginfunc begin function:
The following error codes return from the SFEMP3Shield::begin() member function.
<pre>
0 OK
1 Failure of SdFat to initialize physical contact with the SdCard
2 Failure of SdFat to start the SdCard's volume
3 Failure of SdFat to mount the root directory on the volume of the SdCard
4 Other than default values were found in the SCI_MODE register.
5 SCI_CLOCKF did not read back and verify the configured value.
6 Patch was not loaded successfully. This may result in playTrack errors
</pre>

\subsection playfunc Playing functions:
The following error codes return from the SFEMP3Shield::playTrack() or SFEMP3Shield::playMP3() member functions.
<pre>
0 OK
1 Already playing track
2 File not found
</pre>

\subsection skipTofunc Skip function:
The following error codes return from the SFEMP3Shield::skipTo()member function.
<pre>
0 OK
1 Not Playing track
2 Failed to skip to new file location
</pre>

\section comment Support
The code has been written with plenty of appropiate comments, describing key components, features and reasonings in Doxygen markdown style as to autogenerate this html suppoting document. Which is loaded into the repositories' gh-page branch to be displayed on the projects's GitHub Page.

Additional support may be reached from any of the following:
Please read through this document and refering linked resources.
- <A HREF = "http://www.billporter.info/forum/forum/sparkfun-mp3-shield-support-forum/" > Bills WordPress forum </A>
- <A HREF = "https://github.com/mpflaga/Sparkfun-MP3-Player-Shield-Arduino-Library/issues" > Flaga's github Issues forum </A>
- <A HREF = "https://github.com/madsci1016/Sparkfun-MP3-Player-Shield-Arduino-Library/issues" > Bill's github Issues forum </A>

\note Problems with SdCard initializing is outside the scope of this library. <A HREF = "http://code.google.com/p/sdfatlib/" > SdFatLib' repository </A> was selected being the best choice.

\section  References References
\see

<A HREF = "http://www.vlsi.fi/en/products/vs1053.html"> VS1053 Datasheet</A>.

<A HREF = "http://www.vsdsp-forum.com/phpbb/index.php"> VLSI's DSP support Forum</A>.

<A HREF = "http://www.vlsi.fi/en/support/software.html"> VLSI's software download of Apps, Patches, Plugins and tools</A>.

<A HREF = "http://www.vlsi.fi/fileadmin/app_notes/vlsi/vs10XXan_output.pdf"> VS10XX AppNote: Connecting analog outputs</A>.

<A HREF = "http://www.sparkfun.com/"> Sparkfun Electonics</A>

<A HREF = "https://www.sparkfun.com/products/10628"> MP3 Player Shield DEV-10628</A>.

<A HREF = "http://www.sparkfun.com/tutorials/295"> MP3 Player Shield Landing Page / Tutorials</A>.

<A HREF = "http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Dev/Arduino/Shields/MP3%20Shield-v13.pdf"> SFE's Schematic</A>.

<A HREF = "http://www.adafruit.com/"> Adafruit Industries:</A>.

<A HREF = "http://www.arduino.cc/"> The Arduino site</A>.

<A HREF = "http://www.atmel.com/dyn/resources/prod_documents/doc8161.pdf"> The ATmega328 datasheet</A>.

*/