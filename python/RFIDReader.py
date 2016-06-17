#/usr/bin/env python

"""
Example code on accessing the RFID Reader 125kHz and reading data from tags

The code here is experimental, and is not intended to be used
in a production environment. It demonstrates the basics of what is
Required to get the Raspberry Pi receiving RFID data and configuring
the RFID Reader parameters.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation as version 2 of the License.

For more informaiton refer to www.cogniot.eu

Available commands:
  z - Display firmware version information
  S - Acknowledge presence of Tag
  F - Perform a Factory Reset
  P - Program EEPROM Polling delay
  v - Select reader operating mode
  R - Read Tag and PAGE 00 data
  r - Read Tag and BLOCK 00-03 data
  e - Exit program
  
"""

import wiringpi2
import time
import sys

# set for GPIO Pin to use based on the jumper connection
GPIO_PIN = 1 # Jumper 1, also known as GPIO18
# GPIO_PIN = 0 # Jumper 2, also known as GPIO17
# GPIO_PIN = 2 # Jumper 3, also known as GPIO21 (Rv 1) or GPIO27 (Rv 2)
# GPIO_PIN = 3 # Jumper 4, also known as GPIO22


def WaitForCTS():
    # continually monitor the selected GPIO pin and wait for the line to go low
    # print "Waiting for CTS" # Added for debug purposes
    while wiringpi2.digitalRead(GPIO_PIN):
        # do nothing
        time.sleep(0.001)
    return

def ReadText(fd):
    # read the data back from the serial line and return it as a string to the calling function
    qtydata = wiringpi2.serialDataAvail(fd)
    # print "Amount of data: %d bytes" % qtydata # Added for debug purposes
    response = ""
    while qtydata > 0:
        # while there is data to be read, read it back
        # print "Reading data back %d" % qtydata #Added for Debug purposes
        response = response + chr(wiringpi2.serialGetchar(fd))
        qtydata = qtydata - 1   
    return response

def ReadInt(fd):
    # read a single character back from the serial line
    qtydata = wiringpi2.serialDataAvail(fd)
    # print "Amount of data: %s bytes" % qtydata  # Added for debug purposes
    response = 0
    if qtydata > 0:
        # print "Reading data back %d" % qtydata #Added for Debug purposes
        response = wiringpi2.serialGetchar(fd)
    return response

def RFIDSetup():
    # setup up the serial port and the wiringpi software for use
    # call setup for the wiringpi2 software
    response = wiringpi2.wiringPiSetup()
    # set the GPIO pin for input
    wiringpi2.pinMode(GPIO_PIN, 0)
    # open the serial port and set the speed accordingly
    fd = wiringpi2.serialOpen('/dev/ttyAMA0', 9600)

    # clear the serial buffer of any left over data
    wiringpi2.serialFlush(fd)
    
    if response == 0 and fd >0:
        # if wiringpi is setup and the opened channel is greater than zero (zero = fail)
        print "PI setup complete on channel %d" %fd
    else:
        print "Unable to Setup communications"
        sys.exit()
        
    return fd

def ReadVersion(fd):
    # read the version from the RFID board
    WaitForCTS()
    # print "Sending Read Version Command" #Added for Debug purposes
    wiringpi2.serialPuts(fd,"z")
    time.sleep(0.1)
    ans = ReadText(fd)
    print "Response: %s" % ans

def ReadTagStatus(fd):
    # read the RFID reader until a tag is present
    notag = True
    while notag:
        WaitForCTS()
        # print "Sending Tag Status Command" #Added for Debug purposes
        wiringpi2.serialPuts(fd,"S")
        time.sleep(0.1)
        ans = ReadInt(fd)
        # print "Tag Status: %s" % hex(ans) # Added for Debug purposes
        if ans == int("0xD6", 16):
            # D6 is a positive response meaning tag present and read
            notag = False
    print "Tag Status: %s" % hex(ans)
    return

def FactoryReset(fd):
    # send the factory reset command
    WaitForCTS()
    # print "Performing a factory reset ...." #Added for Debug purposes
    wiringpi2.serialPutchar(fd, 0x46)
    wiringpi2.serialPutchar(fd, 0x55)
    wiringpi2.serialPutchar(fd, 0xAA)
    time.sleep(0.1)
    print "FACTORY RESET COMPLETE "
    print ""

def SetPollingDalay(fd):
    # set the polling delay for the reader
    print "Setting Polling delay ......."
    WaitForCTS()

    wiringpi2.serialPutchar(fd, 0x50)
    wiringpi2.serialPutchar(fd, 0x00)
    # various polling delays possible, standard one uncommented
    #wiringpi2.serialPutchar(fd, 0x00) # 0x00 is no delay
    #wiringpi2.serialPutchar(fd, 0x20) # 0x20 is approx 20ms
    #wiringpi2.serialPutchar(fd, 0x40) # 0x40 is approx 65ms
    wiringpi2.serialPutchar(fd, 0x60) # 0x60 is approx 262ms
    #wiringpi2.serialPutchar(fd, 0x80) # 0x60 is approx 1 Seconds
    #wiringpi2.serialPutchar(fd, 0xA0) # 0x60 is approx 4 Seconds

    time.sleep(0.1)
    ans = ReadInt(fd)
    # print "Tag Status: %s" % hex(ans) # Added for Debug Purposes 
    if ans == int("0xC0", 16):
        # C0 is a positive result
        print "Polling delay changed ......"
    else:
        print "Unexpected response %s" % hex(ans)
        # flush any remaining characters from the buffer
        wiringpi2.serialFlush(fd)

def ReadTagPageZero(fd):
    # read the tag page 00 command
    notag = True

    print "Reading Tag Data Page 00......."

    print "Waiting for a tag ...."

    notag = True
    while notag:
        WaitForCTS()
        # print "Sending Tag Read Page Command" #Added for Debug purposes
        wiringpi2.serialPutchar(fd, 0x52)
        wiringpi2.serialPutchar(fd, 0x00)
        time.sleep(0.1)
        ans = ReadInt(fd)
        # print "Tag Status: %s" % hex(ans) #Added for Debug purposes
        if ans == int("0xD6", 16):
            # Tag present and read
            notag = False
            # print "Tag Present" #Added for Debug purposes
            ans = ReadText(fd)
            print "Page 00"
            print "-->%s<--" %ans
    return

def ReadTagAndBlocks(fd):
    # read the tag and all blocks from within it
    # Only works for HS/1 as other tags don't support it
    notag = True

    print "Reading Tag Data Blocks 00, 01, 02, 03 ......."

    print "Waiting for a tag ...."

    notag = True
    while notag:
        WaitForCTS()
        # print "Sending Tag Read Blocks command" #Added for Debug purposes
        wiringpi2.serialPutchar(fd, 0x74)
        wiringpi2.serialPutchar(fd, 0x04)
        time.sleep(0.1)
        ans = ReadInt(fd)
        # print "Tag Status: %s" % hex(ans) #Added for Debug purposes
        if ans == int("0xD6", 16):
            # Tag present and read
            notag = False
            # print "Tag Present" #Added for Debug purposes
            ans = ReadText(fd)
            print "Blocks 00, 01, 02, 03"
            print "-->%s<--" % ans
    return

def ChangeReaderOpMode(fd):
    # prvide an additional menu to choose the type of tag to be read and set the reader accordingly
    print "Setting Reader Operating Tag Mode......."

    desc = ""
    choice = ""
    while choice == "":
        print "*********************************************"
        print "a - Hitag H2"
        print "b - Hitag H1/S (factory default)"
        print "c - EM/MC2000"
        # promt the user for a choice
        choice = raw_input("Please select tag type .....:")
        # print "choice: %s" %choice # Added for Debug purposes
        
        if choice =="a" or choice == "A":
            desc = "Hitag H2"
            WaitForCTS()
            wiringpi2.serialPutchar(fd, 0x76)
            wiringpi2.serialPutchar(fd, 0x01) # 0x01 = H2
        elif choice =="b" or choice == "B":
            desc = "Hitag H1/S"
            WaitForCTS()
            wiringpi2.serialPutchar(fd, 0x76)
            wiringpi2.serialPutchar(fd, 0x02) # 0x01 = H1/S
        elif choice =="c" or choice == "C":
            desc = "Em / MC2000"
            WaitForCTS()
            wiringpi2.serialPutchar(fd, 0x76)
            wiringpi2.serialPutchar(fd, 0x03) # 0x03 = EM/MC2000
        else:
            print "Invalid option.  Please try again..."
            choice = ""

    time.sleep(0.1)
    ans = ReadInt(fd)
    # print "Tag Status: %s" % hex(ans) #Added for Debug purposes
    if ans == int("0xC0", 16):
        # Positive result
        print "Reader Operating Mode %s ......" % desc
    else:
        print "Unexpected response %s" % hex(ans)
        # clear the buffer
        wiringpi2.serialFlush(fd)

def HelpText():
    # show the help text
    print "**************************************************************************\n"
    print "Available commands: -"
    print "z - Display firmware version information"
    print "S - Acknowledge presence of Tag"
    print "F - Perform a Factory Reset"
    print "P - Program EEPROM Polling delay"
    print "v - Select reader operating mode"
    print "R - Read Tag and PAGE 00 data"
    print "r - Read Tag and BLOCK 00-03 data"
    print "e - Exit program"



# main code loop

print "Bostin Technology Ltd"
print "Cogniot Products"
print "PirFlx"
print ""
print "Press h for help"
print ""

comms = RFIDSetup()

while True:
    choice = raw_input ("Select Menu Option:")

    if choice == "H" or choice == "h":
        HelpText()
    elif choice == "z":
        ReadVersion(comms)
    elif choice == "S":
        ReadTagStatus(comms)
    elif choice == "F":
        FactoryReset(comms)
    elif choice == "P":
        SetPollingDalay(comms)
    elif choice == "v":
        ChangeReaderOpMode(comms)
    elif choice == "R":
        ReadTagPageZero(comms)
    elif choice == "r":
        ReadTagAndBlocks(comms)
    elif choice == "E" or choice == "e":
        sys.exit()


