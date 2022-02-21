# Imports
import pygame
from threading import Thread
import sys
import serial
import time
MESSAGE = " "
pygame.init()
pygame.font.init()

res = (1200, 800)
# Setting up the serial with the port number and USB port the XBee module is connected to
ser=serial.Serial('/dev/cu.usbserial-AL1RH4J0',9600,timeout=1)

# Thread function to allow the XBee to read in messages
def threadFunc():

   global MESSAGE
   while True:
     msg = ser.readline()
     if len(msg) > 0:
       MESSAGE = msg.decode()
       time.sleep(0.5)

# Thread being created so that the program allows Zumo actions to be performed while also reading in messages
t = Thread(target=threadFunc)
t.daemon = True
t.start()
# Setting up display variables
screen = pygame.display.set_mode(res)
white = (255,255,255)
black = (0,0,0)
width = screen.get_width()
height = screen.get_height()

smallfont = pygame.font.SysFont('Arial',35)

key=''

# While loop that checks for keyboard inputs
while 1:
    for event in pygame.event.get():

        if event.type == pygame.QUIT:
            pygame.quit()
        if event.type == pygame.KEYDOWN:
                key=event.key
        if event.type == pygame.KEYUP:
                key=''
    time.sleep(0.02)
    # If there is a key input then write that input over the Serial
    if key != '':
        ser.write(chr(key).encode())
    # If the input equals any of the manual controls then reset the key variable. This is done because if not then the Zumo performs actions repetitively
    if key == 119 or key == 97 or key == 115 or key == 100 or key == 101 or key == 113 or key == 117:
        key = ''

    # Make the screen white
    screen.fill((255, 255, 255))
    
    # Create a string that stores the message received by the XBee module
    msg_string = MESSAGE
    msg_text = smallfont.render(msg_string, True, black)
    # Show the message on the screen
    screen.blit(smallfont.render(msg_string, True, black), (25, 50))
    
    # List of the manual instructions for the Zumo. In Auto mode Room search is the same and all other commands are asked for my Zumo
    # so the instructions will be shown through the Serial messages sent by the Zumo
    screen.blit(smallfont.render('"w" Forward', True, black) , (10,200))
    screen.blit(smallfont.render('"s" Backwards', True, black) , (10,250))
    screen.blit(smallfont.render('"a" Left', True, black) , (10,300))
    screen.blit(smallfont.render('"d" Right', True, black) , (10,350))
    screen.blit(smallfont.render('"q" 90 degrees left', True, black) , (10,400))
    screen.blit(smallfont.render('"e" 90 degrees right', True, black) , (10,450))
    screen.blit(smallfont.render('"b" Stop for room', True, black) , (10,500))
    screen.blit(smallfont.render('"c" Start room search', True, black) , (10,550))
    screen.blit(smallfont.render('"u" 180 degree turn', True, black) , (10,600))
    screen.blit(smallfont.render('"k" Emergency Stop', True, black) , (10,650))
    screen.blit(smallfont.render('"m" Auto Mode', True, black) , (10,700))
    
      
    # updates the frames of the game
    pygame.display.update()