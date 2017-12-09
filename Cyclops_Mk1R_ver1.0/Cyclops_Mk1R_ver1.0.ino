/*
  Copyright (c) 2017 Sng Woei Shyong

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  Email info@machinationstudio.com for any clarification of the code.

  -----

  This sketch uses the VarSpeedServo arduino library. Do not use VarSpeedServo.h and servo.h at the same time.
  https://github.com/netlabtoolkit/VarSpeedServo
  
  This sketch the IRremote arduino library. Do not use the IRremote and RobotIRremote at the same time.
  https://github.com/z3t0/Arduino-IRremote
  Copyright 2009-2012 Ken Shirriff 

  -----

  Timing function:
** All software operations make use of the internal clock function, millis(). Programming with delay() is not advised.

  Thanks to David Teo of Insert Coin http://insertcoin.sg/ for his help with the code.
  Thanks to Annabelle Kwok of Neuralbay https://www.neuralbay.com/ for cleaning up the code and comments.
*/

#include <VarSpeedServo.h> // Do not use VarSpeedServo.h and servo.h at the same time
#include <boarddefs.h>
#include <IRremote.h> // Do not use the IRremote and RobotIRremote at the same time.
#include <IRremoteInt.h>

VarSpeedServo servoRight; // Supports up to 8 servos
VarSpeedServo servoMiddle;
VarSpeedServo servoLeft;
VarSpeedServo servoGun2;

/****************Pin Assignment*****************/
int RECV_PIN = A0;
IRrecv irrecv(RECV_PIN);
decode_results results;
/***********************************************/

/************Variable Initialisation************/
//Controller
boolean isRepeatKey = false;
int controllerButton = 0;
unsigned long controllerCurrentMillis = 0;
unsigned long controllerPreviousMillis = 0;

//Colossus: Four LEDs
int led0State = LOW;
int led1State = LOW;
int led2State = LOW;
int led4State = LOW;

//Action: LED blinking
unsigned long ledPreviousMillis = 0;
unsigned long led0PreviousMillis = 0;
const long led0Interval = 1000;

//Action: Walk
unsigned long walkTimer = 0;
boolean allowWalkRunOnce = true;
/***********************************************/

void setup() {
  //Serial.begin(9600);
  //Serial.println("Launching Setup");

  //Colossus: Gun (initialisation)
  pinMode(8, OUTPUT);   //Gun1
  pinMode(11, OUTPUT);  //Gun3
  digitalWrite(8, LOW);
  digitalWrite(11, LOW);

  //Colossus: Four LEDs (initialisation)
  pinMode(0, OUTPUT);   //red LED
  pinMode(1, OUTPUT);   //yellow LED
  pinMode(2, OUTPUT);   //yellow LED
  pinMode(4, OUTPUT);   //yellow LED
  digitalWrite(0, HIGH);
  digitalWrite(1, HIGH);
  digitalWrite(2, HIGH);
  digitalWrite(4, HIGH);
  //note: Digital pin 3 is reserved for one more LED

  //Controller: IR Receiver
  irrecv.enableIRIn();

  //Colossus: Four Servos (initialisation)
  servoRight.attach(5, 1200, 1800); // attach(pin, min, max ) - Attaches to a pin setting min and max values in microseconds, default min is 544, max is 2400
  servoMiddle.attach(6, 1200, 1800);
  servoLeft.attach(9, 1200, 1800);
  servoGun2.attach(10, 1200, 1800);
  servoRight.write(90, 10, false); // write(value, speed, wait) - value sets the servo angle in 0-180 degrees.  - speed varies the speed of the move to new position 0=full speed, 1-185 slower to faster  - wait is a boolean that, if true, causes the function call to block until move is complete
  servoMiddle.write(90, 10, false);
  servoLeft.write(90, 10, true);
  servoGun2.write(90, 10, true);

  delay(1000);
  //  Serial.println("Finished Setup");
}

void loop() {

  led0();   //red LED
  led1();   //yellow LED
  led2();   //yellow LED
  led4();   //yellow LED

  controllerCurrentMillis = millis();

  /* IR Controller decoded results: 1=FFA25D , 2=FF629D , 3=FFE21D , 4=FF22DD , 5=FF02FD , 6=FFC23D , 7=FFE01F , 8=FFA857 , 9=FF906F , 10(0)=FF9867 , 11(*)=FF6897
    , 12(#)=FFB04F  , 13(UP)=FF18E7 , 14(DOWN)=FF4AB5 , 15(RIGHT)=FF5AA5 , 16(LEFT)=FF10EF , 17(OK)=FF38C7*/

  if (irrecv.decode(&results)) {
    //Serial.print(results.value, HEX);

    switch (results.value) {
      case 0xFF18E7: // IR controller code
        controllerButton = 13; // button number
        //Serial.println("Up");
        controllerPreviousMillis = millis();
        break;

      case 0xFF4AB5:
        controllerButton = 14;
        //Serial.println("Down");
        controllerPreviousMillis = millis();
        break;

      case 0xFF10EF:
        controllerButton = 16;
        //Serial.println("Left");
        controllerPreviousMillis = millis();
        break;

      case 0xFF5AA5:
        controllerButton = 15;
        //Serial.println("Down");
        controllerPreviousMillis = millis();
        break;

      case 0xFFA25D:
        controllerButton = 1;
        //Serial.println("gun1");
        controllerPreviousMillis = millis();
        break;

      case 0xFF629D:
        controllerButton = 2;
        //Serial.println("gun2");
        controllerPreviousMillis = millis();
        break;

      case 0xFFE21D:
        controllerButton = 3;
        //Serial.println("gun3");
        controllerPreviousMillis = millis();
        break;

      case 0xFFFFFFFF:
        isRepeatKey = true;
        //Serial.println("Repeat");
        controllerPreviousMillis = millis();
        break;
    }
    irrecv.resume(); // Receive the next value
  }

  /***********************Debug************************/
  //  Serial.print(" || ");
  //  Serial.print(controllerCurrentMillis - controllerPreviousMillis);

  //  Serial.print(" || ");
  //  Serial.print(controllerButton);

  //  Serial.print(" || ");
  //  Serial.println(isRepeatKey);
  /****************************************************/

  /***********************Action******************************/
  if ( controllerButton == 13 && isRepeatKey ) {
    if (allowWalkRunOnce) {
      allowWalkRunOnce = false;
      walkTimer = millis();
    }
    walkForward();
  }
  else if ( controllerButton == 14 && isRepeatKey ) {
    if (allowWalkRunOnce) {
      allowWalkRunOnce = false;
      walkTimer = millis();
    }
    walkBackward();
  }
  else if ( controllerButton == 16 && isRepeatKey ) {
    if (allowWalkRunOnce) {
      allowWalkRunOnce = false;
      walkTimer = millis();
    }
    walkTurnLeft();
  }
  else if ( controllerButton == 15 && isRepeatKey ) {
    if (allowWalkRunOnce) {
      allowWalkRunOnce = false;
      walkTimer = millis();
    }
    walkTurnRight();
  }
  else if ( controllerButton == 1 && isRepeatKey ) {
    if (allowWalkRunOnce) {
      allowWalkRunOnce = false;
      walkTimer = millis();
    }
    digitalWrite(8, HIGH); // turn on gun1
  }
  else if ( controllerButton == 2 && isRepeatKey ) {
    if (allowWalkRunOnce) {
      allowWalkRunOnce = false;
      walkTimer = millis();
    }
    gun2();
  }
  else if ( controllerButton == 3 && isRepeatKey ) {
    if (allowWalkRunOnce) {
      allowWalkRunOnce = false;
      walkTimer = millis();
    }
    digitalWrite(11, HIGH); // turn on gun3
  }
  else if ( !isRepeatKey ) {
    allowWalkRunOnce = true;
    walkRest();
    digitalWrite(8, LOW); // turn off gun1
    digitalWrite(11, LOW); // turn off gun3
  }

  if (millis() - controllerPreviousMillis > 500) {
    controllerButton = 999; // assigned a number that will not be used
    isRepeatKey = false;
  }
}

void led0() {    //red LED
  unsigned long ledCurrentMillis = millis();

  if (ledCurrentMillis - led0PreviousMillis > led0Interval) {
    led0PreviousMillis = ledCurrentMillis;
    if (led0State == LOW)
      led0State = HIGH;
    else
      led0State = LOW;
    digitalWrite(0, led0State);
  }
}

void led1() {   //yellow LED
  long ledInterval = random(1, 10);
  unsigned long ledCurrentMillis = millis();

  if (ledCurrentMillis - ledPreviousMillis > ledInterval) {
    ledPreviousMillis = ledCurrentMillis;
    if (led1State == LOW)
      led1State = HIGH;
    else
      led1State = LOW;
    digitalWrite(1, led1State);
  }
}

void led2() {   //yellow LED
  long ledInterval = random(1, 10);
  unsigned long ledCurrentMillis = millis();

  if (ledCurrentMillis - ledPreviousMillis > ledInterval) {
    ledPreviousMillis = ledCurrentMillis;
    if (led2State == LOW)
      led2State = HIGH;
    else
      led2State = LOW;
    digitalWrite(2, led2State);
  }
}

void led4() {   //yellow LED
  long ledInterval = random(1, 10);
  unsigned long ledCurrentMillis = millis();

  if (ledCurrentMillis - ledPreviousMillis > ledInterval) {
    ledPreviousMillis = ledCurrentMillis;
    if (led4State == LOW)
      led4State = HIGH;
    else
      led4State = LOW;
    digitalWrite(4, led4State);
  }
}


void walkForward() {
  int walkTimerDifference = millis() - walkTimer;

  if (walkTimerDifference >= 0 && walkTimerDifference < 800) {
    servoRight.write(72, 18, false);
    servoLeft.write(72, 18, false);
  }
  else if (walkTimerDifference >= 800 && walkTimerDifference < 1600) {
    servoMiddle.write(80, 18, false);
  }
  else if (walkTimerDifference >= 1600 && walkTimerDifference < 2400) {
    servoRight.write(108, 18, false);
    servoLeft.write(108, 18, false);
  }
  else if (walkTimerDifference >= 2400 && walkTimerDifference < 3200) {
    servoMiddle.write(100, 18, false);
  }
  else if (walkTimerDifference >= 3200) {
    walkTimer = millis();
  }
}

void walkBackward() {
  int walkTimerDifference = millis() - walkTimer;

  if (walkTimerDifference >= 0 && walkTimerDifference < 800) {
    servoRight.write(108, 18, false);
    servoLeft.write(108, 18, false);
  }
  else if (walkTimerDifference >= 800 && walkTimerDifference < 1600) {
    servoMiddle.write(80, 18, false);
  }
  else if (walkTimerDifference >= 1600 && walkTimerDifference < 2400) {
    servoRight.write(72, 18, false);
    servoLeft.write(72, 18, false);
  }
  else if (walkTimerDifference >= 2400 && walkTimerDifference < 3200) {
    servoMiddle.write(100, 18, false);
  }
  else if (walkTimerDifference >= 3200) {
    walkTimer = millis();
  }
}

void walkTurnLeft() {
  int walkTimerDifference = millis() - walkTimer;

  if (walkTimerDifference >= 0 && walkTimerDifference < 800) {
    servoRight.write(80, 18, false);
    servoLeft.write(100, 18, false);
  }
  else if (walkTimerDifference >= 800 && walkTimerDifference < 1600) {
    servoMiddle.write(80, 18, false);
  }
  else if (walkTimerDifference >= 1600 && walkTimerDifference < 2400) {
    servoRight.write(100, 18, false);
    servoLeft.write(80, 18, false);
  }
  else if (walkTimerDifference >= 2400 && walkTimerDifference < 3200) {
    servoMiddle.write(100, 18, false);
  }
  else if (walkTimerDifference >= 3200) {
    walkTimer = millis();
  }
}

void walkTurnRight() {
  int walkTimerDifference = millis() - walkTimer;

  if (walkTimerDifference >= 0 && walkTimerDifference < 800) {
    servoRight.write(80, 18, false);
    servoLeft.write(100, 18, false);
  }
  else if (walkTimerDifference >= 800 && walkTimerDifference < 1600) {
    servoMiddle.write(100, 18, false);
  }
  else if (walkTimerDifference >= 1600 && walkTimerDifference < 2400) {
    servoRight.write(100, 18, false);
    servoLeft.write(80, 18, false);
  }
  else if (walkTimerDifference >= 2400 && walkTimerDifference < 3200) {
    servoMiddle.write(80, 18, false);
  }
  else if (walkTimerDifference >= 3200) {
    walkTimer = millis();
  }
}

void gun2() {
  int walkTimerDifference = millis() - walkTimer;

  if (walkTimerDifference >= 0 && walkTimerDifference < 500) {
    servoGun2.write(40, 100, false);
  }
  else if (walkTimerDifference >= 500 && walkTimerDifference < 1000) {
    servoGun2.write(140, 100, false);
  }
  else if (walkTimerDifference >= 1000) {
    walkTimer = millis();
  }
}

void walkRest() {
  servoRight.write(90, 18, false);
  servoMiddle.write(90, 18, false);
  servoLeft.write(90, 18, false);
  servoGun2.write(90, 100, false);
}

/*
  The VarSpeedServo.h Arduino library allows the use of up to 8 servos moving asynchronously (because it uses interrupts). In addition, you can set the speed of a move, optionally wait (block) until the servo move is complete, and create sequences of moves that run asynchronously.
  Do not use VarSpeedServo.h and servo.h at the same time.

  This code is an adaptation of the standard Arduino Servo.h library, which was first adapted by Korman and posted on the Arduino forum to add the speed capability. Philip van Allen updated it for Arduino 1.0 + and added the ability to to wait for the move to complete.

  Supports up to 8 servos
  Allows simultaneous, asynchronous movement of all servos
  The speed of a move can be set
  The write() function initiates a move and can optionally wait for completion of the move before returning
  A servo can be sent a sequence of moves (where each move has a position and speed)

  A servo is activated by creating an instance of the Servo class passing the desired pin to the attach() method. The servos are pulsed in the background using the value most recently written using the write() method

  VarSpeedServo - Class for manipulating servo motors connected to Arduino pins. Methods:

  attach(pin )  - Attaches a servo motor to an i/o pin.
  attach(pin, min, max  ) - Attaches to a pin setting min and max values in microseconds
  default min is 544, max is 2400

  write(value)     - Sets the servo angle in degrees.  (invalid angle that is valid as pulse in microseconds is treated as microseconds)
  write(value, speed) - speed varies the speed of the move to new position 0=full speed, 1-185 slower to faster
  write(value, speed, wait) - wait is a boolean that, if true, causes the function call to block until move is complete

  writeMicroseconds() - Sets the servo pulse width in microseconds
  read()      - Gets the last written servo pulse width as an angle between 0 and 180.
  readMicroseconds()  - Gets the last written servo pulse width in microseconds. (was read_us() in first release)
  attached()  - Returns true if there is a servo attached.
  detach()    - Stops an attached servos from pulsing its i/o pin.

  slowmove(value, speed) - The same as write(value, speed), retained for compatibility with Korman's version

  stop() - stops the servo at the current position

  sequencePlay(sequence, sequencePositions); // play a looping sequence starting at position 0
  sequencePlay(sequence, sequencePositions, loop, startPosition); // play sequence with number of positions, loop if true, start at position
  sequenceStop(); // stop sequence at current position
*/
