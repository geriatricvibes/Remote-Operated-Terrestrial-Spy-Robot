//-----Robot Body Code-----//

//--nRF Module Libaries--//
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//--nRF Module Pins--//
#define CE_PIN   9
#define CSN_PIN 10


//Defining Pipe Address For nRF Module
const byte slaveAddress[5] = {'R', 'x', 'A', 'A', 'A'};

//Instantiating the Radio Communication
RF24 radio(CE_PIN, CSN_PIN);

//Initiating Data Array For nRF Communication and Ack Data
int dataToSend[8] = {1, 2, 3, 4, 5, 6, 7, 8};
int ackData[5] = {10, 20, 20, 20, 20};
bool newData = false;

//Interval Between Communication
unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 10; // send once 0.1 of a second

//To add DHT Sensor Code
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

//DHT11 Sensor
float h, t;
#define DHTPIN 36
#define DHTTYPE    DHT11     // DHT 11 
DHT_Unified dht(DHTPIN, DHTTYPE);

//--Rain Sensor--//
#define sensorPower 35
#define sensorPin 36
int rainstatus = 0;

//--PIR Sensor--//
#define PIRSensor 31 //Signal Pin Of Sensor To Digital Pin 5
int state = LOW;
int val = 0;
int PIR = 0;

//--Ultrasonic Sensor--//
#define echoPin 28// Attach digital pin 8 Arduino to pin Echo of HC-SR04
#define trigPin 29 //Attach digital pin 9 Arduino to pin Trig of HC-SR04

//Defining Variables For Ultrasonic Sensor
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement3

//Movement L293D
#define IN1 2
#define IN2 3
#define IN3 4
#define IN4 5
#define EN1 6   // to control speed of motor 1
#define EN2 7 // to control speed of motor 2
#define LED A0

//Defining variables for movement(l293d + camera)
int x1, y1, x2, y2 = 0;
int speed = 0;

//----Pan Tilt----////
#include <Servo.h>
Servo tilt;  // create servo object to control a servo
Servo pan;
int tiltVal;    // variable to read the value from the analog pin
int panVal;

//----TFT----//
// include TFT and SPI libraries
#include <TFT.h>
#include <SPI.h>

//Pins
#define cs   26
#define dc   25
#define rst  24

//Creating An Instance Of The TFT
TFT TFTscreen = TFT(cs, dc, rst);

////----VOID SETUP----////
void setup() {

  //Initiating The Serial With 9600 Bps rate
  Serial.begin(9600);

  //--nRF Module--//
  Serial.println("SimpleTx Starting");
  Serial.println("SimpleTxAckPayload Starting");
  radio.begin();

  //--Ack Payload Concept--//
  radio.setDataRate( RF24_250KBPS );
  radio.enableAckPayload();
  radio.setRetries(5, 5); // delay, count
  radio.openWritingPipe(slaveAddress);

  //--Ultrasonic Sensor--//
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT);  // Sets the echoPin as an INPUT

  //--PIR Sensor--//9
  pinMode(PIRSensor, INPUT); //PIR Motion Sensor As Input

  //--Rain Sensor--//
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW); //Initially Keep The Sensor Off

  //--DHT--//
  dht.begin();
  sensor_t sensor;

  //Movement(L293D)
  pinMode(IN1 , OUTPUT);       // to L293D
  pinMode(IN2, OUTPUT);        // to L293D
  pinMode(IN3, OUTPUT);        // to L293D
  pinMode(IN4, OUTPUT);        // to L293D
  pinMode(LED, OUTPUT);        //to glow led when distance is less than 20 cm
  pinMode(EN1, OUTPUT);        //pwm pin to control speed
  pinMode(EN2, OUTPUT);        //pwm pin to control speed


  //Pan-Tilt
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  tilt.attach(11);  // attaches the servo
  pan.attach(12);

  //TFT
  TFTscreen.begin();
  TFTscreen.background(0, 0, 0); // clear the screen with a black background
  TFTscreen.setTextSize(1);
  TFTscreen.setRotation(1);

}

////----VOID LOOP----////
void loop() {

  //--nRF Module--//
  currentMillis = millis();
  if (currentMillis - prevMillis >= txIntervalMillis) {
    send(); //Calling The Send Function Which Will Write On the Radio
    prevMillis = millis();
  }

  //Showing Received Data
  showData();

  //--Ultrasonic Sensor--//
  digitalWrite(trigPin, LOW);  // Clears the trigPin condition
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)

  //--PIR Sensor--//
  val = digitalRead(PIRSensor);
  if (val == HIGH) {
    if (state == LOW) {
      PIR = 1;
      state = HIGH;
    }
  }
  else {
    if (state == HIGH) {
      PIR = 0;
      state = LOW;
    }
  }

  //--Rain Sensor--//
  int rainval = readRainSensor(); //Obtaining Value By Calling Function
  if (rainval == 1) {
    rainstatus = 0;
  }
  else {
    rainstatus = 1;
  }

  //DHT11 Sensor
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    t = event.temperature;
    Serial.println(F("°C"));


  }

  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    h = event.relative_humidity;
    Serial.println(F("%"));
  }


  //Motion
  x1 = ackData[0];
  y1 = ackData[1];

  if (dataToSend[0] < 30) {
    digitalWrite(LED, HIGH);

    digitalWrite(EN1, LOW);
    digitalWrite(EN2, LOW);

    //--Movement(L2393D)--//
    if (x1 > 650) {

      //Forward
      digitalWrite(IN1, HIGH);
      digitalWrite(IN4, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);

      //For speed control
      speed = 0;
      speed =  map(x1, 650, 1023, 20, 255);
      analogWrite(EN1, speed);
      analogWrite(EN2, speed);

    }

    else if (x1 < 350) {
      //Backward
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN1, LOW);
      digitalWrite(IN4, LOW);

      //for speed control
      speed = 0;
      speed =  map(x1, 350, 0, 20, 255);
      analogWrite(EN1, speed);
      analogWrite(EN2, speed);
    }

    else if (y1 < 350 ) {
      //For Right
      digitalWrite(IN3, HIGH);
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN4, LOW);

      //For speed control
      speed = 0;
      speed =  map(y1, 350, 0, 20, 255);
      analogWrite(EN1, speed);
      analogWrite(EN2, speed);

    }

    else {
      //STOP THE ROBOT
      digitalWrite(IN4, LOW);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      analogWrite(EN1, 255);
      analogWrite(EN2, 255);
    }
  }

  else {
    digitalWrite(LED, LOW);

    //--Movement(L2393D)--//
    if (y1 > 650) {

      //Forward
      digitalWrite(IN1, HIGH);
      digitalWrite(IN4, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);

      //For speed control
      speed = 0;
      speed =  map(x1, 650, 1023, 20, 255);
      analogWrite(EN1, speed);
      analogWrite(EN2, speed);

    }
    else if (y1 < 350) {
      //Backward
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN1, LOW);
      digitalWrite(IN4, LOW);

      //for speed control
      speed = 0;
      speed =  map(x1, 350, 0, 20, 255);
      analogWrite(EN1, speed);
      analogWrite(EN2, speed);
    }
    else if (x1 < 350)
    {
      //Left
      digitalWrite(IN4, HIGH);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN1, LOW);
      digitalWrite(IN3, LOW);

      //for speed control
      speed = 0;
      speed =  map(y1, 650, 1023, 20, 255);
      analogWrite(EN1, speed);
      analogWrite(EN2, speed);
    }
    else if (650 < x1) {
      //For Right
      digitalWrite(IN3, HIGH);
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN4, LOW);

      //For speed control
      speed = 0;
      speed =  map(y1, 350, 0, 20, 255);
      analogWrite(EN1, speed);
      analogWrite(EN2, speed);

    }
    else {
      //STOP THE ROBOT
      digitalWrite(IN4, LOW);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      analogWrite(EN1, 255);
      analogWrite(EN2, 255);
    }
  }

  //Pan Tilt
  //Confirms Mode (ackData is the mode variable)
  if (ackData[4] == 1) {
    ackData[4] = 0;
    int pos = 0;
    int IRSensor = 37; //(IR Sensor Pin)

    for (pos = 0; pos <= 180; pos += 1) {                  //auto sweep from 0 to 180 degress by 1 degree incr.
      int statusSensor = digitalRead (IRSensor);  //Reads IR Sensor
      if (statusSensor == LOW) {
        pan.detach();                                    //servo detaches when object detected
        Serial.println("Object detected");
        dataToSend[5] = 1;                               //Sends varible for Focus Moudle
      }
      else {
        pan.attach(11);
        pan.write(pos);
        dataToSend[5] = 0;                            //Sends varible for Auto Sweep Mode
      }
    }

    for (pos = 180; pos >= 0; pos -= 1) {                  //auto sweep from 0 to 180 degress by 1 degree incr.
      int statusSensor = digitalRead (IRSensor);
      if (statusSensor == LOW) {
        pan.detach();                                    //servo detaches when object detected
        Serial.println("Object detected");
        dataToSend[5] = 1;;
      }
      else {
        pan.attach(11);
        pan.write(pos);
        dataToSend[5] = 0;
      }
    }
  }

  //Normal Mode
  else {
    //Vertical
    tilt.attach(11);
    tiltVal = map(ackData[3], 0, 1023, 90, 180);
    tilt.write(tiltVal);

    //Horizontal
    pan.attach(12);
    panVal = map(ackData[2], 0, 1023, 0, 180);
    pan.write(panVal);
  }

  //TFT
  TFTscreen.setRotation(1);
  TFTscreen.stroke(255, 255, 224); //Light Yellow Font Colour

  //Printing On TFT
  TFTscreen.text("Shivaji College", 0, 27);
  TFTscreen.text("Robotics Project!", 0, 47);
  TFTscreen.text("Made By:", 0, 67);
  TFTscreen.text("Raghav, Bhavesh, Himanshu", 0, 87);
}

////----NRF Send Function----////
void send() {

  bool rslt;

  // For Radio
  rslt = radio.write( &dataToSend, sizeof(dataToSend) );

  //For Serial Monitor
  Serial.print("Distance From The Obstacle: "); Serial.println(dataToSend[0]);
  Serial.print("Human Motion : "); Serial.println(dataToSend[1]);
  Serial.print("Rain Status: "); Serial.println(dataToSend[2]);
  Serial.print("Temperature: "); Serial.println(dataToSend[3]);
  Serial.print("Humidity: "); Serial.println(dataToSend[4]);
  //If Radio Communication Successful
  if (rslt) {
    if (radio.isAckPayloadAvailable()) {
      radio.read(&ackData, sizeof(ackData));
      newData = true;
    }
    else {
      Serial.println(" Acknowledged But No Data");
    }
    updateMessage();
  }
  else {
    Serial.println("Tx Failed");
  }
  prevMillis = millis(); //Radio Communication Fail

}


////----NRF DATA UPDATE FUNCTION----////
void updateMessage() {
  dataToSend[0] = distance;
  dataToSend[1] = PIR;
  dataToSend[2] = rainstatus;
  dataToSend[3] = t;
  dataToSend[4] = h;
  x1 = ackData[0];
  y1 = ackData[1] ;
  x2 = ackData[2];
  y2 = ackData[3] ;

}


////----RAIN SENSOR DATA FUNCTION----////
int readRainSensor() {
  digitalWrite(sensorPower, HIGH);  // Turn the sensor ON
  delay(10);              // Allow power to settle
  int rainval2 = digitalRead(sensorPin); // Read the sensor output
  digitalWrite(sensorPower, LOW);   // Turn the sensor OFF
  return rainval2;             // Return the value
}


////----SHOWING RECEIVED DATA----////
void showData() {
  if (newData == true) {
    Serial.print("  Acknowledge data ");
    Serial.print(ackData[0]);
    Serial.print(", ");
    Serial.println(ackData[1]);
    Serial.print("X: ");
    Serial.println(ackData[2]);
    Serial.print("Y: ");
    Serial.println(ackData[3]);
    Serial.print("Tilt: ");
    Serial.println(tiltVal);
    Serial.print("Pan: ");
    Serial.println(panVal);
    Serial.println();
    newData = false;
  }
}
