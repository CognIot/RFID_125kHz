#/usr/bin/env python


import wiringpi2
import time
import sys

# set for GPIO PIn zero,
GPIO_PIN = 0

def WaitForCTS():
    while wiringpi2.digitalRead(GPIO_PIN):
		# do nothing
		time.sleep(0.001)

def ReadText(fd):
	qtydata = wiringpi2.serialDataAvail(fd)
	print "Amount of data: %s bytes" % qtydata
	response = ""
	while qtydata > 0:
		#	print "Reading data back %d" % QtyData
			response = response + chr(wiringpi2.serialGetchar(fd))
			qtydata = qtydata - 1	
	return response

def ReadInt(fd):
    qtydata = wiringpi2.serialDataAvail(fd)
    print "Amount of data: %s bytes" % qtydata
    response = 0
    if qtydata > 0:
        # print "Reading data back %d" % QtyData
        response = wiringpi2.serialGetchar(fd)

    return response

def RFIDSetup():
    response = wiringpi2.wiringPiSetup()
    
    wiringpi2.pinMode(GPIO_PIN, 0)

    fd = wiringpi2.serialOpen('/dev/ttyAMA0', 9600)

    # clear the buffer
    wiringpi2.serialFlush(fd)
    
    if response == 0:
        print "PI setup complete "
    else:
        print "Unable to Setup communications"
        sys.exit()
        
    return fd

def ReadVersion(fd):
    WaitForCTS()
    print "Sending Read Status Command"
    wiringpi2.serialPuts(fd,"z")
    time.sleep(0.1)
    ans = ReadText(fd)
    print "Response: %s" % ans

def ReadTagStatus(fd):
    notag = True
    while notag:
        WaitForCTS()
        print "Sending Tag Status Command"
        wiringpi2.serialPuts(fd,"S")
        time.sleep(0.1)
        ans = ReadInt(fd)
        print "Tag Status: %s" % hex(ans)
        if ans == int("0xD6", 16):
            # Tag present and read
            notag = False


# main code loop

print "Bostin Technology Ltd"
print "Cogniot Products"
print "PirFlx"
print ""
print "Press h for help"
print ""

while True:
	choice = raw_input ("Select Menu Option:")

	comms = RFIDSetup()

	if choice == "H" or choice == "h":
		print "help, you've got to be kidding"
	elif choice == "z":
		ReadVersion(comms)
	elif choice == "S":
		ReadTagStatus(comms)
	elif choice == "X" or choice == "x":
		sys.exit()


