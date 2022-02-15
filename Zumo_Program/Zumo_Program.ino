#include <Zumo32U4Motors.h>
#include <Zumo32U4LineSensors.h>
#include <Zumo32U4ProximitySensors.h>
#include <Zumo32U4Buzzer.h>
//Serial1 communicates over XBee
//Serial communicates over USB cable
Zumo32U4Motors motors;
Zumo32U4LineSensors sensors;
Zumo32U4Buzzer buzzer;
Zumo32U4ProximitySensors proxSensors;

//Speed Declarations
#define TURN_SPEED        150
#define CALIBERATE_SPEED  200
#define BACKWARD_SPEED    150
#define FORWARD_SPEED     200
#define STOP_SPEED        0                             // Setting engine speed to zero

//Storage and declarations for line sensor used within course
#define QTR_THRESHOLD     750               // microseconds
#define NUM_SENSORS       3                 //declaring sensor count
unsigned int lineSensorValues[NUM_SENSORS]; //array for line sensor values

int roomNumber = 0;
int endCounter = 0;
String foundRooms[50];
String roomDirection;
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
    case 2:
      reachedImpass();
  }
}

void calibrateRobot() {

  sensors.initThreeSensors();
  proxSensors.initFrontSensor();
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
  motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  buzzer.play(">g32>>c32");
  Serial1.println("Calibration Done");
}

void manualControl() {

  incomingByte = Serial1.read();

  if(incomingByte == 'w') {
    Serial1.println("Moving Forward");
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    delay(250);
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
    delay(250);
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 'd') {
    Serial1.println("Turning Right");
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
    delay(250);
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 'q') {
    Serial1.println("Rotating Left");
    motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
    delay(500);
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 'e') {
    Serial1.println("Rotating Right");
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
    delay(500);
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 'b') {
    
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
      motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
      delay(200);
      motors.setSpeeds(STOP_SPEED, STOP_SPEED);
      roomNumber++;
      foundRooms[roomNumber] = "left";
    }

    else
    {
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      delay(200);
      motors.setSpeeds(STOP_SPEED, STOP_SPEED);
      roomNumber++;
      foundRooms[roomNumber] = "right";
    }
    Serial1.println("Room found is on the " + foundRooms[roomNumber]);
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

  else if (incomingByte == 'l') {

    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
    endCounter++;
    if(endCounter == 1) {
      Serial1.println("Reached end " + endCounter);
      Serial1.print(" turning around");
      motors.setSpeeds(-BACKWARD_SPEED, -BACKWARD_SPEED);
      delay(200);
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      delay(2000);
      motors.setSpeeds(STOP_SPEED, STOP_SPEED);
    }
    else if (endCounter == 2) {
      Serial1.println("Reached end " + endCounter);
      Serial1.print(" end of corridor");
    }
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
    if(incomingByte == 'b') {
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
        motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
        delay(200);
        motors.setSpeeds(STOP_SPEED, STOP_SPEED);
        roomNumber++;
        foundRooms[roomNumber] = "left";
      }

      else
      {
        motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
        delay(200);
        motors.setSpeeds(STOP_SPEED, STOP_SPEED);
        roomNumber++;
        foundRooms[roomNumber] = "right";
      }
      Serial1.println("Room found is on the " + foundRooms[roomNumber]);
      Serial1.println("Press C to search room");

      while (incomingByte != 'c')
      {
        incomingByte = (char) Serial1.read();
      }

      if(incomingByte == 'c') {
        searchRoom();
      }
    }
    //if left and right sensor get a black border reading then...
    else if(((lineSensorValues[2] > QTR_THRESHOLD) && (lineSensorValues[0] > QTR_THRESHOLD)) || (lineSensorValues[1] > QTR_THRESHOLD)){ //if center sensor detects black line |sensor 1
      //reached impass function
      robotStatus = 2;
      //delay half a second
      delay(500);
      motors.setSpeeds(0,0);
      } 
    //if left sesnor gets a black reading...
    else if(lineSensorValues[0] > QTR_THRESHOLD){ //left sensor detects line | sensor 0
      //go right for 1/4 second
      motors.setSpeeds(TURN_SPEED, 0);
      delay(250);
      motors.setSpeeds(75, 75);
    //if right sensor gets a black boarder reading...
    }else if(lineSensorValues[2] > QTR_THRESHOLD){ //if right sensor detects black line| sensor 2
      //go ;left for 1/4 second
      motors.setSpeeds(0,  TURN_SPEED);
      delay(250);
      motors.setSpeeds(75, 75);
    }
   else if(incomingByte == 'k') {
      motors.setSpeeds(STOP_SPEED, STOP_SPEED);
      Serial1.println("EMERGENCY STOP!");
      robotStatus = 0;
      Serial1.println("Robot returned to manual mode");
   }
    else {
      motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    }
  

}

void searchRoom() {
  motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
  delay(200);
  motors.setSpeeds(STOP_SPEED, STOP_SPEED);

  for(uint16_t i = 0; i < 120; i++)
  {
    if (i > 30 && i <= 90)
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
  if(proxSensors.countsLeftWithLeftLeds() > 0 || proxSensors.countsFrontWithLeftLeds() > 0 || proxSensors.countsFrontWithRightLeds() > 0 || proxSensors.countsRightWithRightLeds() > 0) {
    Serial1.println("Person found!");
  }
  else {
    Serial1.println("No Person found");
  }
  motors.setSpeeds(-BACKWARD_SPEED, -BACKWARD_SPEED);
  delay(200);
  motors.setSpeeds(STOP_SPEED, STOP_SPEED);

  Serial1.println("If action is complete, press C to continue");
  while (incomingByte != 'c')
  {
    incomingByte = (char) Serial1.read();
  }
}

//when zumo reaches an impass
void reachedImpass(){
  incomingByte = Serial1.read();
  motors.setSpeeds(0, 0); 
  delay(250);
  motors.setSpeeds(-BACKWARD_SPEED, -BACKWARD_SPEED);
  delay(500);
  motors.setSpeeds(0, 0);
  delay(250);
  Serial1.println("Reached Impass");
  Serial1.println("Press Y for corner, T for T-Junction or L for end of hallway");
  switch (incomingByte)
  {
  case 'y':
    Serial1.println("Press A to turn left or D to turn right");
    while ((incomingByte != 'a') && (incomingByte != 'd'))
      {
        incomingByte = (char) Serial1.read();
      }

      if (incomingByte == 'a')
      {
        motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
        delay(500);
        motors.setSpeeds(STOP_SPEED, STOP_SPEED);
        robotStatus = 1;
      }

      else
      {
        motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
        delay(500);
        motors.setSpeeds(STOP_SPEED, STOP_SPEED);
        robotStatus = 1;
      }
    break;
  case 't':
    Serial1.println("T Junction reached, pressed A to turn left or D to turn right");
    while ((incomingByte != 'a') && (incomingByte != 'd'))
      {
        incomingByte = (char) Serial1.read();
      }

      if (incomingByte == 'a')
      {
        motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
        delay(500);
        motors.setSpeeds(STOP_SPEED, STOP_SPEED);
        robotStatus = 1;
      }

      else
      {
        motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
        delay(500);
        motors.setSpeeds(STOP_SPEED, STOP_SPEED);
        robotStatus = 1;
      }
    break;
  case 'l':
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
    endCounter++;
    if(endCounter == 1) {
      Serial1.println("Reached end " + endCounter);
      Serial1.print(" turning around");
      motors.setSpeeds(-BACKWARD_SPEED, -BACKWARD_SPEED);
      delay(200);
      motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
      delay(2000);
      motors.setSpeeds(STOP_SPEED, STOP_SPEED);
    }
    else if (endCounter == 2) {
      Serial1.println("Reached end " + endCounter);
      Serial1.print(" end of corridor");
    }
    break;
  }

}
