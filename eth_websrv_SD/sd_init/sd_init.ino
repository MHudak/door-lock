/*--------------------------------------------------------------
  Program:      eth_websrv_SD

  Description:  Arduino web server that serves up a basic web
                page. The web page is stored on the SD card.
  
  Hardware:     Arduino Uno and official Arduino Ethernet
                shield. Should work with other Arduinos and
                compatible Ethernet shields.
                2Gb micro SD card formatted FAT16
                
  Software:     Developed using Arduino 1.0.3 software
                Should be compatible with Arduino 1.0 +
                SD card contains web page called index.htm
  
  References:   - WebServer example by David A. Mellis and 
                  modified by Tom Igoe
                - SD card examples by David A. Mellis and
                  Tom Igoe
                - Ethernet library documentation:
                  http://arduino.cc/en/Reference/Ethernet
                - SD Card library documentation:
                  http://arduino.cc/en/Reference/SD

  Date:         10 January 2013
 
  Author:       W.A. Smith, http://startingelectronics.com
--------------------------------------------------------------*/

#include <SPI.h>
#include <SD.h>

File file;
void setup()
{
    Serial.begin(9600);       // for debugging
    
    // initialize SD card
    if (!SD.begin(4)) {
        return;    // init failed
    }
    SD.remove("log.txt");
    file = SD.open("log.txt", FILE_WRITE);
    if(file){
      Serial.println("file opened");
      file.println(0);
      file.print("password");
      file.close();
      Serial.println("output success");
    }
//    char c;
//    file = SD.open("log.txt", FILE_READ);
//    while(file.available()){
//      c = file.read();
//      Serial.print(c);
//    }
//    file.close();
    
    
}

void loop()
{
}
