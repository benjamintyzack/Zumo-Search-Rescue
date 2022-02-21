/* Zumo Search and Rescue
 This program performs a Search and Rescue using a Zumo32u4. It includes a manual mode and autonomous mode.
 The Communication is done via XBee modules on serial port 9600

 Manual Mode:
 wasd - move Zumo around
 b - stop to log a room
 c - perform a room search
 q & e - Perform a 90 degree turn either left or right
 k - Emergency Stop
 u - Perform a 180 degree turn
 m - switch to automatic mode

 Automatic mode:
 The Zumo will navigate the corridor by itself using the line sensors to detect the corridor walls.
 Each time it gets to a dead end it will ask the user what action to perform.
 For a corner or T Junction it will ask which direction to turn and at a end of the corridor it
 will ask to perform a U Turn
 */


// External Libraries
#include <Zumo32U4Motors.h>
#include <Zumo32U4LineSensors.h>
#include <Zumo32U4ProximitySensors.h>
#include <Zumo32U4Buzzer.h>
#include <Zumo32U4Encoders.h>
//Serial1 communicates over XBee
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
String foundRooms[15];
String roomDirection;
boolean enableMessages = true; // Boolean for whether room searching is enabled
int robotStatus;
int incomingByte; // for incoming serial data
void setup() {
  // Serial1 takes in the port number 9600 so it can listen for incoming messages
  Serial1.begin(9600);
  incomingByte = Serial1.read();

  calibrateRobot();
  // Robot starts in manual mode and can be switched at anytime
  robotStatus = 0;
  Serial1.println("Manual Mode Active, Press m to activate Auto mode");
}

void loop() {
  // Switch case used to what controls to perform for which robot status
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

  // Initialise the line sensors, the proximity sensors and encoders
  sensors.initThreeSensors();
  delay(100);
  proxSensors.initFrontSensor();
  delay(100);
  encoders.init();
  // Wait 1 second and then begin automatic sensor calibration
  // Zumo will rotate in place to calibrate sensors
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
  // The maximum values detected by the line sensors are stored in an array
  for(int i= 0; i < NUM_SENSORS; i++) {
    calibrateData[i] = sensors.calibratedMaximumOn[i];
  }
  motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  buzzer.play(">g32>>c32");
  Serial1.println("Calibration Done");
}

void manualControl() {

  // Basic manual controls for the Zumo, whatever the incomingByte is checked and the relevant action is performed
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
/* 
Autonomous Mode: 
The Zumo will navigate by itself down the corridor
If either the right or left line sensor detects a black line it will adjust its course to
stay within the lines
If both the right and left sensor detect a black line or the middle sensor detects a black line then
it will stop and ask the user what task to perform
*/
void autonomous() {
  incomingByte = Serial1.read();

  sensors.read(lineSensorValues);
  if(incomingByte == 'b' && enableMessages == true) {
    logRoom();
    searchRoom();
  }
  // when the Zumo has reached the first end of the corridor it will wait for the user to press 'p' before being able to do a room search again
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
  //if left and right sensor detects a black line or if the centre sensor detects a black line
  else if(((lineSensorValues[2] > calibrateData[2] + 100) && (lineSensorValues[0] > calibrateData[0] + 100) || (lineSensorValues[1] > calibrateData[1] + 100))){
    motors.setSpeeds(0,0);
    endCounter++;
    // Has reached a corner, T junction or end of hallway
    junction();
  } 
  //if left sensor detects a black line
  else if(lineSensorValues[0] > calibrateData[0] + 100){
    //go right for 1/4 second
    motors.setSpeeds(TURN_SPEED, 0);
  //if right sensor detects a black line
  }else if(lineSensorValues[2] > calibrateData[2] + 100){
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

  // Similiar to the calibration at the start the Zumo will turn from side to side while firing off the proximity sensors
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
  // Checking whether the proximity sensor values are above 3. If so then there is a person in the room
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
  // Reset the encoder values to 0 and the actual encoders
  encodersCountRight = 0;
  encodersCountLeft = 0;
  encoders.getCountsAndResetRight();
  encoders.getCountsAndResetLeft();
  motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
  // While the value of the encoders is not -700 keep the Zumo turning and read in the encoder value
  while ((encodersCountRight > -700)) {
    encodersCountRight = encoders.getCountsRight();
  }
  encodersCountRight = encoders.getCountsAndResetRight();
  encodersCountLeft = encoders.getCountsAndResetLeft();
}

void turnLeft90() {
  // Reset the encoder values to 0 and the actual encoders
  encodersCountLeft = 0;
  encodersCountRight = 0;
  encoders.getCountsAndResetRight();
  encoders.getCountsAndResetLeft();
  motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
  // While the value of the encoders is not -700 keep the Zumo turning and read in the encoder value
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
  
  // While the input doesn't equal left or right just keep reading in the input but don't do anything
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

  // Print out the room number and the direction of the room, had to print it out like this as there was issues in printing it in one line
  Serial1.print("Room ");
  Serial1.print(roomNumber);
  Serial1.print(" is on the ");
  Serial1.print(foundRooms[roomNumber]);
  Serial1.println(" ");
}

// Function for when the Zumo hits a dead end. Will ask what task to perform based of what the end counter equals to 
void junction() {
  incomingByte = Serial1.read();


  // if the end count less then two then the Zumo is at either a corner or the T Junction and will ask to turn left or right
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
  // if the end count is at 3 it's reached the first end of the corridor and asks to perform a U-turn
  else if (endCounter == 3) {
    Serial1.println("Press U to complete a 180 degree turn");
    while ((incomingByte != 'u'))
    {
      incomingByte = (char) Serial1.read();
    }
    Serial1.println("Starting 180 degree turn...");
    turnLeft90();
    turnLeft90();
    robotStatus = 1;
    // Ensure that the zumo can no longer search
    enableMessages = false;
    Serial1.println("Press P when passed the T Junction to re-enable room searching");
  }
  // When end count is 4 the Zumo has reached the end of the journey
  else if (endCounter == 4) {
    Serial1.println("End of Hallway.");
    robotStatus = 0;
    endCounter = 0;
  }
}
