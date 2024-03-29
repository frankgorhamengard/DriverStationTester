/*This is code to test the Sparky GEN 2 Control Panel 
 * with 7 segment
 * THIS  IS A TEST PROGRAM TO LOAD INTO CONTROL PANEL

This sketch reads controls and 
sends command values to the console
Developed by Miss Daisy FRC Team 341
*/

//  this is for test mode
//#include <AltSoftSerial.h>
//AltSoftSerial Serial;
const int mybaud = 600;
boolean runTimeMonitorEnabled = false;
// D8 - AltSoftSerial RX
// D9 - AltSoftSerial TX

#include "Wire.h"
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_7segment matrix = Adafruit_7segment();

//#include <EasyTransfer.h>
//create two objects
//EasyTransfer ETin, ETout; 

// Transfer data strucures must be the same at both ends
//   so, they are defined in a library included by both sketches
#include <SparkyXfrBuffers.h>

// declare global structures for data used by the transfer objects
// reverse naming on opposite ends
FROM_SPARKY_DATA_STRUCTURE rxdata;
TO_SPARKY_DATA_STRUCTURE txdata;

//#define DRIVE_MODE        13 
#define SYSTEM_ENABLE   12
//  #define PANEL_LED_5     11  panel LEDs are controlled by setLED function, 0 to 4
#define ENABLE_LED_3     3    // pin 10  
// pin 9 used by altser
#define SHOOT_BUTTON    8
#define INTAKE_BUTTON   7
  #define INTAKEBUTTON_LED     0
  #define SHOOTBUTTON_LED     1
#define TEST_SWITCH     4
#define R_STICK_BUTTON  3
#define HC05_POWER_ON_LOW_2  2

//#define L_STICK_Y      1
#define R_STICK_X    0    // vertical forward-backward stick axis attached to A0
//#define R_STICK_Y      1
#define L_STICK_X    2    // horizontal left-right-turn stick axis attached to A2
#define SHOOTERSPEED 3    // shooter peed knob attached to A3

unsigned long triggerTime;
unsigned long headingTime;
const int panelLedArr[5] = {5,6,9,10,11}; //map of wired pins to LEDs
long int messageCounter = 0;

/////////////////////////////////////////////////////////////////////////////
// FUNCTION: setLED,   returns nothing
// ARGUMENTS: LEDnum is value 0 to 4, brightness is 0 (off) to 255 (full on)
void setLED(int LEDnum, unsigned int brightness) {
  if ( brightness > 255 ) brightness = 255;
  signed long brightness_l = brightness; 
  if ( 0 <= LEDnum && LEDnum < 2 ) {
    //  index to pins, use panelLedArr   ,  value to write is non-linear
    // these LEDs are high active
    int LEDoutput = max( ((brightness_l+7)/8), (brightness_l*3)-510 );
    analogWrite( panelLedArr[LEDnum], LEDoutput );
  } else if ( LEDnum < 5 ) {
    //  index to pins, use panelLedArr   ,  value to write is non-linear
    // these LEDs are LOW active
    int LEDoutput = min( 255-((brightness_l+7)/8), (255-brightness_l)*3 );
    analogWrite( panelLedArr[LEDnum], LEDoutput );
  }
}
/////////////////////////////////////////////////////////////////////////////////
// called once at start
void setup(){
  //digitalWrite(HC05_POWER_ON_LOW_2, HIGH );  // make sure it starts up OFF
  //pinMode(HC05_POWER_ON_LOW_2  , OUTPUT); // 2 LOW is active

  Serial.begin(9600);
  while (!Serial) ; // wait for serial port to connect. Needed for native USB
  
  //start the library, pass in the data details and the name of the serial port. Can be Serial, Serial1, Serial2, etc.
  //ETin.begin(details(rxdata), &Serial);
  //ETout.begin(details(txdata), &Serial);

  //altser.begin(600);

  // Wire. is a TwoWire type object declared in the include file Wire.h
  Wire.begin(); // join i2c bus (address optional for master)
  // 2wire code is at the end of loop - end of this file

  matrix.begin(0x70);

  matrix.writeDigitNum(0, 8 , true);
  matrix.writeDigitNum(1, 8 , true);
  matrix.drawColon(true);
  matrix.writeDigitNum(3, 8 , true);
  matrix.writeDigitNum(4, 8 , true);
  matrix.writeDisplay();

  digitalWrite(HC05_POWER_ON_LOW_2, LOW );  // now turn the HC05 on 

  //  init LEDs     //////////////////////////
  for (int i=0; i<5; i++) {
    pinMode( panelLedArr[i], OUTPUT);
    setLED( i, 255); // on
    delay(1500);
    setLED( i, 0);
  }

  matrix.writeDigitRaw(0, 0 );
  matrix.writeDigitRaw(1, 0 );
  matrix.drawColon(false);
  matrix.writeDigitRaw(3, 0 );
  matrix.writeDigitRaw(4, 0 );
  matrix.writeDisplay();

 
  // init inputs and enable pullup
  pinMode(13      , INPUT_PULLUP); // 13 unused
  pinMode(SYSTEM_ENABLE   , INPUT_PULLUP); // 12 LOW is ENABLEd
  pinMode(SHOOT_BUTTON    , INPUT_PULLUP); // 8 LOW is SHOOT
  pinMode(INTAKE_BUTTON   , INPUT_PULLUP); // 7 LOW is INATKE
  pinMode(TEST_SWITCH     , INPUT_PULLUP); // 4 LOW is on
  pinMode(R_STICK_BUTTON  , INPUT_PULLUP); // 3 LOW is active
  // // other pins setup before this
  
//  triggerTime = millis() + 3000;  // 3 seconds from now
//  delay(2000);
//  altser.println();
//  altser.print("Sparky Control Panel proto3 tester is loaded   :Created ");
//  altser.print( __DATE__ );
//  altser.print(" ");
//  altser.println( __TIME__ );
//  if ( digitalRead(TEST_SWITCH) == HIGH ) { // LOW is on
//    runTimeMonitorEnabled = false;
//    altser.println("Runtime Monitor is disabled");
//  }

  Serial.print("Sparky GEN 2 Control Panel tester   :Created ");
  Serial.print( __DATE__ );
  Serial.print(" ");
  Serial.println( __TIME__ );
  if ( digitalRead(TEST_SWITCH) == HIGH ) { // LOW is on
    Serial.println("TEST_SWITCH is OFF");
  } else {
    Serial.println("TEST_SWITCH is ON");
  }
  
}
/////////////////////  MAIN LOOP  /////////////////////////////
void loop(){
  static unsigned long wireTimer0,wireTimer1;
  
  // read our potentiometers and buttons and store raw data in data structure for transmission
  txdata.stickLy = analogRead(R_STICK_X); 
  txdata.stickLx = txdata.stickLy; 
  txdata.stickLbutton = LOW;   // no STICK BUTTONs attached
  txdata.stickRy = 1023-analogRead(L_STICK_X); 
  txdata.stickRx = txdata.stickRy;  
  txdata.stickRbutton = LOW;

  txdata.drivemode = 1;
  txdata.enabled = !digitalRead(SYSTEM_ENABLE);
  
  txdata.shooterspeed = analogRead(SHOOTERSPEED);

  int buttonValue = 240;
  if( !digitalRead(INTAKE_BUTTON) ){
    txdata.intake = 1;
    buttonValue -= 80;
  }
  else {
    txdata.intake = 0;
  }
  if( !digitalRead(SHOOT_BUTTON) ){
    txdata.shoot = HIGH;
    buttonValue -= 80;
  }
  else {
    txdata.shoot = LOW;
  }
  // Check if the controller is enabled
  if(txdata.enabled){
    buttonValue -= 80;
    messageCounter += 1;
    txdata.counter = messageCounter;
  }
  //then we will go ahead and send that data out
  //ETout.sendData();

  
 //there's a loop here so that we run the recieve function more often then the 
 //transmit function. This is important due to the slight differences in 
 //the clock speed of different Arduinos. If we didn't do this, messages 
 //would build up in the buffer and appear to cause a delay.
 // for(int i=0; i<5; i++){
    //remember, you could use an if() here to check for new data, this time it's not needed.
    //ETin.receiveData();
    //delay(10);   // short delay between receive attempts
  ///}
  
  //delay for good measure
  delay(10);

//  *********  RUNTIME DISPLAY or DISPLAY TESTING *****************
  unsigned long testnow = millis();
  // once per second 
  if ( testnow >= triggerTime ) {
    triggerTime = testnow + 1000;
    
    if ( headingTime > testnow ) {
      Serial.print( "   " );
      Serial.print( txdata.enabled );
      Serial.print( ",   " );
      Serial.print( txdata.intake );
      Serial.print( ",   " );
      Serial.print( txdata.shoot );
      Serial.print( "   : " );
      Serial.print( txdata.shooterspeed );
      Serial.print( " :  " );
      Serial.print( txdata.stickLy );
      Serial.print( "   " );
      Serial.println( txdata.stickRx );
     
    } else {
      Serial.println( "ENAB,INTA,SHOO:SPEE:  UP RIGHT" );
      headingTime = testnow + 20000;
    }
  }
  static unsigned long updateDue;
  unsigned long now;
  static unsigned int ENval = 0;
  static unsigned int INval = 0;
  static unsigned int SHval = 0;
  static int phase;
  unsigned int phasefade;
  now = millis();
  if ( now > updateDue ) {
    updateDue = now + 100; // 10 updates per second, max

    if ( txdata.enabled ) {
      if (ENval < 251) ENval += 5;
    } else {
      if (ENval > 4 ) ENval -= 5;
    } 
    if ( txdata.intake ) {
      if (INval < 251) INval += 5;
    } else {
      if (INval > 4 ) INval -= 5;
    } 
    if ( txdata.shoot ) {
      if (SHval < 251) SHval += 5;
    } else {
      if (SHval > 4 ) SHval -= 5;
    } 

    setLED( ENABLE_LED_3, ENval );
    setLED( INTAKEBUTTON_LED, INval );
    setLED( SHOOTBUTTON_LED, SHval );
  }

  ///////////////////////  TWI code for 7segment display interface ///////////
  if ( wireTimer1 < (millis() - 500) ) {
    int calctemp;
    static int speeddisplay;
    
    wireTimer1 = millis(); // reset
    wireTimer0 = micros();

    calctemp = ((rxdata.supplyvoltagereading*15)+17) / 142; //tenths of a volt resolution
    matrix.writeDigitNum(0, (calctemp/10) % 10 , true);
    matrix.writeDigitNum(1, calctemp % 10 , false);
    matrix.drawColon(true);
    calctemp = (txdata.shooterspeed * 15) / 152;
    if ( abs(calctemp-speeddisplay)>1 ) {
      speeddisplay = calctemp;
    }
    matrix.writeDigitNum(3, (speeddisplay / 10) , false);
    matrix.writeDigitNum(4, speeddisplay % 10, false);
 
    matrix.writeDisplay();

    wireTimer0 = micros() - wireTimer0;     // how long did this take?
  }
}
