# Zumo-Search-Rescue Assignment
This is an individual project for the Zumo32u4 robot performing a search and rescue mission down a homemade map of a corridor. The corridor includes a 90 degree turn and the beginning and a T junction. The Zumo can be controlled manually through keyboard inputs or can be set to drive autonomously down the corridor. While going down the corridor it'll check rooms for 'people' to rescue.

The GUI was created using pyGame and allows the user to control the Zumo robot wirelessly and to see the messages sent by the Zumo robot over the XBee modules. The controls for the manual mode are shown on the GUI and the autonomous instructions get sent by the Zumo when an action is required.

# Getting Started
Prerequisites
1. Install Arduino Software from https://www.arduino.cc/en/software
2. Install Python 3 https://www.python.org/downloads/

# Setup
1. Clone the project onto your computer (or download ZIP file)
2. Open Arduino and in the Sketch tab click on 'include Library' and install the Zumo32U4 library
3. Open the project in Arduino
4. Plug your Zumo into your computer, select the port it is connected to under the Tools tab and upload the program to the Zumo
5. Open up a Terminal Prompt and use PIP to install pySerial and pyGame libraries
6. Make sure your XBee modules are paired and are on the prot 9600
7. Turn on the Zumo and run the python GUI

# Author
Benjamin Tyzack

# Video Demo
https://youtu.be/lbmkDrb9qcc
