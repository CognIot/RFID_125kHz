
/*
 * RFIDReader.c:
 *	Example code on accessing the RFID Reader and reading data from tags
 *
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>


#include <wiringPi.h>
#include <wiringSerial.h>

#define GPIO17 0

// Define global variables

int fd;	// File handle for connection to the serial port.


// Helper functions

void WaitForCTS()
{
	// Generic call to wait for the CTS going
	serialFlush(fd);

	while (digitalRead(GPIO17) == HIGH)
	{
		// Do Nothing
		// printf(".");
	}

}

void GetTextResult()
{
	// Generic routine to return text from the serial port.

	while (serialDataAvail (fd))
	{
		printf ("\x1B[32m%c", serialGetchar (fd)) ;
		fflush (stdout) ;
	}
	printf("\x1B[0m\n\n");
}





int main ()
{
//


// Initialise WiringPi so we can use the GPIO on the Raspberry Pi

 if (wiringPiSetup () == -1)
  {
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
    return 1 ;
  }

// The PirFix can use one of the following GPIO pins configured as an input
//
// GPIO17
// GPIO18
// GPIO21
// GPIO22
//

  pinMode(GPIO17,INPUT);



  if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
  {
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    return 1 ;
  }
  else
  {
  	// We have opened communications with the onboard Serial device
	printf("Opened communications with PirFix.\n");

  }




char option, tagOption, pollDelay;
int noTag;


do {
	printf("**************************************************************************\n");
	printf("Available commands: -\n\n");
	printf("z - Display firmware version information\n");
	printf("S - Acknowledge presence of Tag\n");
	printf("F - Perform a Factory Reset \n");
	printf("P - Program EEPROM Polling delay \n");
	printf("v - Select reader operating mode \n");
	printf("R - Read Tag and PAGE 00 data \n");
	printf("r - Read Tag and BLOCK 00-03 data \n");
	printf("e - Exit program \n");
	printf(" \n");

	printf("Please select command -> ");

	option = getchar();
	getchar();  // have to press enter and this consumes the enter character


       switch (option)
       {

            case 'z': // Read the firmware version

		printf("\nRead Firmware Details - Reading device..>\n\n");

		WaitForCTS();

                serialPutchar(fd, 0x7A); // Send 'z' to the PirFix

		delay(100); // ??? Need to wait otherwise the command does not work

		GetTextResult();

		break;


	   case 'S': // Read the status of the RFID device

		noTag = 1;

		printf("\nWaiting for a tag ....\n");

		while (noTag == 1)
		{
			WaitForCTS();

			serialPutchar(fd, 0x53); // Send 'S' to the PirFix

			delay(100); // ??? Need to wait otherwise the command does not work

			while ( serialDataAvail(fd) )
			{
				char result;
				result = serialGetchar(fd);
				if (result == 0xD6)
				{
					noTag = 0;
					printf ("\n\x1B[32mTag present.\x1B[0m\n\n");
				}
			}
		}
		break;


            case 'F': // Perform a factory reset

		printf("\nPerforming a factory reset ....\n");

		serialPutchar(fd, 0x46);
		serialPutchar(fd, 0x55);
		serialPutchar(fd, 0xAA);

		delay(100);

		printf("\n\n\x1B[32mFACTORY RESET COMPLETE \x1B[0m\n\n");


		break;


	   case 'P': // Program the internal EEPROM Polling delay

		printf("\n\nSetting Polling delay .......\n\n");

		WaitForCTS();

		serialPutchar(fd, 0x50);
		serialPutchar(fd, 0x00);
		//serialPutchar(fd, 0x00); // 0x00 is no delay
		//serialPutchar(fd, 0x20); // 0x20 is approx 20ms
		//serialPutchar(fd, 0x40); // 0x40 is approx 65ms
		serialPutchar(fd, 0x60); // 0x60 is approx 262ms
		//serialPutchar(fd, 0x80); // 0x60 is approx 1 Seconds
		//serialPutchar(fd, 0xA0); // 0x60 is approx 4 Seconds

		delay(100);

		while ( serialDataAvail(fd) )
		{
			char result;
			result = serialGetchar(fd);
			if (result == 0xC0)
			{
				printf("\n\n\x1B[32mPolling delay changed .......\x1B[0m\n\n");
			}
			else
			{
				printf("\n\n\x1B[32mUnexpected reply >%X< \x1B[0m\n\n", result);
				while (serialDataAvail (fd))
				{
					printf ("\x1B[32m %X", serialGetchar (fd)) ;
					fflush (stdout) ;
				}
				printf("\x1B[0m\n\n");
			}
		}
		break;




            case 'v': // Select the reader operating mode

			printf("\n\nSetting Reader Operating Tag Modde.......\n\n");

			{
				printf("\t*********************************************\n");
				printf("\ta - Hitag H2\n");
				printf("\tb - Hitag H1/S (factory default)\n");
				printf("\tc - EM/MC2000\n\n");

				printf("\tPlease select tag type .....>");

				tagOption = getchar();
				getchar(); // Needed to consume the carriage return.

				switch (tagOption)
				{
					case 'a':
						WaitForCTS();

						serialPutchar(fd, 0x76);
						serialPutchar(fd, 0x01);  // 0x01 = H2
						break;


					case 'b':
						WaitForCTS();

						serialPutchar(fd, 0x76);
						serialPutchar(fd, 0x02);  // 0x02 = H2
						break;

					case 'c':
						WaitForCTS();

						serialPutchar(fd, 0x76);
						serialPutchar(fd, 0x03);// 0x03 = EM/MC2000
						break;

					default:
						// wait until a valid entry has been selected.

				}

				delay(100);
				while ( serialDataAvail(fd) )
				{
					char result;
					result = serialGetchar(fd);
					if (result == 0xC0)
					{
						printf("\n\n\x1B[32mReader Operating Tag Mode changed .......\x1B[0m\n\n");
					}
					else
					{
						printf("\n\n\x1B[32mUnexpected reply >%X< \x1B[0m\n\n", result);
						while (serialDataAvail (fd))
						{
							printf ("\x1B[32m %X", serialGetchar (fd)) ;
							fflush (stdout) ;
						}
						printf("\x1B[0m\n\n");
					}
				}





		break;


	   case 'R': // Read a page of data from the Tag

		noTag = 1;

		printf("\n\nReading Tag Data Page 00.......\n\n");

		printf("\nWaiting for a tag ....\n");

		while (noTag == 1)
		{
			WaitForCTS();

	                serialPutchar(fd, 0x52);
 			serialPutchar(fd, 0x00); // Tag Page 00 - both H1/S and H2 should work here

			delay(100); // ??? Need to wait otherwise the command does not work

			while ( serialDataAvail(fd) )
			{
				char result;
				result = serialGetchar(fd);
				if (result == 0xD6)
				{
					noTag = 0;
					printf ("\n\x1B[32mTag present.\x1B[0m\n\n");
					while (serialDataAvail (fd))
					{
						printf ("\x1B[32m %X", serialGetchar (fd)) ;
						fflush (stdout) ;
					}
					noTag = 0;
					printf("\x1B[0m\n\n");
				}
				else
				{
				}
			}
		}

 		break;

 	   case 'r': // Read data tag block data

		noTag = 1;

		printf("\n\nReading Tag Data Block 00, 01, 02 & 03.......\n\n");

		printf("\nWaiting for a tag ....\n");

		while (noTag == 1)
		{
			WaitForCTS();

	                serialPutchar(fd, 0x74);
 			serialPutchar(fd, 0x04); // Tag blocks 00-003 - only H1/S should work here

			delay(100); // ??? Need to wait otherwise the command does not work

			while ( serialDataAvail(fd) )
			{
				char result;
				result = serialGetchar(fd);
				if (result == 0xD6)
				{
					noTag = 0;
					printf ("\n\x1B[32mTag present.\x1B[0m\n\n");
					while (serialDataAvail (fd))
					{
						printf ("\x1B[32m %X", serialGetchar (fd)) ;
						fflush (stdout) ;
					}
					noTag = 0;
					printf("\x1B[0m\n\n");
				}
				else
				{
				}
			}
		}
		break;


	    case 'e':
	    	printf("Exiting.......\n");
		option = 'e';
		break;

            default:
	    	printf("Unrecognised command!\n");


       }

       fflush (stdout) ;

     } while(option != 'e');

return(0);

}

