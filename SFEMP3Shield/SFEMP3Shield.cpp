#include "SFEMP3Shield.h"
// inslude the SPI library:
#include "SPI.h"
//avr pgmspace library for storing the LUT in program flash instead of sram
#include <avr/pgmspace.h>

//bitrate lookup table      V1,L1  V1,L2   V1,L3   V2,L1  V2,L2+L3
//190 bytes(!!); better to store in progmem or eeprom
uint16_t bitrate_table[15][6] PROGMEM = {
                 { 0,   0,  0,  0,  0,  0}, //0000
                 { 32, 32, 32, 32,  8,  8}, //0001
                 { 64, 48, 40, 48, 16, 16}, //0010
                 { 96, 56, 48, 56, 24, 24}, //0011
                 {128, 64, 56, 64, 32, 32}, //0100
                 {160, 80, 64, 80, 40, 40}, //0101
                 {192, 96, 80, 96, 48, 48}, //0110
                 {224,112, 96,112, 56, 56}, //0111
                 {256,128,112,128, 64, 64}, //1000
                 {288,160,128,144, 80, 80}, //1001
                 {320,192,160,160, 96, 69}, //1010
                 {352,224,192,176,112,112}, //1011
                 {384,256,224,192,128,128}, //1100
                 {416,320,256,224,144,144}, //1101
                 {448,384,320,256,160,160}  //1110
               };

// Initialize static class variables
Sd2Card  SFEMP3Shield::card;
SdVolume SFEMP3Shield::volume;
SdFile   SFEMP3Shield::root;
SdFile   SFEMP3Shield::track;
uint8_t  SFEMP3Shield::playing;
uint16_t SFEMP3Shield::spiRate;
#if defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer
  SimpleTimer timer;
  int timerId_mp3;
#endif

//buffer for music
uint8_t  SFEMP3Shield::mp3DataBuffer[32];

//Inits everything
uint8_t  SFEMP3Shield::begin(){

  pinMode(MP3_DREQ, INPUT);
  pinMode(MP3_XCS, OUTPUT);
  pinMode(MP3_XDCS, OUTPUT);
  pinMode(MP3_RESET, OUTPUT);

  cs_high();  //MP3_XCS, Init Control Select to deselected
  dcs_high(); //MP3_XDCS, Init Data Select to deselected
  digitalWrite(MP3_RESET, LOW); //Put VS1053 into hardware reset

  //Setup SD card interface
  //Initialize the SD card and configure the I/O pins.
  if (!card.init(SPI_FULL_SPEED, SD_SEL)) return 1; // Serial.println(F("Error: Card init"));
  //Initialize a volume on the SD card.
  if (!volume.init(&card)) return 2; //Serial.println(F("Error: Volume ini"));
  //Open the root directory in the volume.
  if (!root.openRoot(&volume)) return 3; //Serial.println(F("Error: Opening root")); //Open the root directory in the volume.

  //From section 7.6 of datasheet, max SCI reads are CLKI/7. 
  //Assuming CLKI = 12.288MgHz for Shield and 16.0MgHz for Arduino
  //The VS1053's internal clock multiplier SCI_CLOCKF:SC_MULT is 1.0x after power up.
  //For a maximum SPI rate of 1.8MgHz = (CLKI/7) = (12.288/7) the VS1053's default.
  
  //Warning: 
  //Note that spi transfers interleave between SdCard and VS10xx.
  //Where Sd2Card.cpp sets SPCR & SPSR each and every transfer

  //The SDfatlib using SPI_FULL_SPEED results in an 8MHz spi clock rate, 
  //faster than initial allowed spi rate of 1.8MgHz. 
  
  // set initial mp3's spi to safe rate
  spiRate = SPI_CLOCK_DIV16; // initial contact with VS10xx at slow rate

  //Initialize VS1053 chip
  delay(10);
  digitalWrite(MP3_RESET, HIGH); //Bring up VS1053

   //Let's check the status of the VS1053
  int MP3Mode = Mp3ReadRegister(SCI_MODE);
  //int MP3Clock = Mp3ReadRegister(SCI_CLOCKF);
/*
  Serial.print(F("SCI_Mode (0x4800) = 0x"));
  Serial.println(MP3Mode, HEX);

  Serial.print(F("SCI_Status (0x48) = 0x"));
  Serial.println(MP3Status, HEX);

  Serial.print(F("SCI_ClockF = 0x"));
  Serial.println(MP3Clock, HEX);
  */

  if(MP3Mode != (SM_LINE1 | SM_SDINEW)) return 4;

  //Now that we have the VS1053 up and running, increase the internal clock multiplier and up our SPI rate
  Mp3WriteRegister(SCI_CLOCKF, 0x6000); //Set multiplier to 3.0x
  //Internal clock multiplier is now 3x.
  //Therefore, max SPI speed is 52MgHz.
  spiRate = SPI_CLOCK_DIV4; //use safe SPI rate of (16MHz / 4 = 4MHz)
  delay(10); // settle time

  //test reading after data rate change
  int MP3Clock = Mp3ReadRegister(SCI_CLOCKF);
  if(MP3Clock != 0x6000) return 5;

  if (VSLoadUserCode("patches.053")) Serial.println(F("Warning: patch file not found, skipping."));

  SetVolume(40, 40);

#if defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_Timer1
  Timer1.initialize(MP3_REFILL_PERIOD);
#elif defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer
  timerId_mp3 = timer.setInterval(MP3_REFILL_PERIOD, refill);
  timer.disable(timerId_mp3);
#endif

  return 0;
}

//Store and Push member volume to VS10xx chip
void SFEMP3Shield::SetVolume(unsigned char leftchannel, unsigned char rightchannel){

  VolL = leftchannel;
  VolR = rightchannel;
  Mp3WriteRegister(SCI_VOL, leftchannel, rightchannel);
}


//call on a mp3 just with a number
uint8_t SFEMP3Shield::playTrack(uint8_t trackNo){

  //a storage place for track names
  char trackName[] = "track001.mp3";
  uint8_t trackNumber = 1;

  //tack the number onto the rest of the filename
  sprintf(trackName, "track%03d.mp3", trackNo);

  //play the file
  return playMP3(trackName);
}


uint8_t SFEMP3Shield::playMP3(char* fileName){

  if (playing) return 1;

  //Open the file in read mode.
  if (!track.open(&root, fileName, O_READ)) return 2;

  playing = TRUE;

  //look for first MP3 frame (11 1's)
  bitrate = 0;
  uint8_t temp = 0;
  uint8_t row_num =0;

  // find length of arrary at pointer
  int fileNamefileName_length = 0;
  while(*(fileName + fileNamefileName_length))
    fileNamefileName_length++;

//  if ((fileName[fileNamefileName_length-2] & 0x7F) == 'p') { // case insensitive check for P of .MP3 filename extension.
//    for(uint16_t i = 0; i<65535; i++){
//    //for(;;){
//      if(track.read() == 0xFF) {
//
//        temp = track.read();
//
//        if(((temp & 0b11100000) == 0b11100000) && ((temp & 0b00000110) != 0b00000000)) {
//
//          //found the 11 1's
//          //parse version, layer and bitrate out and save bitrate
//          if(!(temp & 0b00001000)) //!true if Version 1, !false version 2 and 2.5
//            row_num = 3;
//            if((temp & 0b00000110) == 0b00000100) //true if layer 2, false if layer 1 or 3
//            row_num += 1;
//          else if((temp & 0b00000110) == 0b00000010) //true if layer 3, false if layer 2 or 1
//            row_num += 2;
//
//          //parse bitrate code from next byte
//          temp = track.read();
//          temp = temp>>4;
//
//          //lookup bitrate
//          bitrate = pgm_read_word_near ( &(bitrate_table[temp][row_num]) );
//
//          //convert kbps to Bytes per mS
//          bitrate /= 8;
//
//          //record file position
//          track.seekCur(-3);
//          start_of_music = track.curPosition();
//
//          //Serial.print(F("POS: "));
//          //Serial.println(start_of_music);
//
//          //Serial.print(F("Bitrate: "));
//          //Serial.println(bitrate);
//
//          //break out of for loop
//          break;
//
//        }
//      }
//    }
//  }


  //gotta start feeding that hungry mp3 chip
  refill();

  //attach refill interrupt off DREQ line, pin 2
  enableRefill();

  return 0;
}

void SFEMP3Shield::enableRefill() {
#if defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_Timer1
  Timer1.attachInterrupt( refill );
#elif defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer
  timer.enable(timerId_mp3);
#elif !defined(USE_MP3_REFILL_MEANS) || USE_MP3_REFILL_MEANS == USE_MP3_INTx
  attachInterrupt(MP3_DREQINT, refill, RISING);
#endif
}

void SFEMP3Shield::disableRefill() {
#if defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_Timer1
  Timer1.detachInterrupt();
#elif defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer
  timer.disable(timerId_mp3);
#elif !defined(USE_MP3_REFILL_MEANS) || USE_MP3_REFILL_MEANS == USE_MP3_INTx
  detachInterrupt(MP3_DREQINT);
#endif
}

//close track gracefully, cancel intterupt
void SFEMP3Shield::stopTrack(){

  if(playing == FALSE)
    return;

  //cancel external interrupt
  disableRefill();
  playing=FALSE;

  track.close(); //Close out this track

  flush_cancel(pre); //possible mode of "none" for faster response.

  //Serial.println(F("Track is done!"));

}

//is there a song playing?
uint8_t SFEMP3Shield::isPlaying(){

  if(playing == FALSE)
    return 0;
  else
    return 1;
}

void SFEMP3Shield::trackArtist(char* infobuffer){
  getTrackInfo(TRACK_ARTIST, infobuffer);
}

void SFEMP3Shield::trackTitle(char* infobuffer){
  getTrackInfo(TRACK_TITLE, infobuffer);
}

void SFEMP3Shield::trackAlbum(char* infobuffer){
  getTrackInfo(TRACK_ALBUM, infobuffer);
}

//reads and returns the track tag information
void SFEMP3Shield::getTrackInfo(uint8_t offset, char* infobuffer){

  //disable interupts
  pauseDataStream();

  //record current file position
  uint32_t currentPos = track.curPosition();

  //skip to end
  track.seekEnd((-128 + offset));

  //read 30 bytes of tag informat at -128 + offset
  track.read(infobuffer, 30);
  infobuffer = strip_nonalpha_inplace(infobuffer);

  //seek back to saved file position
  track.seekSet(currentPos);

  //renable interupt
  resumeDataStream();

}

//reads and returns the track tag information
void SFEMP3Shield::getAudioInfo() {

  //disable interupts
  pauseDataStream();

  uint16_t MP3HDAT1 = Mp3ReadRegister(SCI_HDAT1);
  Serial.print(F("SCI_HDAT1 (?) = 0x"));
  Serial.print(MP3HDAT1, HEX);
  
  uint16_t MP3HDAT0 = Mp3ReadRegister(SCI_HDAT0);
  Serial.print(F(" SCI_HDAT0 (?) = 0x"));
  Serial.print(MP3HDAT0, HEX);
  
  uint16_t MP3Mode = Mp3ReadRegister(SCI_MODE);
  Serial.print(F(" SCI_Mode (0x4800) = 0x"));
  Serial.print(MP3Mode, HEX);
  
  uint16_t MP3Status = Mp3ReadRegister(SCI_STATUS);
  Serial.print(F(" SCI_Status (0x48) = 0x"));
  Serial.print(MP3Status, HEX);
  
  uint16_t MP3Clock = Mp3ReadRegister(SCI_CLOCKF);
  Serial.print(F(" SCI_ClockF = 0x"));
  Serial.print(MP3Clock, HEX);
  
  uint16_t MP3para_version = Mp3ReadWRAM(para_version);
  Serial.print(F(" para_version = 0x"));
  Serial.print(MP3para_version, HEX);
  
  uint16_t MP3ByteRate = Mp3ReadWRAM(para_byteRate);
  Serial.print(F(" para_byteRate = 0x"));
  Serial.print(MP3ByteRate, HEX);
  
  uint16_t MP3SCI_DECODE_TIME = Mp3ReadRegister(SCI_DECODE_TIME);
  Serial.print(F(" SCI_DECODE_TIME(s) = "));
  Serial.print(MP3SCI_DECODE_TIME, DEC);
  
  Serial.println();

  //renable interupt
  resumeDataStream();

}

//cancels interrupt feeding MP3 decoder
void SFEMP3Shield::pauseDataStream(){

  //cancel external interrupt
  if(playing)
    disableRefill();

}

//resumes interrupt feeding MP3 decoder
void SFEMP3Shield::resumeDataStream(){

  //make sure SPI is right speed
  SPI.setDataMode(SPI_MODE0);

  if(playing) {
    //see if it is already ready for more
    refill();

    //attach refill interrupt off DREQ line, pin 2
    enableRefill();
  }

}

//skips to a certain point in th track
bool SFEMP3Shield::skipTo(uint32_t timecode){

  if(playing) {

    //stop interupt for now
    disableRefill();
    playing=FALSE;

    //seek to new position in file
    if(!track.seekSet((timecode * bitrate) + start_of_music))
      return 2;

    Mp3WriteRegister(SCI_VOL, 0xFE, 0xFE);
    //seeked successfully

    flush_cancel(pre); //possible mode of "none" for faster response.

    //gotta start feeding that hungry mp3 chip
    refill();

    //again, I'm being bad and not following the spec sheet.
    //I already turned the volume down, so when the MP3 chip gets upset at me
    //for just slammin in new bits of the file, you won't hear it.
    //so we'll wait a bit, and restore the volume to previous level
    delay(50);

    //one of these days I'll come back and try to do it the right way.
    SetVolume(VolL,VolR);

    //attach refill interrupt off DREQ line, pin 2
    enableRefill();
    playing=TRUE;

    return 0;
  }

  return 1;
}

//returns current timecode in ms. Not very accurate/detministic
uint32_t SFEMP3Shield::currentPosition(){

  return((track.curPosition() - start_of_music) / bitrate );
}

//force bit rate, useful if auto-detect failed
void SFEMP3Shield::setBitRate(uint16_t bitr){

  bitrate = bitr;
  return;
}

void SFEMP3Shield::cs_low() {
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(spiRate); //Set SPI bus speed to 1MHz (16MHz / 16 = 1MHz)
  digitalWrite(MP3_XCS, LOW);
}

void SFEMP3Shield::cs_high() {
  digitalWrite(MP3_XCS, HIGH);
}

void SFEMP3Shield::dcs_low() {
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(spiRate); //Set SPI bus speed to 1MHz (16MHz / 16 = 1MHz)
  digitalWrite(MP3_XDCS, LOW);
}

void SFEMP3Shield::dcs_high() {
  digitalWrite(MP3_XDCS, HIGH);
}

void SFEMP3Shield::Mp3WriteRegister(uint8_t addressbyte, uint16_t data) {
  union twobyte val;
  val.word = data;
  Mp3WriteRegister(addressbyte, val.byte[1], val.byte[0]);
}

void SFEMP3Shield::Mp3WriteRegister(uint8_t addressbyte, uint8_t highbyte, uint8_t lowbyte) {

  //cancel interrupt if playing
  if(playing)
    disableRefill();

  //Wait for DREQ to go high indicating IC is available
  while(!digitalRead(MP3_DREQ)) ;
  //Select control
  cs_low();

  //SCI consists of instruction byte, address byte, and 16-bit data word.
  SPI.transfer(0x02); //Write instruction
  SPI.transfer(addressbyte);
  SPI.transfer(highbyte);
  SPI.transfer(lowbyte);
  while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating command is complete
  cs_high(); //Deselect Control

  //resume interrupt if playing.
  if(playing) {
    //see if it is already ready for more
    refill();

    //attach refill interrupt off DREQ line, pin 2
    enableRefill();
  }

}

//Read the 16-bit value of a VS10xx register
unsigned int SFEMP3Shield::Mp3ReadRegister (unsigned char addressbyte){

  //cancel interrupt if playing
  if(playing)
    disableRefill();

  while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating IC is available
  cs_low(); //Select control

  //SCI consists of instruction byte, address byte, and 16-bit data word.
  SPI.transfer(0x03);  //Read instruction
  SPI.transfer(addressbyte);

  char response1 = SPI.transfer(0xFF); //Read the first byte
  while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating command is complete
  char response2 = SPI.transfer(0xFF); //Read the second byte
  while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating command is complete

  cs_high(); //Deselect Control

  unsigned int resultvalue = response1 << 8;
  resultvalue |= response2;
  return resultvalue;

  //resume interrupt if playing.
  if(playing) {
    //see if it is already ready for more
    refill();

    //attach refill interrupt off DREQ line, pin 2
    enableRefill();
  }
}

//Read the 16-bit value of a VS10xx WRAM location
uint16_t SFEMP3Shield::Mp3ReadWRAM (uint16_t addressbyte){

  unsigned short int tmp1,tmp2;

  Mp3WriteRegister(SCI_WRAMADDR, addressbyte);
  tmp1 = Mp3ReadRegister(SCI_WRAM);

  Mp3WriteRegister(SCI_WRAMADDR, addressbyte);
  tmp2 = Mp3ReadRegister(SCI_WRAM);

  if (tmp1==tmp2) return tmp1;
  Mp3WriteRegister(SCI_WRAMADDR, addressbyte);
  tmp2 = Mp3ReadRegister(SCI_WRAM);

  if (tmp1==tmp2) return tmp1;
  Mp3WriteRegister(SCI_WRAMADDR, addressbyte);
  tmp2 = Mp3ReadRegister(SCI_WRAM);

  if (tmp1==tmp2) return tmp1;
  return tmp1;
}

// public interface of refill.
//This is will allow future helpers of
void SFEMP3Shield::available() {
#if defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer
  timer.run();
#elif defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_Polled
  refill();
#endif
}

//refill VS10xx buffer with new data
void SFEMP3Shield::refill() {


  //Serial.println(F("filling"));

  while(digitalRead(MP3_DREQ)){

    if(!track.read(mp3DataBuffer, sizeof(mp3DataBuffer))) { //Go out to SD card and try reading 32 new bytes of the song
      track.close(); //Close out this track
      playing=FALSE;

      //cancel external interrupt
      disableRefill();

      flush_cancel(post); //possible mode of "none" for faster response.

      //Oh no! There is no data left to read!
      //Time to exit
      break;
    }


    //Once DREQ is released (high) we now feed 32 bytes of data to the VS1053 from our SD read buffer
    dcs_low(); //Select Data
    for(int y = 0 ; y < sizeof(mp3DataBuffer) ; y++) {
      //while(!digitalRead(MP3_DREQ)); // wait until DREQ is or goes high // turns out it is not needed.
      SPI.transfer(mp3DataBuffer[y]); // Send SPI byte
    }

    dcs_high(); //Deselect Data
    //We've just dumped 32 bytes into VS1053 so our SD read buffer is empty. go get more data
  }
}

//flush the buffer and cancel
// post - will flush vx10xx's 2K buffer after cancelled, typically with stopping a file, to have immediate affect.
// pre  - will flush buffer prior to issuing cancel, typically to allow completion of file
// both - will flush before and after issuing cancel
// none - will just issue cancel. Not sure if this should be used. Such as in skipto.
// note if cancel fails the vs10xx will be reset and initialized to current values.
void SFEMP3Shield::flush_cancel(flush_m mode) {
  int8_t endFillByte = (int8_t) (Mp3ReadWRAM(para_endFillByte) & 0xFF);

  if ((mode == post) || (mode == both)) {
    dcs_low(); //Select Data
    for(int y = 0 ; y < 2052 ; y++) {
      while(!digitalRead(MP3_DREQ)); // wait until DREQ is or goes high
      SPI.transfer(endFillByte); // Send SPI byte
    }
    dcs_high(); //Deselect Data
  }

  for (int n = 0; n < 64 ; n++)
  {
    Mp3WriteRegister(SCI_MODE, SM_LINE1 | SM_SDINEW | SM_CANCEL);
    dcs_low(); //Select Data
    for(int y = 0 ; y < 32 ; y++) {
      while(!digitalRead(MP3_DREQ)); // wait until DREQ is or goes high
      SPI.transfer(endFillByte); // Send SPI byte
    }
    dcs_high(); //Deselect Data

    int cancel = Mp3ReadRegister(SCI_MODE) & SM_CANCEL;
    if (cancel == 0) {
      // Cancel has succeeded.
      if ((mode == pre) || (mode == both)) {
        dcs_low(); //Select Data
        for(int y = 0 ; y < 2052 ; y++) {
          while(!digitalRead(MP3_DREQ)); // wait until DREQ is or goes high
          SPI.transfer(endFillByte); // Send SPI byte
        }
        dcs_high(); //Deselect Data
      }
      return;
    }
  }
  // Cancel has not succeeded.
  //Serial.println(F("Warning: VS10XX chip did not cancel, reseting chip!"));
  Mp3WriteRegister(SCI_MODE, SM_LINE1 | SM_SDINEW | SM_RESET);
  //begin(); // however, SFEMP3Shield::begin() is member function that does not exist statically.
}

// load VS1xxx with patch or plugin from file on SDcard.
uint8_t SFEMP3Shield::VSLoadUserCode(char* fileName){

  union twobyte val;
  union twobyte addr;
  union twobyte n;

  if (playing) return 1;

  //Open the file in read mode.
  if (!track.open(&root, fileName, O_READ)) return 2;
  //playing = TRUE;
  //while (i<size_of_Plugin/sizeof(Plugin[0])) {
  while (1) {
    //addr = Plugin[i++];
    if (!track.read(addr.byte, 2)) break;
    //n = Plugin[i++];
    if (!track.read(n.byte, 2)) break;
    if (n.word & 0x8000U) { /* RLE run, replicate n samples */
      n.word &= 0x7FFF;
      //val = Plugin[i++];
      if (!track.read(val.byte, 2)) break;
      while (n.word--) {
        Mp3WriteRegister(addr.word, val.word);
      }
    } else {           /* Copy run, copy n samples */
      while (n.word--) {
        //val = Plugin[i++];
        if (!track.read(val.byte, 2))   break;
        Mp3WriteRegister(addr.word, val.word);
      }
    }
  }
  track.close(); //Close out this track
  //playing=FALSE;
  return 0;
}

// chomp non printable characters out of string.
char* strip_nonalpha_inplace(char *s) {
  for ( ; *s && !isalpha(*s); ++s)
    ; // skip leading non-alpha chars
  if (*s == '\0')
    return s; // there are no alpha characters

  char *tail = s + strlen(s);
  for ( ; !isalpha(*tail); --tail)
    ; // skip trailing non-alpha chars
  *++tail = '\0'; // truncate after the last alpha

  return s;
}
