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
#define REVERSE_SPEED     50
#define TURN_SPEED        150
#define CALIBERATE_SPEED  200
#define BACKWARD_SPEED    150
#define FORWARD_SPEED     140
#define STOP_SPEED        0                             // Setting engine speed to zero

int roomNumber = 0;
String foundRooms[50];
String roomDirection;
int robotStatus;
int incomingByte; // for incoming serial datawwww
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  incomingByte = Serial1.read();

  calibrateRobot();

  robotStatus = 0;
  Serial1.println("Manual Mode Active, Press m to activate Auto mode");
  Serial.println("Manual Mode Active, Press m to activate Auto mode");
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (robotStatus) {
    case 0:
      manualControl();
      break;
  }
}

void calibrateRobot() {

  sensors.initFiveSensors();
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
  Serial.println("Calibration Done");
}

void manualControl() {

  incomingByte = Serial1.read();

  if(incomingByte == 'w') {
    Serial1.println("Moving Forward");
    Serial.println("Moving Forward");
    motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
    delay(250);
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 's') {
    Serial1.println("Moving Backwards");
    Serial.println("Moving Backwards");
    motors.setSpeeds(-BACKWARD_SPEED, -BACKWARD_SPEED);
    delay(250);
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 'a') {
    Serial1.println("Turning Left");
    Serial.println("Turning Left");
    motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
    delay(400);
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 'd') {
    Serial1.println("Turning Right");
    Serial.println("Turning Right");
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
    delay(400);
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 'q') {
    Serial1.println("Rotating Left");
    Serial.println("Rotating Left");
    motors.setSpeeds(-200, 200);
    delay(400);
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 'e') {
    Serial1.println("Rotating Right");
    Serial.println("Rotating Right");
    motors.setSpeeds(200, -200);
    delay(400);
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
  }
  else if(incomingByte == 'b') {
    
    Serial1.println("Stopping for Room");
    Serial.println("Stopping for Room");
    Serial1.println("Indicate which direction the room is, press A for left or D for right");
    Serial.println("Indicate which direction the room is, press A for left or D for right");
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
    
    while ((incomingByte != 'a') && (incomingByte != 'd'))
    {
      incomingByte = (char) Serial1.read();
    }

    // Save the room number and the side of it if user presses a is left but in other situations is right
    if (incomingByte == 'a')
    {
      roomNumber++;
      foundRooms[roomNumber] = "left";
    }

    else
    {
      roomNumber++;
      foundRooms[roomNumber] = "right";
    }
    Serial1.println(roomNumber);
    Serial1.println("Direction " + foundRooms[roomNumber]);

    searchRoom();
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
      motors.setSpeeds(-CALIBERATE_SPEED, CALIBERATE_SPEED);
      proxSensors.read();
    }
    else
    {
      motors.setSpeeds(CALIBERATE_SPEED, -CALIBERATE_SPEED);
      proxSensors.read();
    }
  
  }
}
