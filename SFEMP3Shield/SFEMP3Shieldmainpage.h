/**
\file SFEMP3Shieldmainpage.h

\brief Main Page of MarkDown Documentation
\remarks comments are implemented with Doxygen Markdown format

\mainpage Arduino SFEMP3Shield Library

\tableofcontents

<CENTER>Copyright &copy; 2012
</CENTER>

\section Intro Introduction
The Arduino SFEMP3Shield Library is a driver for VSLI's VS10xx, implemented as a Slave co-processor to audio decode streams of Ogg Vorbis/MP3/AAC/WMA/FLAC/WAVMIDI formats, across the SPI bus of the Arduino, along with mixing input signals. Principally this library is developed for the VS1053, where it may be compatible with other VS10xx's

Initial development was implemented on an Arduino 328 UNO/Duemilanove with a SparkFun MP3 Player Shield. Where additional support has been provided for Arduino base systems and Shields. \ref Hardware and documentation is provided as to how to implement these. Where this driver is modular in concept to allow ready porting to other Arduino or Wiring platforms.

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
  - 5V Arduino 328 UNO/Duemilanove or better.
  - Shield or break out with appropiate pins wired up.
  - Shield or break out for SdCard
  - SdCard FAT formatted with valid <A HREF = "http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Dev/Arduino/Shields/MP3_Player_Files.zip"> Audio files</A> and filenames
  - See \ref Hardware for alternative solutions.

\note This library was originally developed on IDE 0.2x, later ported to 1.x and best support on IDE 1.5.0+ going forward. Compatibility was lost with use of the Serial.print(F("...")) and can be restored by replacing it with SdFat's PgmPrint function

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

\subsection ArduinoBareTouch Arduino Bare Touch
Support for Bare Conductive's Touch Board is provided and documented in \ref SFEMP3ShieldConfig.h.

\note Reference \ref Arduino_Leonardo's note about example files.

\subsection ArduinoMega Arduino Mega Board
Support for Arduino and Seeeduino Mega's are documented in \ref SFEMP3ShieldConfig.h, which simply REQUIRES additional jumpers. As the SPI are not on the same pins as the UNO/Duemilanove.

\subsection Arduino_Leonardo Arduino Leonardo Board
Support for Arduino Leonardo's are afflicted by having the SPI and INT0 pins not routed to the same pins as the UNO/Duemilanove . This is similar to the Arduino Mega. Which simply REQUIRES additional jumpers, as documented in \ref SFEMP3ShieldConfig.h to correct the SPI. The swapping of INT0/INT1 is automatically corrected based on the Leonardo's processor type of __AVR_ATmega32U4__ being detected.

\note While the Leonardo has the same amount of physical program space as the UNO, it actually has less space available for use. As it uses approximately 4K, for core library USB features. Where there is adaquate space available for typical applications, the provided example files MP3Shield_Library_Demo.ino and FilePlayer.ino will compile a reduced set of examples based on the processor type of __AVR_ATmega32U4__ being detected. This is only for the demonstration, and other sketchs may call any of these functions, given its environment.

\subsection Arduino_Pro Arduino Pro 5V vs 3V
SFE Arduino Pro's while similar to UNO/Duemilanove's pin outs, they are available in either 5V or 3.3V. Where the SFE MP3 Player Shield requires 5V and locally generating the needed 3.3V and 1.8V for the VS10xx chip. Noting that 3.3V Pro's do not supply 5V, this causes a problem. It is possible to modify the shield as to use base Arduino supplied 3V's.

\subsection SparkFunMP3Player SparkFun MP3 Player Shield
SparkFun MP3 Player Shield should just work out of the box (bag) with a Arduino 328 UNO/Duemilanove, with Interrupts.

\subsection Seeeduino Seeduino MP3 Player Shield
Support for Seeduino MP3 Player Shield please see \ref SEEEDUINO and may require additional libraries, as per \ref Requirements

\subsection Gravitech MP3-4NANO Shield
Support for Gravitech MP3-4NANO shield please see \ref GRAVITECH

\subsection limitation Limitations.

- <b>The SPI Bus:</b>
The configuration of the VS10xx chip as a Slave on the SPI bus, along with the SdCard on that same bus master hosted by the Arduino. See \ref Performance

- <b>Non-Blocking:</b>
The controlling sketch needs to enquire via SFEMP3Shield::isPlaying as to determine if the current audio stream is finished or still playing. This is actually good and a result of the library being non-blocking, allowing the calling sketch to simply initiate the play of a desired audio stream from SdCard by simply calling playTrack or playMP3, of the desired file, and move on with other RealTime issues.

- <b>Multi-Chip VS10xx support:</b>
Not at this time. There are too many issues with member functions of the SFEMP3Shield class requiring to be static.

- <b>Audio Input</b>
Most commericially available shields at this time do not support either Line Level or Microphone Input. With the exception of the Seeeduino MP3 Shield and other home made shields. Where as the below admx____.053 and SFEMP3Shield::ADMixerLoad and SFEMP3Shield::ADMixerVol are provided for such devices. Otherwise the example MP3Shield_Library_Demo.ino has these lines commented out in setup(). As to reduce complications. To re-enable simply uncomment.

- <b>Recording</b>
As most commericially available shields do not support audio input this feature has not been implemented.
\todo
Support Audio Recording.

\section Performance Performance

Understanding that every byte streamed to the VS10xx needs also to be read from the SdCard over the same shared SPI bus, resulting in the SPI bus being less than half as efficient. Along with overhead. Depending upon the Bitrate of the file being streamed to the VSdsp, there is only so much Real Time available. This may impact the performance of high bit-rate audio files being streamed. Additionally the Play Speed Multiplier feature can be exhausted quickly. Where on a typical UNO there is plenty of real-time to transfer good quality files, with CPU to spare for other tasks, assuming they do not consume too much time either.

The available CPU can be increased by either or both increasing the speed of the SPI and or the Arduino F_CPU. Where the Speed of the SPI is individually maintained by both this driver and SdFatLib. As not to or be interfered with each other and or other libraries using the same SPI bus. The SdCard can be increased from SPI_HALF_SPEED to SPI_FULL_SPEED argument in the SD.begin. Where this library will set the Read and Write speeds to the VSdsp correspondingly, based on F_CPU of the Arduino.

The actual consumed CPU utilization can be measured by defining the \ref PERF_MON_PIN to a valid pin, which generates a low signal on configured pin while servicing the VSdsp. This is inclusive of the SdCard reads.

The below table show's typical average CPU utilizations of the same MP3 file that has been resampled to various bit rates and using different configurations. Where a significant difference is observed in performance.

| BitRate | SdCard | Refilling | IDLE |
| :-----: | :----: | :-------: | :--: |
|    128K |   Half |       12% |  88% | 
|    128K |   Full |       10% |  90% | 
|     96K |   Full |        7% |  93% | 
|     56K |   Full |        4% |  96% | 

\note Only F_CPU of 8MgHz and 16Hz are suppored. Others will default to SPI_CLOCK_DIV2, assuming 4MgHz.

\section Plug_Ins Plug Ins and Patches

The VS10xx chips are DSP's that run firmware out of ROM, that is internal to the VS10xx chip itself. Where the VSdsp's RAM can additionally be loaded with externally provided firmware and executed, also known as patches or plug-ins, over the SPI port and executed. This allows the VSdsp to have a method for both fixing problems that may exist in the factory ROM's firmware and or add new features provided by <A HREF = "http://www.vlsi.fi/en/support/software.html">VLSI's website</A>. It is even possible to write your own custom VSdsp code, using there Integrated Development Tools (VSIDE).

\em vs_plg_to_bin.pl is a perl script, that is provided in this library to run on your PC, to read and digest the .plg files converting them to raw binary as to be read by SFEMP3Shield::VSLoadUserCode() from the SdCard.
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

\note All plugins should be placed in the root of the SdCard.
\note \b patches.053 is a cumulative update correcting many known troublesome issues. Hence patches.053 is attempted in SFEMP3Shield::vs_init.
\note VSLI may post periodic updates on there <A HREF = "http://www.vlsi.fi/en/support/software.html">software website</A>
\note Perl is natively provided on Linux systems, and may be downloaded from <a href="http://www.activestate.com/activeperl/downloads">Active Perl </a> for windows systems.
\see about Analog to Digital Mixer (e.g. admx____.053) please note \ref limitation

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

- Is \b MP3player.begin() locking up, in setup()?
- Are you trying to update from a version prior to 1.01.00?
- Why does my Serial Monitor display: \"<tt>...do not have a sd.begin in the main sketch, See Trouble Shooting Guide.</tt>\"
- Compiler Error: \"<tt>...undefined reference to `sd'</tt>\"
- Is the last thing printed to the Serial Monitor: \"<tt>Free RAM = 1097 Should be a base line of 1095, on ATmega328 when using INTx</tt>\" then nothing...
  - Versions after 1.01.00 require the \c SdFat::begin() to be initialized in the main sketch.ino, as shown in the below example. This provides more immediate access to the SdCard's files by the main sketch. However, if not done there is no immediate compiler error and the sketch will lock up after as it attempts SFEMP3Shield::begin.
\code
...
SdFat sd; // newly required in 1.01.00 and higher
void setup() {
  if(!sd.begin(SD_SEL, SPI_HALF_SPEED)) sd.initErrorHalt(); // newly required in 1.01.00 and higher
  if(MP3player.begin() != 0) {Serial.print(F("ERROR"));
...
\endcode

- Is it \b FAT (FAT16 or FAT32)?
  - If the Error Code is indicating problems with the INIT, VOLUME or Track not being successful. It is recommend to use SdFat Example Library's QuickStart.ino as to see if it can access the card. Additionaly, SdInfo.ino may indicate if it can mount the card. Which may then need to formatted in FAT16 or FAT32. Where SdFormatter.ino can do this for you.

- Are the needed files on the \b root?
  - Remember to put patch and audio track files on the SdCard after formatting.
  - Are the filenames 8.3 format? See below warning.

- <tt>"Error code: 1 when \b trying to play track"</tt>
  - See the above \ref limitation about Non-Blocking.
  - Remember to check your audio cables and volume.

- <tt>"Warning: patch file not found, skipping."</tt>
  - See the \ref Plug_Ins

- Why do I only \b hear 1 second of music, or less?
  - This symptom is typical of the interrupt not triggering the SFEMP3Shield::refill(). I bet repeatidly sendnig a track number will advance the play about one second at a time, then stop.
  - What board is it? Check Hardware \ref limitation about Interrupts.
  - Are you trying the SFE provided <a href="&quot;http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Dev/Arduino/Shields/MP3_Player_Files.zip">test files</a> ? Or some homemade mp3 files? The SFE test files are nice as they are Immediately LOUD.
  - Interrupt problems may cause mp3 files that have a quiet lead in (or ramp up of volume) to be falsely diagnosed as not playing at all. Where the first 1 second may not be loud enough to be heard.

- <tt>Free RAM = 1090 Should be a base line of 1094</tt>
  - As a courtesy and good practice the provided example MP3Shield_Library_Demo.ino prints out the available remaining RAM, not statically allocated. And the actual available amount may depend on specific processor, IDE version, libraries and or other factors. A Uno built with IDE version 1.0.2 should have approximately 1094 bytes available from the example as is. And a Mega using a 2560 may show 6713, as it has more RAM.

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
1 *Failure of SdFat to initialize physical contact with the SdCard
2 *Failure of SdFat to start the SdCard's volume
3 *Failure of SdFat to mount the root directory on the volume of the SdCard
4 Other than default values were found in the SCI_MODE register.
5 SCI_CLOCKF did not read back and verify the configured value.
6 Patch was not loaded successfully. This may result in playTrack errors
</pre>
\deprecated Error codes 1,2,3 due to use of \c sd.begin() as global, starting version 1.1.0

\subsection playfunc Playing functions:
The following error codes return from the SFEMP3Shield::playTrack() or SFEMP3Shield::playMP3() member functions.
<pre>
0 OK
1 Already playing track
2 File not found
3 indicates that the VSdsp is in reset.
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