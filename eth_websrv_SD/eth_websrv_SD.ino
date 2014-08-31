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

#define PASSWORD_LENGTH 8
#define VARIABLE_LENGTH 10
#define VALUE_LENGTH 10

#define MOTOR_FORWARD 3
#define MOTOR_BACKWARD 5
#define RED_BUTTON 6
#define LOCKED 0  
#define OPENED 1

// Codes for the motor function.
#define FORWARD 1
#define BACKWARD -1

//codes for response output
#define SUCCESS 1
#define RELOAD 2
#define FAILURE -1
#define OK 3

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x2E, 0xAC };

// the router's gateway address:
IPAddress gateway(192, 168, 0, 1);

// the subnet:
IPAddress subnet(255, 255, 255, 0);

IPAddress ip(192, 168, 0, 52); // IP address, may need to change depending on network

EthernetServer server(80);  // create a server at port 80

String variable;
String value;
File webFile;
char c;
EthernetClient client;
File userFile;
int turnOn = 0;
String seed = "";
String password = "";
String saltedPW = "";
char incPassword [PASSWORD_LENGTH + 1];
boolean seedRead = false;
boolean success = false;
int lock_State = 0;

unsigned long currentRandomNumberSeed = 0L;
unsigned long getRandomInt(){
  currentRandomNumberSeed = currentRandomNumberSeed & 0xFFF;
  currentRandomNumberSeed = (unsigned long)(214013L * currentRandomNumberSeed + 2531011L)%16777216L;
  return currentRandomNumberSeed;
}

void motor(int dir, int time){
 switch(dir){
   case 1: //forward 
     digitalWrite(MOTOR_BACKWARD, LOW);
     digitalWrite(MOTOR_FORWARD, HIGH);
     break;
   case -1: //backward
     digitalWrite(MOTOR_FORWARD, LOW);
     digitalWrite(MOTOR_BACKWARD, HIGH);
     break;
 }
 delay(time);
 digitalWrite(MOTOR_BACKWARD, LOW);
 digitalWrite(MOTOR_FORWARD, LOW);
}

void setup()
{
    //reserve space for strings
    value.reserve(VALUE_LENGTH);
    variable.reserve(VARIABLE_LENGTH);
    saltedPW.reserve(PASSWORD_LENGTH+1);
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
    Serial.begin(9600);       // for debugging
    
    // initialize SD card
    if (!SD.begin(4)) {
//      Serial.println("SD Init Failed");
        return;    // init failed
    }
    
    //setup motor
    pinMode(MOTOR_FORWARD, OUTPUT);
    pinMode(MOTOR_BACKWARD, OUTPUT);
    digitalWrite(MOTOR_FORWARD, LOW);
    digitalWrite(MOTOR_BACKWARD, LOW);
    
//    Serial.println("setup");
}


void processVariable(){
  String usernamePath;
  char usernameCharArray [15];
  usernamePath.reserve(15);
  usernamePath = "users/";
//  usernamePath = "";
   switch(variable.charAt(0)){
      case 'u':  
                 usernamePath += value;
                 usernamePath += ".txt ";
                 usernamePath.toCharArray(usernameCharArray, usernamePath.length());
//                 Serial.print("path: ");
//                 Serial.println(usernameCharArray);
                 if(!SD.exists(usernameCharArray)){
                   break;          
                 }
//                 Serial.println("file exists");
                 userFile = SD.open(usernameCharArray, FILE_READ);
                 seed = "";
                 password="";
                 saltedPW = "";
                 while((c = userFile.read()) != '\n'){
                   seed += c; 
                 };
                 currentRandomNumberSeed = seed.toInt();
//                 Serial.print("cSeed: ");
//                 Serial.println(currentRandomNumberSeed);
                 seedRead = true;
                 while(userFile.available()){
                   c=userFile.read();
                   password += c;
                   c = (((unsigned long)c + getRandomInt()) % 256L);
                   saltedPW += c; 
                 };
                 while(saltedPW.length() < PASSWORD_LENGTH){
                   c = (getRandomInt() % 256L);
                   saltedPW += c; 
                 }
                 userFile.close();  
                 break;

      case 'c': //command
                 if(value.compareTo("open") == 0){
                   turnOn = 1;
                 }else if(value.compareTo("close") == 0){//assume value == close
                   turnOn = -1;     
                 }else{ //verifying user
                   turnOn = 0;
                 }
                 break;
                 seedRead = false;
                 break;
      case 'p': //password
      Serial.print("incPW: ");
      Serial.println(incPassword);
                 Serial.print("actPW: ");
                 Serial.println(saltedPW); 
                 Serial.print("compare: ");
                 Serial.println(saltedPW.compareTo(incPassword));
                 if(seedRead && saltedPW.compareTo(incPassword) == 0){
                   success = true;
                   switch(turnOn){
                      case 1://open
//                               Serial.print("open");
                               motor(BACKWARD, 500);
                               break;
                      case -1://close
//                               Serial.print("close");           
                               motor(FORWARD, 500);
                               break;  
                      default: //authenticate
                               break;
                   }
                   SD.remove(usernameCharArray);
                   userFile = SD.open(usernameCharArray, FILE_WRITE);
//                   Serial.print("RNS: ");
//                   Serial.println(currentRandomNumberSeed);
                   userFile.println(currentRandomNumberSeed);
                   userFile.print(password);
                   userFile.close();
                 }
                 break;
   }
   return;
}

int pathPointer = 0;
void readRequestLine(){
  pathPointer = 0;
  //check for params, return if empty
  while((c = client.read()) != '?'){
    pathPointer++;
    if(c == ' '){
      if(pathPointer < 3){
        success = RELOAD;
      }
//      Serial.println("No params!");
      return; 
    }
  };
  boolean onVariable = true;
  int encryptionPointer = 0;
  int i;
  for(i = 0; i < PASSWORD_LENGTH; i++){
     incPassword[i] = 0;
  }
  value = "";
  variable = "";
  int overflowPrevent = 0;
  while((c = client.read()) != ' ' && c != '#'){
    if(c != '&'){
      if(onVariable){
        if(c == '='){
          onVariable = false;
          overflowPrevent = 0;
        }else{
          overflowPrevent++;
          if(overflowPrevent <= VARIABLE_LENGTH){
            variable += c;
          }else{
            return;
          }
        }
      }else{
          if(variable.compareTo("pw") == 0 && encryptionPointer < (3 * PASSWORD_LENGTH)){
             switch(encryptionPointer%3){
               case 0: incPassword[encryptionPointer/3] += (unsigned char) (c - '0') * 100;
                       break;
               case 1: incPassword[encryptionPointer/3] += (unsigned char) (c - '0') * 10;
                       break;
               case 2: incPassword[encryptionPointer/3] += (unsigned char) (c - '0');
                       break;
             }
             encryptionPointer++;
          }else{
            if(overflowPrevent <= VALUE_LENGTH){
              value += c;
            }else{
              break;
            }
          }  
      }
    }else{
      //process current set of variables
      processVariable();
      
      variable = "";
      value = "";
      onVariable = true;
    }
  }
  //process final set of variables
  processVariable();
}

void output(char outputFileCharArray[]){
//  Serial.println("output");
  File outputFile = SD.open(outputFileCharArray);
  if (outputFile) {
//    Serial.println("outputting");
      char c1;
      char previousChar = ' ';
      while(outputFile.available()) {
        c1 = outputFile.read();
        if(c1 == '{' && previousChar == '{'){
          Serial.println("directive");
          variable = "";
          while(!(c1 == '}' && previousChar == '}')){
            c1 = outputFile.read();
            if(c1 != '}'){
              variable += c1;
            }
          }
          if(variable.compareTo("STATE")){
            Serial.println("STATE");
            client.print(lock_State);
          }
          String tempFileName;
          tempFileName.reserve(21);
          tempFileName = "values/" + variable + ".txt";
          char tempCharArray[19];
          tempFileName.toCharArray(tempCharArray, 19);
          output(tempCharArray);
        }
        client.write(c1); // send web page to client
        previousChar = c1;
      }  
      outputFile.close();
  }
}

void checkButton()
{
  if (digitalRead(RED_BUTTON) == HIGH && lock_State == LOCKED){
            motor(BACKWARD, 500);
      lock_State = OPENED;
  }
  else if (digitalRead(RED_BUTTON) == HIGH && lock_State == OPENED){
                motor(FORWARD, 500);
    lock_State = LOCKED;
  }
}

void loop()
{
    client = server.available();  // try to get client
    success = FAILURE;
    
    if (client) {  // got client?
        turnOn = 0;
        password = "";
        boolean currentLineIsBlank = true;
        boolean firstRequestLine = true;
        
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                c = client.read();
//                Serial.print(c);
                //look for GET
                if(firstRequestLine && c == 'G' && (c = client.read()) == 'E' && (c = client.read()) == 'T'){
                  firstRequestLine = false;
                  client.read();     // read space
                  readRequestLine(); //read request params
                }
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {                    
                     switch(success){
                       case SUCCESS://send standard response
                                 output("success.txt");
                                 client.print(lock_State);
                                 break;
                       case RELOAD: //send full response
                                 output("success.txt");
                                 output("index.txt");
                                 break;
                       default:  //failure, send failure response
                                 output("failure.txt");
                    }
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    if(!firstRequestLine){
                      firstRequestLine = true;
                    }
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(5);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)

  // open or lock the door from the inside
  checkButton();
}
