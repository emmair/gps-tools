#include <getopt.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "openSerial.h"

int openSerial(int fd, char* Device, int baud) 
{     	struct termios options;
	speed_t speed;

	switch (baud)
	{
		case 1200:speed=B1200; break;
		case 4800:speed=B4800; break;
		case 9600:speed=B9600; break;
		case 19200:speed=B19200; break;
		case 38400:speed=B38400; break;
		case 57600:speed=B57600; break;
		case 115200:speed=B115200; break;
		case 230400:speed=B230400; break;
	}	

	printf("Opening port %s... ",Device);
	
      	fd = open(Device, O_RDWR );
      	if (fd == -1){
		printf("\nopen_port: Unable to open %s:",Device);
		exit(0);
	}
      	else
//DEBUG:
		//fcntl(fd, F_SETFL,O_NONBLOCK);
		fcntl(fd, F_SETFL, 0);

    	if((tcgetattr(fd, &options))<0)
		printf("Failed to get serial options! ");

    	if((cfsetispeed(&options, speed))<0)
		printf("Could not set input speed to %d! ",baud);

    	if((cfsetospeed(&options, speed))<0)
		printf("Could not set output speed to %d! ",baud);

    	options.c_cc[ VTIME ] = 0; // timeout in tenths of a second
 
    	tcsetattr(fd, TCSANOW, &options); /* set the options */
	tcflush(fd, TCIFLUSH);

	printf("done.\n");
	return fd;
}
