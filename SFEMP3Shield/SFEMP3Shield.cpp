#include "SFEMP3Shield.h"
// inslude the SPI library:
#include "SPI.h"
//avr pgmspace library for storing the LUT in program flash instead of sram
#include <avr/pgmspace.h>

//bitrate lookup table      V1,L1  V1,L2   V1,L3   V2,L1  V2,L2+L3
//168 bytes(!!); better to store in progmem or eeprom
PROGMEM prog_uint16_t bitrate_table[14][6] = { {0,0,0,0,0,0},
					       {32,32,32,32,8,8}, //0001
					       {64,48,40,48,16,16}, //0010
					       {96,56,48,56,24,24}, //0011
					       {128,64,56,64,32,32}, //0100
					       {160,80,64,80,40,40}, //0101
					       {192,96,80,96,48,48}, //0110
					       {224,112,96,112,56,56}, //0111
					       {256,128,112,128,64,64}, //1000
					       {288,160,128,144,80,80}, //1001
					       {320,192,160,160,96,69}, //1010
					       {352,224,192,176,112,112}, //1011
					       {384,256,224,192,128,128}, //1100
					       {416,320,256,224,144,144} };//1101

//Inits everything
uint8_t SFEMP3Shield::begin(){

  pinMode(MP3_DREQ, INPUT);
  pinMode(MP3_XCS, OUTPUT);
  pinMode(MP3_XDCS, OUTPUT);
  pinMode(MP3_RESET, OUTPUT);
  
  digitalWrite(MP3_XCS, HIGH); //Deselect Control
  digitalWrite(MP3_XDCS, HIGH); //Deselect Data
  digitalWrite(MP3_RESET, LOW); //Put VS1053 into hardware reset
  
  //Setup SD card interface
  //Pin 10 must be set as an output for the SD communication to work.
  //pinMode(10, OUTPUT);
  //pinMode(53, OUTPUT);      
  //Initialize the SD card and configure the I/O pins.
  if (!card.init(SPI_FULL_SPEED, SD_SEL)) return 1; // Serial.println("Error: Card init"); 
  //Initialize a volume on the SD card.
  if (!volume.init(&card)) return 2; //Serial.println("Error: Volume ini"); 
  //Open the root directory in the volume.
  if (!root.openRoot(&volume)) return 3; //Serial.println("Error: Opening root"); //Open the root directory in the volume. 

  //We have no need to setup SPI for VS1053 because this has already been done by the SDfatlib
  
  //From page 12 of datasheet, max SCI reads are CLKI/7. Input clock is 12.288MHz. 
  //Internal clock multiplier is 1.0x after power up. 
  //Therefore, max SPI speed is 1.75MHz. We will use 1MHz to be safe.
  SPI.setClockDivider(SPI_CLOCK_DIV16); //Set SPI bus speed to 1MHz (16MHz / 16 = 1MHz)
  SPI.transfer(0xFF); //Throw a dummy byte at the bus
  //Initialize VS1053 chip 
  delay(10);
  digitalWrite(MP3_RESET, HIGH); //Bring up VS1053
  
  SFEMP3Shield::SetVolume(40, 40);
  VolL = 40;
  VolR = 40;
  
   //Let's check the status of the VS1053
  int MP3Mode = Mp3ReadRegister(SCI_MODE);
  //int MP3Clock = Mp3ReadRegister(SCI_CLOCKF);
/*
  Serial.print("SCI_Mode (0x4800) = 0x");
  Serial.println(MP3Mode, HEX);

  Serial.print("SCI_Status (0x48) = 0x");
  Serial.println(MP3Status, HEX);

  Serial.print("SCI_ClockF = 0x");
  Serial.println(MP3Clock, HEX);
  */
  
  if(MP3Mode != 0x4800) return 4;
  
  
  //Now that we have the VS1053 up and running, increase the internal clock multiplier and up our SPI rate
  Mp3WriteRegister(SCI_CLOCKF, 0x60, 0x00); //Set multiplier to 3.0x
  
  //From page 12 of datasheet, max SCI reads are CLKI/7. Input clock is 12.288MHz. 
  //Internal clock multiplier is now 3x.
  //Therefore, max SPI speed is 5MHz. 4MHz will be safe.
  SPI.setClockDivider(SPI_CLOCK_DIV4); //Set SPI bus speed to 4MHz (16MHz / 4 = 4MHz)
  
  //test reading after data rate change
  int MP3Clock = Mp3ReadRegister(SCI_CLOCKF);
  if(MP3Clock != 0x6000) return 5;
  
  if (VSLoadUserCode("patches.053")) Serial.println("Warning: patch file not found, skipping.");
  
  return 0;
}


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
    return (SFEMP3Shield::playMP3(trackName));
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
	
	if ((fileName[fileNamefileName_length-2] & 0x7F) == 'p') { // case insensitive check for P of .MP3 filename extension.
		for(uint16_t i = 0; i<65535; i++){
		//for(;;){
			if(track.read() == 0xFF) {
				
				temp = track.read();
				
				if(((temp & 0b11100000) == 0b11100000) && ((temp & 0b00000110) != 0b00000000)) {
	
					//found the 11 1's
					//parse version, layer and bitrate out and save bitrate
					if(!(temp & 0b00001000)) //!true if Version 1, !false version 2 and 2.5
						row_num = 3;
				    if((temp & 0b00000110) == 0b00000100) //true if layer 2, false if layer 1 or 3
						row_num += 1;
					else if((temp & 0b00000110) == 0b00000010) //true if layer 3, false if layer 2 or 1	
						row_num += 2;
					
					//parse bitrate code from next byte
					temp = track.read();
					temp = temp>>4;
					
					//lookup bitrate
					bitrate = pgm_read_word_near ( temp*5 + row_num );
					//							      bitrate_table[temp][row_num];
					
					//convert kbps to Bytes per mS
					bitrate /= 8;
					
					//record file position
					track.seekCur(-3);
					start_of_music = track.curPosition();
					
					//Serial.print("POS: ");
					//Serial.println(start_of_music);
					
					//Serial.print("Bitrate: ");
					//Serial.println(bitrate);
					
					//break out of for loop
					break;
				
				}	    
			}
		}
	}
	
	  
	//gotta start feeding that hungry mp3 chip
	refill();
	  
	//attach refill interrupt off DREQ line, pin 2
	attachInterrupt(MP3_DREQINT, refill, RISING);
	  
	return 0;
}

//close track gracefully, cancel intterupt
void SFEMP3Shield::stopTrack(){
  
	if(playing == FALSE)
		return;
  
	//cancel external interrupt
	detachInterrupt(MP3_DREQINT);
	playing=FALSE;

	//tell MP3 chip to do a soft reset. Fixes garbles at end, and clears its buffer. 
	//easier then the way your SUPPOSE to do it by the manual, same result as much as I can tell.
	Mp3WriteRegister(SCI_MODE, 0x48, SM_RESET);
	  
	track.close(); //Close out this track
	
	  
	//Serial.println("Track is done!");
  
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

//cancels interrupt feeding MP3 decoder
void SFEMP3Shield::pauseDataStream(){

	//cancel external interrupt
	if(playing)
		detachInterrupt(MP3_DREQINT);

}

//resumes interrupt feeding MP3 decoder
void SFEMP3Shield::resumeDataStream(){

	//make sure SPI is right speed
	SPI.setDataMode(SPI_MODE0);

	if(playing)	{
		//see if it is already ready for more
		refill();

		//attach refill interrupt off DREQ line, pin 2
		attachInterrupt(MP3_DREQINT, refill, RISING);
	}

}

//skips to a certain point in th track
bool SFEMP3Shield::skipTo(uint32_t timecode){

	if(playing) {
	
		//stop interupt for now
		detachInterrupt(MP3_DREQINT);
		playing=FALSE;
		
		//seek to new position in file
		if(!track.seekSet((timecode * bitrate) + start_of_music))
			return 2;
			
		Mp3WriteRegister(SCI_VOL, 0xFE, 0xFE);
		//seeked successfully
		
		//tell MP3 chip to do a soft reset. Fixes garbles at end, and clears its buffer. 
	    //easier then the way your SUPPOSE to do it by the manual, same result as much as I can tell.
	    Mp3WriteRegister(SCI_MODE, 0x48, SM_RESET);
		
		//gotta start feeding that hungry mp3 chip
		refill();
		
		//again, I'm being bad and not following the spec sheet.
		//I already turned the volume down, so when the MP3 chip gets upset at me
		//for just slammin in new bits of the file, you won't hear it. 
		//so we'll wait a bit, and restore the volume to previous level
		delay(50);
		
		//one of these days I'll come back and try to do it the right way.
		SFEMP3Shield::SetVolume(VolL,VolR);
		  
		//attach refill interrupt off DREQ line, pin 2
		attachInterrupt(MP3_DREQINT, refill, RISING);
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

void Mp3WriteRegister(uint8_t addressbyte, uint16_t data) {
	union twobyte val;
	val.word = data;
	Mp3WriteRegister(addressbyte, val.byte[1], val.byte[0]);
}

void Mp3WriteRegister(uint8_t addressbyte, uint8_t highbyte, uint8_t lowbyte) {

	//cancel interrupt if playing
	if(playing)
		detachInterrupt(MP3_DREQINT);
	
	//Wait for DREQ to go high indicating IC is available
	while(!digitalRead(MP3_DREQ)) ; 
	//Select control
	digitalWrite(MP3_XCS, LOW); 

	//SCI consists of instruction byte, address byte, and 16-bit data word.
	SPI.transfer(0x02); //Write instruction
	SPI.transfer(addressbyte);
	SPI.transfer(highbyte);
	SPI.transfer(lowbyte);
	while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating command is complete
	digitalWrite(MP3_XCS, HIGH); //Deselect Control
	
	//resume interrupt if playing. 
	if(playing)	{
		//see if it is already ready for more
		refill();

		//attach refill interrupt off DREQ line, pin 2
		attachInterrupt(MP3_DREQINT, refill, RISING);
	}
	
}

//Read the 16-bit value of a VS10xx register
unsigned int Mp3ReadRegister (unsigned char addressbyte){
  
	//cancel interrupt if playing
	if(playing)
		detachInterrupt(MP3_DREQINT);
	  
	while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating IC is available
	digitalWrite(MP3_XCS, LOW); //Select control

	//SCI consists of instruction byte, address byte, and 16-bit data word.
	SPI.transfer(0x03);  //Read instruction
	SPI.transfer(addressbyte);

	char response1 = SPI.transfer(0xFF); //Read the first byte
	while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating command is complete
	char response2 = SPI.transfer(0xFF); //Read the second byte
	while(!digitalRead(MP3_DREQ)) ; //Wait for DREQ to go high indicating command is complete

	digitalWrite(MP3_XCS, HIGH); //Deselect Control

	unsigned int resultvalue = response1 << 8;
	resultvalue |= response2;
	return resultvalue;
  
	//resume interrupt if playing. 
	if(playing)	{
		//see if it is already ready for more
		refill();

		//attach refill interrupt off DREQ line, pin 2
		attachInterrupt(MP3_DREQINT, refill, RISING);
	}
}

//refill VS10xx buffer with new data
static void refill() {
  
  
  //Serial.println("filling");
  
  while(digitalRead(MP3_DREQ)){

        if(!track.read(mp3DataBuffer, sizeof(mp3DataBuffer))) { //Go out to SD card and try reading 32 new bytes of the song
			track.close(); //Close out this track
			playing=FALSE;
			
			//cancel external interrupt
			detachInterrupt(MP3_DREQINT);
			
			//tell MP3 chip to do a soft reset. Fixes garbles at end, and clears its buffer. 
			//easier then the way your SUPPOSE to do it by the manual, same result as much as I can tell.
			Mp3WriteRegister(SCI_MODE, 0x48, SM_RESET);

           //Oh no! There is no data left to read!
          //Time to exit
          break;
        }
      
   
      //Once DREQ is released (high) we now feed 32 bytes of data to the VS1053 from our SD read buffer
      digitalWrite(MP3_XDCS, LOW); //Select Data
      for(int y = 0 ; y < sizeof(mp3DataBuffer) ; y++) {
        SPI.transfer(mp3DataBuffer[y]); // Send SPI byte
      }
  
      digitalWrite(MP3_XDCS, HIGH); //Deselect Data
       //We've just dumped 32 bytes into VS1053 so our SD read buffer is empty. go get more data
  }
}

// chomp non printable characters out of string.
char* strip_nonalpha_inplace(char *s) {
  for ( ; *s && !isalpha(*s); ++s)
    ; // skip leading non-alpha chars
  if (*s == '\0')
    return s; // there are no alpha characters

//  assert(isalpha(*s));
  char *tail = s + strlen(s);
  for ( ; !isalpha(*tail); --tail)
    ; // skip trailing non-alpha chars
//  assert(isalpha(*tail));
  *++tail = '\0'; // truncate after the last alpha

  return s;
}

// load VS1xxx with patch or plugin from file on SDcard.
uint8_t SFEMP3Shield::VSLoadUserCode(char* fileName){

	union twobyte val;
	union twobyte addr;
	union twobyte n;
	
	if (playing) return 1;

	//Open the file in read mode.
	if (!track.open(&root, fileName, O_READ)) return 2;
//	playing = TRUE;
//  while (i<size_of_Plugin/sizeof(Plugin[0])) {
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
				if (!track.read(val.byte, 2)) 	break;
        Mp3WriteRegister(addr.word, val.word);
      }
    }
  }
	track.close(); //Close out this track
//	playing=FALSE;
	return 0;
}