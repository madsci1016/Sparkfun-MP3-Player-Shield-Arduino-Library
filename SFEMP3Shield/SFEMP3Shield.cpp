#include "SFEMP3Shield.h"
// inslude the SPI library:
#include "SPI.h"



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
  pinMode(10, OUTPUT);       
  //Initialize the SD card and configure the I/O pins.
  if (!card.init(SPI_FULL_SPEED)) return 1; // Serial.println("Error: Card init"); 
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
  
  return 0;
}


void SFEMP3Shield::SetVolume(unsigned char leftchannel, unsigned char rightchannel){
  Mp3WriteRegister(SCI_VOL, leftchannel, rightchannel);
}


//call on a mp3 just with a number
uint8_t SFEMP3Shield::playTrack(uint8_t trackNo){
	
	//a storage place for track names
	char trackName[] = "track001.mp3";
	int trackNumber = 1;
	
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
	  
	//gotta start feeding that hungry mp3 chip
	refill();
	  
	//attach refill interrupt off DREQ line, pin 2
	attachInterrupt(0, refill, RISING);
	  
	return 0;
}

//close track gracefully, cancel intterupt
void SFEMP3Shield::stopTrack(){
  
	if(playing == FALSE)
		return;
  
	//cancel external interrupt
	detachInterrupt(0);

	//tell MP3 chip to do a soft reset. Fixes garbles at end, and clears its buffer. 
	//easier then the way your SUPPOSE to do it by the manual, same result as much as I can tell.
	Mp3WriteRegister(SCI_MODE, 0x48, SM_RESET);
	  
	track.close(); //Close out this track
	playing=FALSE;
	  
	//Serial.println("Track is done!");
  
}

//is there a song playing?
uint8_t SFEMP3Shield::isPlaying(){
  
	if(playing == FALSE)
		return 0;
	else
		return 1;
}

//cancels interrupt feeding MP3 decoder
void SFEMP3Shield::pauseDataStream(){

	//cancel external interrupt
	if(playing)
		detachInterrupt(0);

}

//resumes interrupt feeding MP3 decoder
void SFEMP3Shield::resumeDataStream(){

	if(playing)	{
		//see if it is already ready for more
		refill();

		//attach refill interrupt off DREQ line, pin 2
		attachInterrupt(0, refill, RISING);
	}

}

static void Mp3WriteRegister(unsigned char addressbyte, unsigned char highbyte, unsigned char lowbyte){
	
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
}

//Read the 16-bit value of a VS10xx register
static unsigned int Mp3ReadRegister (unsigned char addressbyte){
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

  int resultvalue = response1 << 8;
  resultvalue |= response2;
  return resultvalue;
}

//refill VS10xx buffer with new data
static void refill() {
  
  
  //Serial.println("filling");
  
  while(digitalRead(MP3_DREQ)){

        if(!track.read(mp3DataBuffer, sizeof(mp3DataBuffer))) { //Go out to SD card and try reading 32 new bytes of the song
			track.close(); //Close out this track
			playing=FALSE;
			
			//cancel external interrupt
			detachInterrupt(0);
			
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







































