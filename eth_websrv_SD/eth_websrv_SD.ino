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
#include <Ethernet.h>
#include <SD.h>

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x2E, 0xAC };

// the router's gateway address:
IPAddress gateway(192, 168, 1, 1);

// the subnet:
IPAddress subnet(255, 255, 255, 0);

IPAddress ip(192, 168, 1, 20); // IP address, may need to change depending on network

EthernetServer server(80);  // create a server at port 80



String getFinder;
String variable;
String value;
File webFile;
File logFile;
int motorControl = 3;
char c;
EthernetClient client;
//
//void log(){
//      logFile = SD.open("log.txt", FILE_WRITE);
//      logFile.print(c);
//      logFile.close(); 
//  
//}

unsigned long int currentRandomNumberSeed = 0;
unsigned long int getRandomInt(){
	currentRandomNumberSeed = (unsigned long int)(75*currentRandomNumberSeed)%65521;
	return currentRandomNumberSeed;
}

void setup()
{
    pinMode(motorControl,OUTPUT);
    
    //reserve space for strings
    value.reserve(25);
    getFinder.reserve(3);
    variable.reserve(10);
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
    Serial.begin(9600);       // for debugging
    
    // initialize SD card
    Serial.println("Initializing SD card...");
    if (!SD.begin(4)) {
        Serial.println("ERROR - SD card initialization failed!");
        return;    // init failed
    }
    Serial.println("SUCCESS - SD card initialized.");
    // check for index.htm file
    if (!SD.exists("index.htm")) {
        Serial.println("ERROR -s Can't find index.htm file!");
        return;  // can't find index file
    }
    Serial.println("SUCCESS - Found index.htm file.");
}

File userFile;
boolean turnOn = false;
String seed = "";
String password = "";

void processVariable(){
      logFile = SD.open("log.txt", FILE_WRITE);
      logFile.print("PROCESSING VARIABLE!");
      logFile.println(variable);
      logFile.close(); 
 
  String usernamePath = "users/";
  //TODO switch to enum
   switch(variable.charAt(0)){
      case 'u':  logFile.println("Request Username: " + value);
                 usernamePath += value;
                 char usernameCharArray [usernamePath.length()];
                 usernamePath.toCharArray(usernameCharArray, usernamePath.length());
                 if(SD.exists(usernameCharArray)){
                  userFile = SD.open(usernameCharArray);
                  while((c = userFile.read()) != ' '){
                    seed += c; 
                  };
                  logFile.println("seed read: " + seed);
                  currentRandomNumberSeed = seed.toInt();
                  
                  //REQUIRED: passwords must end with \n
                  while((c = userFile.read()) != '\n'){
                    password += (char)((int)c + getRandomInt()%256); 
                  };                  
                }
                 break;
      case 'c': //command
                 logFile.println("Request Command: " + value); 
                 if(value.compareTo("open") == 0){
                   turnOn = true;
                   //digitalWrite(motorControl, HIGH);
                 }else{//assume value == close
                   turnOn = false;                   
                 }
                 break;
      case 'p': //password
                 logFile.println("Access Requested: Recieved - " + value + "\t Actual - " + password); 
                 if(value.compareTo(password)){
                   if(turnOn){
                     digitalWrite(motorControl, HIGH);
                   }else{
                     digitalWrite(motorControl, LOW);
                   }
                   logFile.remove();
                   logFile.open(usernamePath, FILE_WRITE);
                   logFile.println(seed + 1);
                   logFile.println(password);
                 }
                 break;
      default:   break; 
   }
}

void readRequestLine(){
  //check for params, return if empty
  while((c = client.read()) != '?'){
    if(c == ' ')
      return; 
  };
  boolean onVariable = true;
  value = "";
  variable = "";
  while((c = client.read()) != ' '){
    if(c != '&'){
      if(onVariable){
        if(c == '='){
          onVariable = false;
        }else{
          variable += c;
        }
      }else{
          value += c;
      }
    }else{ 
      //process current set of variables
      processVariable();
      
      variable = "";
      value = "";
      onVariable = true;
    }
  }
  processVariable();
}

void loop()
{
    client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
          getFinder = "";
            if (client.available()) {   // client data available to read
                c = client.read();
                
                //look for GET
                if(c == 'G' && (c = client.read()) == 'E' && (c = client.read()) == 'T'){
                  client.read();     // read space
                  readRequestLine(); //read request params
                }
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");
                    client.println();
                    
                    logFile = SD.open("log.txt");
                    if (logFile) {
                        while(logFile.available()) {
                            client.write(logFile.read()); // send web page to client
                        }
                        logFile.close();
                    }
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(2);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}
