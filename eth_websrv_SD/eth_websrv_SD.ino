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

// Arduino pins for the shift register
#define MOTORLATCH 12
#define MOTORCLK 4
#define MOTORENABLE 7
#define MOTORDATA 8

// 8-bit bus after the 74HC595 shift register 
// (not Arduino pins)
// These are used to set the direction of the bridge driver.
#define MOTOR1_A 2
#define MOTOR1_B 3

// Arduino pins for the PWM signals.
#define MOTOR1_PWM 11

// Codes for the motor function.
#define FORWARD 1
#define BACKWARD 2
#define BRAKE 3
#define RELEASE 4

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x2E, 0xAC };

// the router's gateway address:
IPAddress gateway(192, 168, 1, 1);

// the subnet:
IPAddress subnet(255, 255, 255, 0);

IPAddress ip(192, 168, 1, 20); // IP address, may need to change depending on network

EthernetServer server(80);  // create a server at port 80

String variable;
String value;
File webFile;
int motorControl = 3;
char c;
EthernetClient client;

unsigned long int currentRandomNumberSeed = 0;
unsigned long int getRandomInt(){
	currentRandomNumberSeed = (unsigned long int)(75*currentRandomNumberSeed)%26843549;
	return currentRandomNumberSeed;
}

void setup()
{
    pinMode(motorControl,OUTPUT);
    
    //reserve space for strings
    value.reserve(10);
    variable.reserve(10);
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
    Serial.begin(9600);       // for debugging
    
    // initialize SD card
    if (!SD.begin(4)) {
        return;    // init failed
    }
}

//motor code/////////////////////////////
void motor(int command, int speed)
{
  int motorA = MOTOR1_A;
  int motorB = MOTOR1_B;

  switch (command)
  {
  case FORWARD:
    motor_output (motorA, HIGH, speed);
    motor_output (motorB, LOW, -1);     // -1: no PWM set
    break;
  case BACKWARD:
    motor_output (motorA, HIGH, speed);
    motor_output (motorB, HIGH, -1);    // -1: no PWM set
    break;
  case BRAKE:
    motor_output (motorA, LOW, 255); // 255: fully on.
    motor_output (motorB, LOW, -1);  // -1: no PWM set
    break;
  case RELEASE:
    motor_output (motorA, LOW, 0);  // 0: output floating.
    motor_output (motorB, LOW, -1); // -1: no PWM set
    break;
  default:
    break;
  }
}

void motor_output (int output, int high_low, int speed)
{
  int motorPWM;

  switch (output)
  {
  case MOTOR1_A:
  case MOTOR1_B:
    motorPWM = MOTOR1_PWM;
    break;
  default:
    speed = -3333;
    break;
  }

  if (speed != -3333)
  {
    shiftWrite(output, high_low);
    if (speed >= 0 && speed <= 255)    
    {
      analogWrite(motorPWM, speed);
    }
  }
}

void shiftWrite(int output, int high_low)
{
  static int latch_copy;
  static int shift_register_initialized = false;
  if (!shift_register_initialized)
  {
    // Set pins for shift register to output
    pinMode(MOTORLATCH, OUTPUT);
    pinMode(MOTORENABLE, OUTPUT);
    pinMode(MOTORDATA, OUTPUT);
    pinMode(MOTORCLK, OUTPUT);
    digitalWrite(MOTORDATA, LOW);
    digitalWrite(MOTORLATCH, LOW);
    digitalWrite(MOTORCLK, LOW);
    digitalWrite(MOTORENABLE, LOW);
    latch_copy = 0;
    shift_register_initialized = true;
  }

  bitWrite(latch_copy, output, high_low);
  shiftOut(MOTORDATA, MOTORCLK, MSBFIRST, latch_copy);
  delayMicroseconds(5);    // For safety, not really needed.
  digitalWrite(MOTORLATCH, HIGH);
  delayMicroseconds(5);    // For safety, not really needed.
  digitalWrite(MOTORLATCH, LOW);
}
/////////////////////////////////////////////////////////////////////////////////////////////

File userFile;
boolean turnOn = false;
String seed = "";
String password = "";
unsigned long int saltedPW = 0;
boolean seedRead = false;
void processVariable(){
  String usernamePath;
  char usernameCharArray [15];
  usernamePath.reserve(15);
  //TODO get directory to work
//  usernamePath = "/users/";
  usernamePath = "";
  //TODO switch to enum
  //TODO change to be order independent
   switch(variable.charAt(0)){
      case 'u':  usernamePath += value;
                 usernamePath += ".txt ";
                 usernamePath.toCharArray(usernameCharArray, usernamePath.length());
//                 Serial.println(usernameCharArray);
                 if(SD.exists(usernameCharArray)){
//                   Serial.println("ufile exists");
                  userFile = SD.open(usernameCharArray, FILE_READ);
                  seed = "";
                  saltedPW = 0;
                  while((c = userFile.read()) != '\n'){
                    seed += c; 
                  };
                  currentRandomNumberSeed = seed.toInt();
                  
                  seedRead = true;
                  while(userFile.available()){
                    c=userFile.read();
                    password += c;
                    saltedPW += (unsigned long int)((unsigned int)c + getRandomInt()); 
                  };
                  userFile.close();
//                  Serial.print("incPW: ");
//                  Serial.println(password);               
                }
                else{
//                  Serial.println("no such username");
                }
                 break;
      case 'c': //command
//                 Serial.print("req cmd: ");
//                Serial.println(value);
                 if(value.compareTo("open") == 0){
                   turnOn = true;
                   //digitalWrite(motorControl, HIGH);
                 }else{//assume value == close
                   turnOn = false;                   
                 }
                 break;
                 seedRead = false;
                 break;
      case 'p': //password
                 Serial.print("incPW: ");
                 Serial.print(value);
                 Serial.print("\t actPW: ");
                 Serial.println(saltedPW); 
                 if(value.toInt() == saltedPW){
                   if(turnOn){
                     Serial.println("on");
                      motor(FORWARD, 255);
                      delay(25);
                      motor(RELEASE, 0);
                   }else{
                     Serial.println("off");
                      motor(BACKWARD, 255);
                      delay(25);
                      motor(RELEASE, 0);
                   }
                   SD.remove(usernameCharArray);
                   userFile = SD.open(usernameCharArray, FILE_WRITE);
                   Serial.println(currentRandomNumberSeed);
                   userFile.println(currentRandomNumberSeed);
                   userFile.println(password);
                   userFile.close();
                 }else{
                  //TODO invalid password 
                 }
                 break;

   }
   return;
}

void readRequestLine(){
  //check for params, return if empty
  while((c = client.read()) != '?'){
    if(c == ' '){
      Serial.println("No params!");
      return; 
    }
  };
  //TODO handle incomplete params (http://.../index.html?param=)
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
  //process final set of variables
  processVariable();
}

void loop()
{
    client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        boolean firstRequestLine = true;
        
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                c = client.read();
                Serial.print(c);
                //look for GET
                if(firstRequestLine && c == 'G' && (c = client.read()) == 'E' && Serial.print(c) && (c = client.read()) == 'T' && Serial.print(c)){
                  firstRequestLine = false;
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
                    
/*                    logFile = SD.open("log.txt");
                    if (logFile) {
                        while(logFile.available()) {
                            client.write(logFile.read()); // send web page to client
                        }
                        logFile.close();
                    }
*/                    break;
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
