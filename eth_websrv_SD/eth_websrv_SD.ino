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
#define MOTOR2_A 1
#define MOTOR2_B 4
#define MOTOR3_A 5
#define MOTOR3_B 7
#define MOTOR4_A 0
#define MOTOR4_B 6

// Arduino pins for the PWM signals.
#define MOTOR1_PWM 11
#define MOTOR2_PWM 3
#define MOTOR3_PWM 6
#define MOTOR4_PWM 5
#define SERVO1_PWM 10
#define SERVO2_PWM 9

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
char c;
EthernetClient client;

/*unsigned long int currentRandomNumberSeed = 0;
unsigned long int getRandomInt(){
	currentRandomNumberSeed = (unsigned long int)(75*currentRandomNumberSeed)%26843549;
	return currentRandomNumberSeed;
}
*/

void setup()
{
    //reserve space for strings
    value.reserve(10);
    variable.reserve(10);
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
    Serial.begin(9600);       // for debugging
    
    // initialize SD card
/*    if (!SD.begin(4)) {
        return;    // init failed
    }
*/
}

File userFile;
boolean turnOn = false;
String seed = "";
String password = "";
unsigned long int saltedPW = 0;
boolean seedRead = false;
/*void processVariable(){
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
                  password="";
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
                   Serial.print("open");
                   turnOn = true;
                     motor(1, BACKWARD, 255);
                     delay(200);
                     motor(1, RELEASE, 0);
                 }else{//assume value == close
                   turnOn = false;
                     Serial.print("close");           
                     motor(1, FORWARD, 255);
                     delay(200);
                     motor(1, RELEASE, 0);        
                 }
                 break;
                 seedRead = false;
                 break;
      case 'p': //password
//                 Serial.print("\t actPW: ");
//                 Serial.println(saltedPW); 
//                 if(value.toInt() == saltedPW){
                   if(turnOn){
//                     Serial.println("on");
                     motor(1, BACKWARD, 255);
                     delay(200);
                     motor(1, RELEASE, 0);
                   }else{   
//                     Serial.println("off");
                     motor(1, FORWARD, 255);
                     delay(200);
                     motor(1, RELEASE, 0);
                   }
                   SD.remove(usernameCharArray);
                   userFile = SD.open(usernameCharArray, FILE_WRITE);
//                   Serial.println(currentRandomNumberSeed);
                   userFile.println(currentRandomNumberSeed);
                   userFile.println(password);
                   userFile.close();
//                 }else{
                  //TODO invalid password 
//                 }
                 break;
   }
   return;
}
*/
void readRequestLine(){
  //check for params, return if empty
  while((c = client.read()) != '?'){
    if(c == ' '){
      
  motor(4, BACKWARD, 255);
  delay(400);
  motor(4, RELEASE, 0);
      Serial.println("No params!");
      return; 
    }
  };
  
  Serial.println("params!");
  motor(4, FORWARD, 255);
  delay(400);
  motor(4, RELEASE, 0);
  Serial.println("wtf");
  
  
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
//      processVariable();
      
      variable = "";
      value = "";
      onVariable = true;
    }
  }
  //process final set of variables
//  processVariable();
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
                if(firstRequestLine && c == 'G' && (c = client.read()) == 'E' && (c = client.read()) == 'T'){
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
                    break;
*/
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
        delay(2);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}
// ---------------------------------
// motor
//
// Select the motor (1-4), the command, 
// and the speed (0-255).
// The commands are: FORWARD, BACKWARD, BRAKE, RELEASE.
//
void motor(int nMotor, int command, int speed)
{
  int motorA, motorB;

  if (nMotor >= 1 && nMotor <= 4)
  {  
    switch (nMotor)
    {
    case 1:
      motorA   = MOTOR1_A;
      motorB   = MOTOR1_B;
      break;
    case 2:
      motorA   = MOTOR2_A;
      motorB   = MOTOR2_B;
      break;
    case 3:
      motorA   = MOTOR3_A;
      motorB   = MOTOR3_B;
      break;
    case 4:
      motorA   = MOTOR4_A;
      motorB   = MOTOR4_B;
      break;
    default:
      break;
    }

    switch (command)
    {
    case FORWARD:
      motor_output (motorA, HIGH, speed);
      motor_output (motorB, LOW, -1);     // -1: no PWM set
      break;
    case BACKWARD:
      motor_output (motorA, LOW, speed);
      motor_output (motorB, HIGH, -1);    // -1: no PWM set
      break;
    case BRAKE:
      // The AdaFruit library didn't implement a brake.
      // The L293D motor driver ic doesn't have a good
      // brake anyway.
      // It uses transistors inside, and not mosfets.
      // Some use a software break, by using a short
      // reverse voltage.
      // This brake will try to brake, by enabling 
      // the output and by pulling both outputs to ground.
      // But it isn't a good break.
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
}



// ---------------------------------
// motor_output
//
// The function motor_ouput uses the motor driver to
// drive normal outputs like lights, relays, solenoids, 
// DC motors (but not in reverse).
//
// It is also used as an internal helper function 
// for the motor() function.
//
// The high_low variable should be set 'HIGH' 
// to drive lights, etc.
// It can be set 'LOW', to switch it off, 
// but also a 'speed' of 0 will switch it off.
//
// The 'speed' sets the PWM for 0...255, and is for 
// both pins of the motor output.
//   For example, if motor 3 side 'A' is used to for a
//   dimmed light at 50% (speed is 128), also the 
//   motor 3 side 'B' output will be dimmed for 50%.
// Set to 0 for completelty off (high impedance).
// Set to 255 for fully on.
// Special settings for the PWM speed:
//    Set to -1 for not setting the PWM at all.
//
void motor_output (int output, int high_low, int speed)
{
  int motorPWM;

  switch (output)
  {
  case MOTOR1_A:
  case MOTOR1_B:
    motorPWM = MOTOR1_PWM;
    break;
  case MOTOR2_A:
  case MOTOR2_B:
    motorPWM = MOTOR2_PWM;
    break;
  case MOTOR3_A:
  case MOTOR3_B:
    motorPWM = MOTOR3_PWM;
    break;
  case MOTOR4_A:
  case MOTOR4_B:
    motorPWM = MOTOR4_PWM;
    break;
  default:
    // Use speed as error flag, -3333 = invalid output.
    speed = -3333;
    break;
  }

  if (speed != -3333)
  {
    // Set the direction with the shift register 
    // on the MotorShield, even if the speed = -1.
    // In that case the direction will be set, but
    // not the PWM.
    shiftWrite(output, high_low);

    // set PWM only if it is valid
    if (speed >= 0 && speed <= 255)    
    {
      analogWrite(motorPWM, speed);
    }
  }
}


// ---------------------------------
// shiftWrite
//
// The parameters are just like digitalWrite().
//
// The output is the pin 0...7 (the pin behind 
// the shift register).
// The second parameter is HIGH or LOW.
//
// There is no initialization function.
// Initialization is automatically done at the first
// time it is used.
//
void shiftWrite(int output, int high_low)
{
  static int latch_copy;
  static int shift_register_initialized = false;

  // Do the initialization on the fly, 
  // at the first time it is used.
  if (!shift_register_initialized)
  {
    // Set pins for shift register to output
    pinMode(MOTORLATCH, OUTPUT);
    pinMode(MOTORENABLE, OUTPUT);
    pinMode(MOTORDATA, OUTPUT);
    pinMode(MOTORCLK, OUTPUT);

    // Set pins for shift register to default value (low);
    digitalWrite(MOTORDATA, LOW);
    digitalWrite(MOTORLATCH, LOW);
    digitalWrite(MOTORCLK, LOW);
    // Enable the shift register, set Enable pin Low.
    digitalWrite(MOTORENABLE, LOW);

    // start with all outputs (of the shift register) low
    latch_copy = 0;

    shift_register_initialized = true;
  }

  // The defines HIGH and LOW are 1 and 0.
  // So this is valid.
  bitWrite(latch_copy, output, high_low);

  // Use the default Arduino 'shiftOut()' function to
  // shift the bits with the MOTORCLK as clock pulse.
  // The 74HC595 shiftregister wants the MSB first.
  // After that, generate a latch pulse with MOTORLATCH.
  shiftOut(MOTORDATA, MOTORCLK, MSBFIRST, latch_copy);
  delayMicroseconds(5);    // For safety, not really needed.
  digitalWrite(MOTORLATCH, HIGH);
  delayMicroseconds(5);    // For safety, not really needed.
  digitalWrite(MOTORLATCH, LOW);
}
