////----SimpleRXAckPayload - The Receiver----////
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN   7
#define CSN_PIN 8

const byte thisSlaveAddress[5] = {'R', 'x', 'A', 'A', 'A'};

RF24 radio(CE_PIN, CSN_PIN);

int dataReceived[8]; // this must match dataToSend in the TX
int ackData[5] = {10, 20, 20, 20, 20}; // the four values to be sent to the master
bool newData = false;

//----LCD----//
#include <Wire.h> //Wire Library 
#include <LiquidCrystal_I2C.h> //liquid crystal with I2C 

//Instantiating LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

//Switch For Two Modes
const int modePin = 4;
int modeState = 0;
bool modeStatus = false;

//Defining Pin For Button For LCD
const int switchPin = 2;
static int hits = 0;
//Variable To Hold The Value Of Switch State
int switchState = 0;
int prevSwitchState = 0;

//BlueTooth
char incomingValue = '0';
int btle;

//Switch For BlueTooth Button
const int bluePin = 3;
int blueState = 0;
bool blueStatus = false;


////----JOYSTICK FOR MOVEMENT----////
int x1 = 0;
int y1 = 0;

////----JOYSTICK FOR PANTILT----////
int x2 = 0;
int y2 = 0;


////----VOID SETUP----////
void setup() {

  //NRf Module
  Serial.begin(9600);
  Serial.println("SimpleRxAckPayload Starting");
  radio.begin();
  radio.setDataRate( RF24_250KBPS );
  radio.openReadingPipe(1, thisSlaveAddress);
  radio.enableAckPayload();
  radio.startListening();
  radio.writeAckPayload(1, &ackData, sizeof(ackData)); // pre-load data

  //Joystick
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);


  //LCD
  lcd.init();
  pinMode(switchPin, INPUT_PULLUP);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Press The Button");
  lcd.setCursor(0, 1);
  lcd.print("To See Different Data");

}


void loop() {

  //--nRF Communication--//
  getData();
  showData();


  //--LCD AND BUTTON--//
  switchState = digitalRead(switchPin);
  if (switchState == prevSwitchState) {
    hits = hits + 1;
    delay(300);
  }

  if (hits == 0)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Distance");
    lcd.setCursor(0, 1);
    lcd.print(dataReceived[0]);
    lcd.setCursor(3, 1);
    lcd.print("cm");

    delay(200);
  }

  else if (hits == 1)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Warm Body: ");
    lcd.setCursor(0, 1);
    if (dataReceived[1] == 1) {
      lcd.print("Positive");
    }
    else {
      lcd.print("Negative");
    }
    delay(200);
  }
  else if ( hits == 2)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Rain Status: ");
    lcd.setCursor(0, 1);
    if (dataReceived[2] == 0) {
      lcd.print("Negative");
    }
    else {
      lcd.print("Postive");
    }
    delay(200);
  }

  else if ( hits == 3)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temperature");
    lcd.setCursor(0, 1);
    lcd.print(dataReceived[3]);
    lcd.setCursor(3, 1);
    lcd.print("C");
    delay(200);
  }
  else if ( hits == 4)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Rel. Humidity");
    lcd.setCursor(0, 1);
    lcd.print(dataReceived[4]);
    lcd.setCursor(3, 1);
    lcd.print("%");
    delay(200);
  }

  else if ( hits == 5)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Motion X: ");
    lcd.setCursor(10, 0);
    lcd.print(x1);
    lcd.setCursor(0, 1);
    lcd.print("Motion Y: ");
    lcd.setCursor(10, 1);
    lcd.print(y1);
    delay(100);
  }

  else if ( hits == 6)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Camera X: ");
    lcd.setCursor(10, 0);
    lcd.print(x2);
    lcd.setCursor(0, 1);
    lcd.print("Camera Y: ");
    lcd.setCursor(10, 1);
    lcd.print(y2);
    delay(100);
  }

  else if ( hits == 7)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Motion Status: ");
    lcd.setCursor(0, 1);
    if (dataReceived[0] < 20) {
      lcd.print("Obstacle Ahead!!!");
    }
    else {
      lcd.clear();
      lcd.print("Motion Status: ");
      lcd.setCursor(0, 1);
      lcd.print("Go ahead :)");
    }
    lcd.print(dataReceived[3]);
    delay(200);
  }

  else if ( hits == 7)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mode Status: ");
    lcd.setCursor(0, 1);
    if (modeStatus == true) {
      if (dataReceived[5] == 1) {
        lcd.print("Focus Mode");
      }
      else  {

        lcd.setCursor(0, 1);
        lcd.print("Auto-Sweep Mode");
      }
    }
    else {
      lcd.print("Normal Mode");
    }
    delay(200);
  }




  else if (hits > 8) {
    hits = 0; //Counter Reset
  }


  //--Joystick--//
  x1 = analogRead(A0);
  y1 = analogRead(A1);
  x2 = analogRead(A2);
  y2 = analogRead(A3);
  //Motion Values
  Serial.print("X Value: ");   Serial.println(x1);
  Serial.print("Y Value: ");   Serial.println(y1);


  //--Modes--//
  modeState = digitalRead(modePin);
  if (modeState == HIGH) {
    modeStatus = true;
  }
  else {
    modeStatus = false;
  }

  //--Bluetooth--//
  blueState = digitalRead(bluePin);
  if (blueState == HIGH) {
    blueStatus = true;
  }

  else {
    blueStatus = false;
  }

  if (blueStatus == true ) {
    if (Serial.available() > 0) {
      incomingValue = Serial.read();
      Serial.print(incomingValue);

      if (incomingValue == '1') {
        btle = 1;
      }
      else if (incomingValue == '2') {
        btle = 2;
      }
      else if (incomingValue == '3') {
        btle = 3;
      }
      else if (incomingValue == '4') {
        btle = 4;
      }
      else if (incomingValue == '0') {
        btle = 0;
      }
    }
  }

}


//--nRF GET DATA FUNCTION--//
void getData() {
  if ( radio.available() ) {
    radio.read( &dataReceived, sizeof(dataReceived) );
    updateReplyData();
    newData = true;
  }
}

//--nRF SHOW DATA FUNCTION--//

void showData() {
  if (newData == true) {
    Serial.print("Data received");
    Serial.print(" ackPayload sent ");
    Serial.print(ackData[0]);
    Serial.print(", ");
    Serial.print(ackData[1]);
    Serial.print(", ");
    Serial.print(ackData[2]);
    Serial.print(", ");
    Serial.print(ackData[3]);
    Serial.println();
    Serial.print("Mode: ");
    Serial.println(modeStatus);
    Serial.print("Blue: ");
    Serial.println(blueStatus);


    newData = false;
  }
}

//--nRF UPDATE PAYLOAD FUNCTION--//
void updateReplyData() {

  if (modeStatus == false) {
    ackData[4] = 0;
    //Pantilt JoyStick Data
    if (dataReceived[1] == 1) {
      int x3 = x2;
      int y3 = y2;
      ackData[2]  = x3;
      ackData[3] = y3;
    }
    else if (dataReceived[1] == 0) {
      ackData[2] = x2;
      ackData[3] = y2;
    }

    //Movement Joystick (Bluetooth OFF)
    if (blueStatus == false) {
      ackData[0] = x1;
      ackData[1] = y1;
    }

    //Movement Joystick (Bluetooth ON)
    else {
      //Forward
      if (btle == 1) {
        ackData[0] = 498;
        ackData[1] = 0;
      }
      //Backward
      if (btle == 2) {
        ackData[0] = 498;
        ackData[1] = 1023;
      }
      //Left
      if (btle == 3) {
        ackData[0] = 0;
        ackData[1] = 498;
      }
      //Right
      if (btle == 4) {
        ackData[0] = 1023;
        ackData[1] = 498;
      }
      //Stop
      if (btle == 0) {
        ackData[0] = 498;
        ackData[1] = 498;
      }
    }
  }
  else if (modeStatus == true) {
    ackData[2] = 498;
    ackData[3] = 498;

    ackData[0] = 498;
    ackData[1] = 498;
    ackData[4] = 1;
  }
  radio.writeAckPayload(1, &ackData, sizeof(ackData)); // load the payload for the next time
}
