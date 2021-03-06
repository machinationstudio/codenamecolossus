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

This sketch uses the VarSpeedServo.h arduino library. Do not use VarSpeedServo.h and servo.h at the same time.

Email info@machinationstudio.com for any clarification of the code.

-----

* Timing function:
** All software operations make use of the internal clock function, millis(). Programming with delay() is not advised. 

Thanks to David Teo of Insert Coin http://insertcoin.sg/ for teaching me how to do this.

-----

* Idle mode function:
** When no buttons or toggle switches are turned activated for more than 5 minutes, this function performs a short sequence. Repeats every 5 minutes.

Thanks to Annabelle Kwok of Smart Cow http://www.smartcow.ai/ for teaching me how to do this.
*/

#include <VarSpeedServo.h> // Do not use VarSpeedServo.h and servo.h at the same time

VarSpeedServo servoRight; // Supports up to 8 servos
VarSpeedServo servoMiddle;
VarSpeedServo servoLeft;
VarSpeedServo servoGun2;

/****************Pin Assignment*****************/
const int buttonForwardPin = A0;
const int buttonBackwardPin = A1;
const int buttonTurnLeftPin = A2;
const int buttonTurnRightPin = A3;
const int toggleGun1Pin = A4;
const int toggleGun2Pin = A5;
const int toggleGun3Pin = 7;
/***********************************************/

/************Variable Initialisation************/
//Controller: Four Buttons
int buttonForwardState = HIGH; // Using Arduino's internal pull up resistor with Press to Off buttons.
int buttonBackwardState = HIGH;
int buttonTurnLeftState = HIGH;
int buttonTurnRightState = HIGH;

//Controller: Three Lever Switches
int toggleGun1State = HIGH; // Gun1 is the Gatling Cannon
int toggleGun2State = HIGH; // Gun2 is the Quad AA guns
int toggleGun3State = HIGH; // Gun3 is the 4 Lewis gun

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

//Action: Gun
unsigned long gunTimer = 0;
boolean allowGunRunOnce = true;

//Action: Idle Mode
bool idleBoolean = false;
bool idleBoolean_Gun2 = false;
unsigned long idleClockStart = 0;
/***********************************************/


void setup() {
  //Serial.begin(9600); //Serial.println("Launching Setup");

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

  //Controller: Four Buttons (initialisation)
  pinMode(buttonForwardPin, INPUT_PULLUP); // Using Arduino's internal pull up resistor to reduce component count.
  pinMode(buttonBackwardPin, INPUT_PULLUP);
  pinMode(buttonTurnLeftPin, INPUT_PULLUP);
  pinMode(buttonTurnRightPin, INPUT_PULLUP);

  //Controller: Three Lever Switches (initialisation)
  pinMode(toggleGun1Pin, INPUT_PULLUP);
  pinMode(toggleGun2Pin, INPUT_PULLUP);
  pinMode(toggleGun3Pin, INPUT_PULLUP);

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
  //Serial.println("Finished Setup");
}

void loop() {

  led0();   //red LED
  led1();   //yellow LED
  led2();   //yellow LED
  led4();   //yellow LED

  /******Re-initialisation of buttons and lever switches******/
  //All unpressed buttons and lever switches are in the LOW state

  buttonForwardState = !digitalRead(buttonForwardPin);
  buttonBackwardState = !digitalRead(buttonBackwardPin);
  buttonTurnLeftState = !digitalRead(buttonTurnLeftPin);
  buttonTurnRightState = !digitalRead(buttonTurnRightPin);

  toggleGun1State = !digitalRead(toggleGun1Pin);
  toggleGun2State = !digitalRead(toggleGun2Pin);
  toggleGun3State = !digitalRead(toggleGun3Pin);
  /***********************************************************/

  /***********************Action: Walk************************/
  if ((buttonForwardState == HIGH) && (buttonBackwardState == LOW) && (buttonTurnLeftState == LOW) && (buttonTurnRightState == LOW)) {
    if (allowWalkRunOnce) {
      allowWalkRunOnce = false;
      walkTimer = millis();
    }
    walkForward();
  }
  else if ((buttonForwardState == LOW) && (buttonBackwardState == HIGH) && (buttonTurnLeftState == LOW) && (buttonTurnRightState == LOW)) {
    if (allowWalkRunOnce) {
      allowWalkRunOnce = false;
      walkTimer = millis();
    }
    walkBackward();
  }
  else if ((buttonForwardState == LOW) && (buttonBackwardState == LOW) && (buttonTurnLeftState == HIGH) && (buttonTurnRightState == LOW)) {
    if (allowWalkRunOnce) {
      allowWalkRunOnce = false;
      walkTimer = millis();
    }
    walkTurnLeft();
  }
  else if ((buttonForwardState == LOW) && (buttonBackwardState == LOW) && (buttonTurnLeftState == LOW) && (buttonTurnRightState == HIGH)) {
    if (allowWalkRunOnce) {
      allowWalkRunOnce = false;
      walkTimer = millis();
    }
    walkTurnRight();
  }
  else {
    allowWalkRunOnce = true;
    walkRest();
  }
  /***********************************************************/

  /************************Action: Gun************************/
  if (toggleGun1State == HIGH) {
    digitalWrite(8, HIGH);
  }
  else {
    digitalWrite(8, LOW);
  }

  if (toggleGun2State == HIGH) {
    if (allowGunRunOnce) {
      allowGunRunOnce = false;
      gunTimer = millis();
    }
    gun2();
  }
  else {
    allowGunRunOnce = true;
    if (idleBoolean_Gun2 == false) {
      gun2Rest();
    }
  }

  if (toggleGun3State == HIGH) {
    digitalWrite(11, HIGH);
  }
  else {
    digitalWrite(11, LOW);
  }
  /***********************************************************/

  /*********************Action: Idle Mode*********************/
  if ((buttonForwardState == LOW) && (buttonBackwardState == LOW) && (buttonTurnLeftState == LOW) && (buttonTurnRightState == LOW) && (toggleGun1State == LOW) && (toggleGun2State == LOW) && (toggleGun3State == LOW)) {
    if (idleBoolean) {
      idleBoolean = false;
      idleClockStart = millis();
    }
    idleMode();
    //Serial.print("Launch idleMode:"); //Serial.print("\t");
  }
  else {
    idleBoolean = true;
    idleBoolean_Gun2 = false;
  }
  /***********************************************************/
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
    servoMiddle.write(105, 18, false); // 5 degrees added to offset weight difference between the sides
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
    servoMiddle.write(105, 18, false); // 5 degrees added to offset weight difference between the sides
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
    servoMiddle.write(105, 18, false); // 5 degrees added to offset weight difference between the sides
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
    servoMiddle.write(105, 18, false); // 5 degrees added to offset weight difference between the sides
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

void walkRest() {
  servoRight.write(90, 18, false);
  servoMiddle.write(90, 18, false);
  servoLeft.write(90, 18, false);
}

void gun2() {
  int gunTimerDifference = millis() - gunTimer;

  if (gunTimerDifference >= 0 && gunTimerDifference < 500) {
    servoGun2.write(40, 100, false);
  }
  else if (gunTimerDifference >= 500 && gunTimerDifference < 1000) {
    servoGun2.write(140, 100, false);
  }
  else if (gunTimerDifference >= 1000) {
    gunTimer = millis();
  }
}

void gun2Rest() {
  servoGun2.write(90, 100, false);
}

void idleMode() {

  /***************Time Assignments in Idle Mode***************/
  /* Order of actions in idle mode:
      Idle mode will launch after 5min (300,000 milliseconds) of inactivity
      Action1: Walk for 900 milliseconds, followed by a 1500 millisecond wait
      Action2: Gun1 for 5000 milliseconds, followed by a 1500 millisecond wait
      Action3: Gun2 for 3900 milliseconds, followed by a 1500 millisecond wait
      Action4: Gun3 for 1500 milliseconds
  */
  unsigned long idleWalkStart = 300000;    //idle mode will launch in 5min (300,000 milliseconds)
  unsigned long idleWalkEnd = (idleWalkStart + 900);
  unsigned long idleGun1Start = (idleWalkEnd + 1500);
  unsigned long idleGun1End = (idleGun1Start + 5000);
  unsigned long idleGun2Start = (idleGun1End + 1500);
  unsigned long idleGun2End = (idleGun2Start + 3900);
  unsigned long idleGun3Start = (idleGun2End + 1500);
  unsigned long idleGun3End = (idleGun3Start + 1500);
  unsigned long idleLastAction = idleGun3End;
  /***********************************************************/

  unsigned long idleTimer = millis() - idleClockStart;
  //Serial.print(idleTimer); //Serial.print("\t");


  //Action 1 : Walk
  if (idleTimer >= idleWalkStart && idleTimer <= idleWalkEnd) {
    unsigned long idleWalkTimer = idleTimer - idleWalkStart;

    if (idleWalkTimer >= 0 && idleWalkTimer < 800) {
      servoMiddle.write(80, 18, false);
      //Serial.print("Walk step 1 \t");
    }
    else if (idleWalkTimer >= 800 && idleWalkTimer < 850) {
      servoMiddle.write(95, 150, false);
      //Serial.print("Walk step 2 \t");
    }
    else if (idleWalkTimer >= 850 && idleWalkTimer <= 900) {
      servoMiddle.write(90, 18, false);
      //Serial.print("Walk step 3 \t");
    }
  }


  //Action 2: Gun1
  if (idleTimer >= idleGun1Start && idleTimer <= idleGun1End) {
    digitalWrite(8, HIGH);
    //Serial.print("Gun1 ON \t");
  }

  if (idleTimer < idleGun1Start || idleTimer > idleGun1End) {
    digitalWrite(8, LOW);
    //Serial.print("Gun1 OFF");
  }


  //Action 2: Gun2
  if (idleTimer >= idleGun2Start && idleTimer <= idleGun2End) {
    unsigned long idleGun2Timer = idleTimer - idleGun2Start;
    idleBoolean_Gun2 = true;
    int interval = 700;

    if (idleGun2Timer >= 0 && idleGun2Timer < 400) {
      //Serial.print("Gun2 step 1 \t");
      servoGun2.write(40, 100, false);
    }
    else if (idleGun2Timer >= 400 && idleGun2Timer < (400 + interval)) {
      //Serial.print("Gun2 step 2 \t");
      servoGun2.write(140, 100, false);
    }
    else if (idleGun2Timer >= (400 + interval) && idleGun2Timer < (400 + interval * 2)) {
      //Serial.print("Gun2 step 1 \t");
      servoGun2.write(40, 100, false);
    }
    else if (idleGun2Timer >= (400 + interval * 2) && idleGun2Timer < (400 + interval * 3)) {
      //Serial.print("Gun2 step 2 \t");
      servoGun2.write(140, 100, false);
    }
    else if (idleGun2Timer >= (400 + interval * 3) && idleGun2Timer < (400 + interval * 4)) {
      //Serial.print("Gun2 step 1 \t");
      servoGun2.write(40, 100, false);
    }
    else if (idleGun2Timer >= (400 + interval * 4) && idleGun2Timer <= (400 + interval * 5)) {
      //Serial.print("Gun2 step 2 \t");
      servoGun2.write(140, 100, false);
    }
  }
  if (idleTimer < idleGun2Start || idleTimer > idleGun2End) {
    servoGun2.write(90, 100, false);
  }


  //Action 3: Gun3
  if (idleTimer >= idleGun3Start && idleTimer <= idleGun3End) {
    digitalWrite(11, HIGH);
    //Serial.print("Gun3 ON \t");
  }
  if (idleTimer < idleGun3Start || idleTimer > idleGun3End) {
    digitalWrite(11, LOW);
    //Serial.print("Gun3 OFF \t");
  }


  //RESET IDLE TIMER
  if (idleTimer > idleLastAction) {
    idleClockStart = millis();
  }
  //Serial.println("  **");
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
