
/*
 * RFIDReader.c:
 *	Example code on accessing the RFID Reader and reading data from tags
 *
 *
 *
 * The code here is experimental, and is not intended to be used
 * in a production environment. It demonstrates the basics of what is
 * required to get the Raspberry Pi receiving RFID data and configuring
 * the RFID Reader parameters.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation as version 2 of the License.
 *
 * For more informaiton refer to www.cogniot.eu
 * 
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>


#include <wiringPi.h>
#include <wiringSerial.h>


// set for GPIO Pin to use based on the jumper connection
#define GPIO_PIN 1       // Jumper 1, also known as GPIO18
// #define GPIO_PIN 0          // Jumper 2, also known as GPIO17
// #define GPIO_PIN 2       // Jumper 3, also known as GPIO21 (Rv 1) or GPIO27 (Rv 2)
// #define GPIO_PIN 3       // Jumper 4, also known as GPIO22

// Define global variables

int fd;	// File handle for connection to the serial port.


// Helper functions

void WaitForCTS()
{
	// Generic call to wait for the CTS going high
	// CTS is implemented via the use of the GPIO as the UART on the
	// Pi doen't have any control lines.
	serialFlush(fd);

	while (digitalRead(GPIO_PIN) == HIGH)
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
		printf ("%c", serialGetchar (fd)) ;
		fflush (stdout) ;
	}
	printf("\n\n");
}



int GetAntennaStatus()
{
	// Perform a firmware read to check the status of the antenna

	WaitForCTS();
    serialPutchar(fd, 0x53);  // Send any command - this happens to be the one to check if a tag is present
	delay(100);

	if ((serialGetchar(fd) & 0x20) == 0x20 )  // Checking bit 5.  If set, indicates antenna fault
	{
		printf("ERROR : ANTENNA or Eprom fault. Please check the antenna is correctly installed.\n\n");
		return 1;  // return value set to indicate error
	}
	else
	{
		return 0;
	}

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

  pinMode(GPIO_PIN,INPUT);	// We are using GPIO_PIN as the pin to identify the "CTS" function



  if ((fd = serialOpen ("/dev/serial0", 9600)) < 0)  // Try to open a connection to the serial port
  {
   fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    return 1 ;
  }
  else
  {
  	// We have opened communications with the onboard Serial device
	int antennaOK = 0;

	printf("Opened communications with PirFix.\n");  // Communications opened successfully

	antennaOK = GetAntennaStatus();  // Check status of the antenna.

	if (antennaOK ==1)
	{
		return 1; // if there is an antenna fault, exit with a non-zero return code
	}
  }




char option, tagOption;
int noTag;


do {
	printf(" \n\n");
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

				while ( serialDataAvail(fd))  // Whilst data is being sent back from the device
				{
					char result;
					result = serialGetchar(fd);	// Get each character
					if (result == 0xD6)  // confirm that the tag is present, valid and no errors
					{
						noTag = 0;		// set this so the outer while loop can terminate
						printf ("\nTag present.\n\n");
					}
				}
			}
			break;


        case 'F': // Perform a factory reset

			printf("\nPerforming a factory reset ....\n");

			serialPutchar(fd, 0x46);	// this command sequence is
			serialPutchar(fd, 0x55);	//
			serialPutchar(fd, 0xAA);	// required to force a factory reset.

			delay(100);

			printf("\n\nFACTORY RESET COMPLETE \n\n");


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
					printf("\n\nPolling delay changed .......\n\n");
				}
				else
				{
					printf("\n\nUnexpected reply >%X< \n\n", result);
					while (serialDataAvail (fd))
					{
						printf (" %X", serialGetchar (fd)) ;
						fflush (stdout) ;
					}
					printf("\n\n");
				}
			}
			break;




		case 'v': // Select the reader operating mode

			printf("\n\nSetting Reader Operating Tag Mode.......\n\n");

			{
				printf("\n\t*********************************************\n");
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
						serialPutchar(fd, 0x02);  // 0x02 = H1/S
						break;

					case 'c':
						WaitForCTS();

						serialPutchar(fd, 0x76);
						serialPutchar(fd, 0x03);// 0x03 = EM/MC2000
						break;

					default:
						printf("\n\tInvalid option.  Please try again...\n\n");
						// wait until a valid entry has been selected.

				}

				delay(100);
				while ( serialDataAvail(fd) )
				{
					char result;
					result = serialGetchar(fd);
					if (result == 0xC0)
					{
						printf("\n\nReader Operating Tag Mode changed .......\n\n");
					}
					else
					{
						printf("\n\nUnexpected reply >%X< \n\n", result);
						while (serialDataAvail (fd))
						{
							printf (" %X", serialGetchar (fd)) ;
							fflush (stdout) ;
						}
						printf("\n\n");
					}
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

			delay(100); //  Need to wait otherwise the command does not work

			while ( serialDataAvail(fd) )
			{
				char result;
				result = serialGetchar(fd);
				if (result == 0xD6)
				{
					noTag = 0;
					printf ("\nTag present.\n\n");
					while (serialDataAvail (fd))
					{
						printf (" %X", serialGetchar (fd)) ;
						fflush (stdout) ;
					}
					noTag = 0;
					printf("\n\n");
				}
				else
				{
					// do nothing
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
					printf ("\nTag present.\n\n");
					while (serialDataAvail (fd))
					{
						printf (" %X", serialGetchar (fd)) ;
						fflush (stdout) ;
					}
					noTag = 0;
					printf("\n\n");
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

     serialClose(fd);

return(0);

}

