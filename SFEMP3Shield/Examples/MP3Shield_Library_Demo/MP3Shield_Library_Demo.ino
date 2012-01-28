/**************************************
*
*  Example for Sparkfun MP3 Shield Library
*      By: Bill Porter
*      www.billporter.info
*
*   Function:
*      This sketch listens for commands from a serial terminal (like the Serial Monitor in 
*      the Arduino IDE). If it sees 1-9 it will try to play an MP3 file named track00x.mp3
*      where x is a number from 1 to 9. For eaxmple, pressing 2 will play 'track002.mp3'.
*      A lowe case 's' will stop playing the mp3.
*      'f' will play an MP3 by calling it by it's filename as opposed to a track number. 
*
*      Sketch assumes you have MP3 files with filenames like 
*      "track001.mp3", "track002.mp3", etc on an SD card loaded into the shield. 
*
***************************************/

#include <SPI.h>

//Add the SdFat Libraries
#include <SdFat.h>
#include <SdFatUtil.h> 

//and the MP3 Shield Library
#include <SFEMP3Shield.h>

//create and name the library object
SFEMP3Shield MP3player;

byte temp;
byte result;

void setup() {

  Serial.begin(115200);
  
  //boot up the MP3 Player Shield
  result = MP3player.begin();
  //check result, see readme for error codes.
  if(result != 0) {
    Serial.print("Error code: ");
    Serial.print(result);
    Serial.println(" when trying to start MP3 player");
    }

  Serial.println("Hello");
  Serial.println("Send a number 1-9 to play a track or s to stop playing");
}

void loop() {
  
  if(Serial.available()){
    temp = Serial.read();
    
    Serial.print("Received command: ");
    Serial.write(temp);
    Serial.println(" ");
    
    //if s, stop the current track
    if (temp == 's')
      MP3player.stopTrack();
      
    else if (temp >= '1' && temp <= '9'){
      //convert ascii numbers to real numbers
      temp = temp - 48;
      
      //tell the MP3 Shield to play a track
      result = MP3player.playTrack(temp);
      
      //check result, see readme for error codes.
      if(result != 0) {
        Serial.print("Error code: ");
        Serial.print(result);
        Serial.println(" when trying to play track");
        }
      }
    
    /* Alterativly, you could call a track by it's file name by using playMP3(filename); 
       But you must stick to 8.1 filenames, only 8 characters long, and 3 for the extension */
    
    else if (temp == 'f') {
      //create a string with the filename
      char trackName[] = "track001.mp3";
      
      //tell the MP3 Shield to play that file
      result = MP3player.playMP3(trackName);
      
      //check result, see readme for error codes.
      if(result != 0) {
        Serial.print("Error code: ");
        Serial.print(result);
        Serial.println(" when trying to play track");
        }
      }
      
  }
  
  delay(100);
  
}
