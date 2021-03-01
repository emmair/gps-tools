#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <getopt.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/poll.h>
#include <sys/time.h>

#include "openSerial.h"
#include "readGps.h"

void showHelp(){

	printf("usage: gpsTrack -g <gps device> [-t <dwell time>] [-s <radio device>] [-p <1-4>]\n");
	printf("\nWhere: \n\n<dwell time> is the time (in seconds) between position update recording.\n");
	printf("<gps device> is the serial device (i.e. /dev/ttySx) of the gps [default=/dev/ttyUSB0]\n");
	printf("Optional: <radio device> is the serial device of the radio [default=NULL]\n");
	printf("-p prints to stdout: 1=GPS only, 2=Stats Only, 3=Vehicle Info Only, 4=Everything (Except vehicle info)\n");

	exit(1);
}
int main(int argc, char *argv[])
{
	int		i=0, l=0;
	char		print=1;
	char*		gpsDevice /*=	"/dev/ttyUSB0"*/;
	char*		telemRadio=	NULL;

	struct		tm timer;
	int		time1=0;
	int		dwellTime=1;	//dwell time in seconds

	int		sd,fd,pd;
    	unsigned char 	buffer[19];
	char		_date[24]={'\n'};
	char		status=0,fix=0;

	float		latitude=0, longitude=0;
	char		UTC[24]={'0','0','0','0','0','0'};
	double		alt=0;
	unsigned int	count=0;
	float		hdop=0;
	int		num_sats=0;

	struct		pollfd pfd[3];		//Poll Device
 
	int		typeCode=0;
	int		event=0;
	satellite_t	satellite;

	_initSatellite(&satellite);

	if(argc < 2){
		printf("usage: %s  -g <gps device> [-t <dwell time>] [-s <radio device>] [-p <1-4>]\n",argv[0]);
		exit(1);
	}
	else
	{
                for (i=1; i < (argc); i++)
                {
                        if (((strncmp (argv[i], "-t",2))==0)&&(argv[i+1]!=NULL)) {dwellTime=atoi(argv[i+1]); }
                        if (((strncmp (argv[i], "-p",2))==0)&&(argv[i+1]!=NULL)) {print=atoi(argv[i+1]); } 
			if (((strncmp (argv[i], "-g",2))==0)&&(argv[i+1]!=NULL)) {gpsDevice=argv[i+1];} 
			if (((strncmp (argv[i], "-s",2))==0)&&(argv[i+1]!=NULL)) {telemRadio=argv[i+1];} 
			if ((strncmp (argv[i], "-h",2))==0) {showHelp();}
                }	
		if(gpsDevice==NULL){
			printf("Warning: GPS Source not specified! Check -g argument. Using default /dev/ttyUSB0\n");
			gpsDevice = "/dev/ttyUSB0";
		}
		if(telemRadio==NULL){
			printf("Warning: Radio Source not specified! Check -s arguments.\n");
		}
	}

	i=0;

	gettimeofday((struct timeval *) &timer,NULL);     		//get the time
	time1=timer.tm_sec;

	//Open Serial Devices*****************************************************************************
	if(telemRadio != NULL){
		printf("Telemetry radio: ");
		sd=openSerial(sd,telemRadio,57600);  		//Radio @ 57600 baud 8N1
	}

	if(gpsDevice != NULL){
		printf("GPS: ");
		fd=openSerial(fd,gpsDevice,4800); 		//GPS device @ 9600 baud
	}

	//Set Up polling ***********************************************************************
	if(gpsDevice != NULL){
		pfd[0].fd = fd; 			
		pfd[0].events = POLLIN;				//poll gps device
	}
	if(telemRadio != NULL){
		pfd[1].fd = sd;
		pfd[1].events = POLLIN;				//poll telemetry radio
	}

	// Start Collection ***************************************************************************
	printf("starting ...\n");

	while(1){					// main capture loop
		
		gettimeofday((struct timeval *) &timer,NULL);     		// get the time
	
		if(((timer.tm_sec-time1)>dwellTime-1) && typeCode != 0 ) 	// write to file every dwellTime
		{								// need to subtract 1s for some reason this makes the
										// timing correct... need to check this.
			time1=timer.tm_sec;							
			if(print==1)
				printGPS(&satellite, print);
			else if(print==2)
				printStats(&satellite, print);
			else if(print==4)
				printAll(&satellite, print);

		}
		
		event=poll(pfd,3,0);
		if(pfd[1].revents){		//read the telemetry Radio
			//Telem code goes here...
		}
		
		if(pfd[0].revents)		//Read GPS
			typeCode=readGps(fd, &satellite, print);
		usleep(5000);
	}
}
