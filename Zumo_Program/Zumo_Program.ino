#include <Zumo32U4Motors.h>
#include <Zumo32U4LineSensors.h>
#include <Zumo32U4ProximitySensors.h>
#include <Zumo32U4Buzzer.h>
#include <Zumo32U4Encoders.h>
//Serial1 communicates over XBee
//Serial communicates over USB cable
Zumo32U4Motors motors;
Zumo32U4LineSensors sensors;
Zumo32U4Buzzer buzzer;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4Encoders encoders;

//Speed Declarations
#define TURN_SPEED        150
#define CALIBERATE_SPEED  150
#define BACKWARD_SPEED    100
#define FORWARD_SPEED     100
#define STOP_SPEED        0                             // Setting engine speed to zero

//Storage and declarations for line sensor used within course
#define NUM_SENSORS       3                 //declaring sensor count
unsigned int lineSensorValues[NUM_SENSORS]; //array for line sensor values

int roomNumber = 0;
int endCounter;
int calibrateData[3];
int encodersCountLeft;
int encodersCountRight;
String foundRooms[50];
String roomDirection;
boolean enableMessages = true;
int robotStatus;
int incomingByte; // for incoming serial data
void setup() {
  // put your setup code here, to run once:
  Serial1.begin(9600);
  incomingByte = Serial1.read();

  calibrateRobot();

  robotStatus = 0;
  Serial1.println("Manual Mode Active, Press m to activate Auto mode");
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (robotStatus) {
    case 0:
      manualControl();
      break;
    case 1:
      autonomous();
      break;
  }
}

void calibrateRobot() {

  sensors.initThreeSensors();
  delay(100);
  proxSensors.initFrontSensor();
  delay(100);
  encoders.init();
    // Wait 1 second and then begin automatic sensor calibration
  // by rotating in place to sweep the sensors over the line
  delay(1000);
  for(uint16_t i = 0; i < 120; i++)
  {
    if (i > 30 && i <= 90)
    {
      motors.setSpeeds(-CALIBERATE_SPEED, CALIBERATE_SPEED);
    }
    else
    {
      motors.setSpeeds(CALIBERATE_SPEED, -CALIBERATE_SPEED);
    }

    sensors.calibrate();
  }

  for(int i= 0; i < NUM_SENSORS; i++) {
    calibrateData[i] = sensors.calibratedMaximumOn[i];
  }
  motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  buzzer.play(">g32>>c32");
  Serial1.println("Calibration Done");
}

void manualControl() {

  incomingByte = Serial1.read();

  if(incomingByte == 'w') {
    Serial1.println("Moving Forward");
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    delay(300);
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 's') {
    Serial1.println("Moving Backwards");
    motors.setSpeeds(-BACKWARD_SPEED, -BACKWARD_SPEED);
    delay(250);
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 'a') {
    Serial1.println("Turning Left");
    motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
    delay(150);
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 'd') {
    Serial1.println("Turning Right");
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
    delay(150);
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 'q') {
    turnLeft90();
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 'e') {
    turnRight90();
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 'b') {
    logRoom();
    Serial1.println("Press C to search room");

    while (incomingByte != 'c')
    {
      incomingByte = (char) Serial1.read();
    }

    if(incomingByte == 'c') {
      searchRoom();
    }
  }
  else if(incomingByte == 'k') {
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
    Serial1.println("EMERGENCY STOP!");
  }

  else if (incomingByte == 'u') {
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
    Serial1.print("Turning around");
    motors.setSpeeds(-BACKWARD_SPEED, -BACKWARD_SPEED);
    turnLeft90();
    turnLeft90();
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }

  else if(incomingByte == 'm') {
    Serial1.println("Autonomous mode active");
    delay(250);
    robotStatus = 1;
  }
}

void autonomous() {
  incomingByte = Serial1.read();

  sensors.read(lineSensorValues);
  if(incomingByte == 'b' && enableMessages == true) {
    logRoom();
    searchRoom();
  }
  else if (incomingByte == 'p') {
    enableMessages = true;
    Serial1.println("T Junctions passed, Room search enabled");
  }
  else if(incomingByte == 'k') {
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
    Serial1.println("EMERGENCY STOP!");
    robotStatus = 0;
    Serial1.println("Robot returned to manual mode");
  }
  //if left and right sensor get a black border reading then...
  else if(((lineSensorValues[2] > calibrateData[2] + 100) && (lineSensorValues[0] > calibrateData[0] + 100) || (lineSensorValues[1] > calibrateData[1] + 100))){ //if center sensor detects black line |sensor 1
    motors.setSpeeds(0,0);
    endCounter++;
    junction();
  } 
  //if left sesnor gets a black reading...
  else if(lineSensorValues[0] > calibrateData[0] + 100){ //left sensor detects line | sensor 0
    //go right for 1/4 second
    motors.setSpeeds(TURN_SPEED, 0);
  //if right sensor gets a black boarder reading...
  }else if(lineSensorValues[2] > calibrateData[2] + 100){ //if right sensor detects black line| sensor 2
    //go ;left for 1/4 second
    motors.setSpeeds(0,  TURN_SPEED);
  }
  else {
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
  }
}

void searchRoom() {
  motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
  delay(200);
  motors.setSpeeds(STOP_SPEED, STOP_SPEED);

  for(uint16_t i = 0; i < 60; i++)
  {
    if (i > 15 && i <= 45)
    {
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      proxSensors.read();
    }
    else
    {
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      proxSensors.read();
    }
  }
  motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  if(proxSensors.countsLeftWithLeftLeds() > 3 || proxSensors.countsFrontWithLeftLeds() > 3 || proxSensors.countsFrontWithRightLeds() > 3 || proxSensors.countsRightWithRightLeds() > 3) {
    Serial1.println("Person found in room ");
    Serial1.println(roomDirection[roomNumber]);
    buzzer.play(">g32>>c32");
  }
  else {
    Serial1.println(" ");
    Serial1.println("No Person found");
  }
  motors.setSpeeds(-BACKWARD_SPEED, -BACKWARD_SPEED);
  delay(200);
  if(foundRooms[roomNumber] == "left") {
    turnRight90();
  }
  else {
    turnLeft90();
  }
  motors.setSpeeds(STOP_SPEED, STOP_SPEED);
}

void turnRight90() {
  encodersCountRight = 0;
  encodersCountLeft = 0;
  encoders.getCountsAndResetRight();
  encoders.getCountsAndResetLeft();
  motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
  while ((encodersCountRight > -700)) {
    encodersCountRight = encoders.getCountsRight();
  }
  encodersCountRight = encoders.getCountsAndResetRight();
  encodersCountLeft = encoders.getCountsAndResetLeft();
}

void turnLeft90() {
  encodersCountLeft = 0;
  encodersCountRight = 0;
  encoders.getCountsAndResetRight();
  encoders.getCountsAndResetLeft();
  motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
  while ((encodersCountLeft > -700)) {
    encodersCountLeft = encoders.getCountsLeft();
  }
  encodersCountLeft = encoders.getCountsAndResetLeft();
  encodersCountRight = encoders.getCountsAndResetRight();
}

void logRoom() {
  incomingByte = Serial1.read();
  Serial1.println("Stopping for Room");
  Serial1.println("Indicate which direction the room is, press A for left or D for right");
  motors.setSpeeds(STOP_SPEED, STOP_SPEED);
    
  while ((incomingByte != 'a') && (incomingByte != 'd'))
  {
    incomingByte = (char) Serial1.read();
  }

  // Save the room number and the side of it if user presses a is left but in other situations is right
  if (incomingByte == 'a')
  {
    turnLeft90();
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
    roomNumber++;
    foundRooms[roomNumber] = "left";
  }

  else
  {
    turnRight90();
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
    roomNumber++;
    foundRooms[roomNumber] = "right";
  }


  Serial1.print("Room ");
  Serial1.print(roomNumber);
  Serial1.print(" is on the ");
  Serial1.print(foundRooms[roomNumber]);
  Serial1.println(" ");
}

void junction() {
  incomingByte = Serial1.read();

  if(endCounter <= 2) {
    Serial1.println("Press A to turn left or D to turn right");

    while ((incomingByte != 'a') && (incomingByte != 'd'))
    {
      incomingByte = (char) Serial1.read();
    }

    if (incomingByte == 'a')
    {
      Serial1.println("Turning left");
      turnLeft90();
    }
    else 
    {
      Serial1.println("Turning right");
      turnRight90();
    }
    //Go back to auto mode
    robotStatus = 1;
  }
  else if (endCounter == 3) {
    Serial1.println("Press U to complete a 180 degree turn");
    while ((incomingByte != 'u'))
    {
      incomingByte = (char) Serial1.read();
    }
    Serial.println("Starting 180 degree turn...");
    turnLeft90();
    turnLeft90();
    robotStatus = 1;
    // Ensure that the zumo can no longer search
    enableMessages = false;
  }
  else if (endCounter == 4) {
    Serial1.println("End of Hallway.");
    robotStatus = 0;
    endCounter = 0;
  }
}
