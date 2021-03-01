#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "readGps.h"

int readGps(int fd, satellite_t* satellite, char print)
{
	int 		nbytes=0;
      	char 		buffer[128];  /* Input buffer */
	int 		i=0,k=0,l=0;
	int 		totalBytes=0;

	char 		c;

	char 		altitude[10];
	char 		*field[70];
	int  		typeCode=0;
	int		numMessage;
	int 		messageNum;
	int 		inView;

	vehicle_t	vehicle[64];
	char		satUsed[64];

	memset(buffer,0,sizeof(buffer));

	while(c!='$')
		read(fd, &c, 1);

	while (1)
	{
		nbytes = read(fd, &c, 1);
		buffer[i++]=c;

		totalBytes += nbytes;
	  	if (c == '\n' || c == 0x0a)
	         	break;

		if(i==sizeof(buffer)){
			i=0; break;
		}
	}
	buffer[totalBytes] = '\0';
/*------------------------------------------------------------------------------------------------------
		Reference: http://aprs.gids.nl/nmea/#gsa
		$GPRMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a*hh

		Minimum GPS Transit Data
		Field# / Description
		1    = UTC of position fix
		2    = Data status (A=valid V=invalid)
		3    = Latitude of fix
		4    = N or S
		5    = Longitude of fix
		6    = E or W
		7    = Speed over ground in knots
		8    = Track made good in degrees True
		9    = UT date
		10   = Magnetic variation degrees (Easterly var. subtracts from true course)
		11   = E or W
		12   = Checksum
------------------------------------------------------------------------------------------------------*/
	if((strncmp(buffer, "GPRMC", 5) == 0)){
		k=parse_nmea_sentence(buffer, field, 20);
		
		memcpy(satellite->date,field[9],6);
		memcpy(satellite->time,field[1],6);
		
		satellite->status=field[2][0];
		satellite->latitude = GpsToDecimal(field[3], field[4]);
		satellite->longitude = GpsToDecimal(field[5], field[6]);

		typeCode=11;	
 	}
/*------------------------------------------------------------------------------------------------------
		Reference: http://aprs.gids.nl/nmea/#gsa
		$GPGGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
		
		GPS Fix Data
		Field# / Description
		1    = UTC of Position
		2    = Latitude
		3    = N or S
		4    = Longitude
		5    = E or W
		6    = GPS quality indicator (0=invalid; 1=GPS fix; 2=Diff. GPS fix)
		7    = Number of satellites in use [not those in view]
		8    = Horizontal dilution of position
		9    = Antenna altitude above/below mean sea level (geoid)
		10   = Meters  (Antenna height unit)
		11   = Geoidal separation (Diff. between WGS-84 earth ellipsoid and
		       mean sea level.  -=geoid is below WGS-84 ellipsoid)
		12   = Meters  (Units of geoidal separation)
		13   = Age in seconds since last update from diff. reference station
		14   = Diff. reference station ID#
		15   = Checksum
------------------------------------------------------------------------------------------------------*/
	if((strncmp(buffer, "GPGGA", 5) == 0)){
		k=parse_nmea_sentence(buffer, field, 20);

		satellite->alt =(atof(field[9]));
		satellite->hdop =(atof(field[8]));
		satellite->fix =(atoi(field[6]));
		satellite->num_sats=(atoi((const char*)field[7]));

		typeCode=3;
 	}
/*------------------------------------------------------------------------------------------------------
		Reference: http://aprs.gids.nl/nmea/#gsa
		$GPGSA,A,3,19,28,14,18,27,22,31,39,,,,,1.7,1.0,1.3*35

		GPS DOP and Active Satellites
		Field# / Description
		1    = Mode:
		       M=Manual, forced to operate in 2D or 3D
		       A=Automatic, 3D/2D
		2    = Mode:
		       1=Fix not available
		       2=2D
		       3=3D
		3-14 = IDs of SVs used in position fix (null for unused fields)
		15   = PDOP
		16   = HDOP
		17   = VDOP
------------------------------------------------------------------------------------------------------*/
	if((strncmp(buffer, "GPGSA", 5) == 0)){
		k=parse_nmea_sentence(buffer, field, 20);

		satellite->pdop=(atof(field[15]));
		satellite->vdop=(atof(field[17]));
		satellite->mode=(atoi(field[2]));


		if(print==3){
			printf("Sats Used: ");


			for(i=3;i<15;i++){
				printf("%2s ",field[i]);
				satellite->satsUsed[i-3]=atoi(field[i]);
			}
			printf("\n");
		}
		typeCode=5;
 	}
/*------------------------------------------------------------------------------------------------------
		Reference: http://aprs.gids.nl/nmea/#gsa
		$GPGSV,1,1,13,02,02,213,,03,-3,000,,11,00,121,,14,13,172,05*67
		
		GPS Satellites in view
		Field# / Description
		1    = Total number of messages of this type in this cycle
		2    = Message number
		3    = Total number of SVs in view
		4    = SV PRN number
		5    = Elevation in degrees, 90 maximum
		6    = Azimuth, degrees from true north, 000 to 359
		7    = SNR, 00-99 dB (null when not tracking)
		8-11 = Information about second SV, same as field 4-7
		12-15= Information about third SV, same as field 4-7
		16-19= Information about fourth SV, same as field 4-7
------------------------------------------------------------------------------------------------------*/
	if((strncmp(buffer, "GPGSV", 5) == 0)){
		k=parse_nmea_sentence(buffer, field, 20);

		numMessage=atoi(field[1]);
		messageNum=atoi(field[2]);
		inView=atoi(field[3]);
		
		if(messageNum*4<inView+1){
			for(i=0;i<4;i++){
				vehicle[i+((messageNum-1)*4)].num=atoi(field[4+4*i]);
				vehicle[i+((messageNum-1)*4)].elevation=atoi(field[5+4*i]);
				vehicle[i+((messageNum-1)*4)].azimuth=atoi(field[6+4*i]);
				vehicle[i+((messageNum-1)*4)].snr=atoi(field[7+4*i]);
			}
		}else{
			for(i=0;i<inView%4;i++){
				vehicle[i+((messageNum-1)*4)].num=atoi(field[4+4*i]);
				vehicle[i+((messageNum-1)*4)].elevation=atoi(field[5+4*i]);
				vehicle[i+((messageNum-1)*4)].azimuth=atoi(field[6+4*i]);
				vehicle[i+((messageNum-1)*4)].snr=atoi(field[7+4*i]);
			}
		}
//DEBUG:
		//if(print>1){
		//	for(i=0;i<k+1;i++)
		//		printf("%s,",field[i]);
			//printf("\n");
		//}

		if(print==3 && (numMessage==messageNum)){

			for(l=0;l<32;l++){
				vehicle[l].inUse='\0';
				for(i=3;i<15;i++){
					if((vehicle[l].num==satellite->satsUsed[i-3])&&(vehicle[l].num!=0)&&(satellite->satsUsed[i-3]!=0))
						vehicle[l].inUse='*';
				}
			}

			printf("\n---------------------------------- in use\n");
			for(i=0;i<inView;i++){
				printf("v[%2d] sat#:%2d ",i,vehicle[i].num);
				printf("elv:%2d ",vehicle[i].elevation);
				printf("azi:%3d ",vehicle[i].azimuth);
				printf("snr:%2d ",vehicle[i].snr);
				printf("  %c\n",vehicle[i].inUse);
			}
			printf("----------------------------------\n\n");
		}
		typeCode=6;
 	}
	return typeCode;
}

int printGPS(satellite_t* satellite, char print){

	printf("Status:%c Lat:%f Lon:%f Alt:%.1f ",satellite->status,satellite->latitude,satellite->longitude,satellite->alt);
	printf("Date:%s Time:%s\n",satellite->date,satellite->time);
}

int printStats(satellite_t* satellite, char print){

	printf("Status:%c ",satellite->status);
	printf("HDOP:%.1f VDOP:%.1f PDOP:%.1f Mode: %d ",satellite->hdop,satellite->vdop,satellite->pdop,satellite->mode);
	printf("# Sats: %d\n",satellite->num_sats);
}

int printAll(satellite_t* satellite, char print){

	printf("Status:%c Lat:%f Lon:%f Alt:%.1f ",satellite->status,satellite->latitude,satellite->longitude,satellite->alt);
	printf("Date:%s Time:%s ",satellite->date,satellite->time);

	printf("HDOP:%.1f VDOP:%.1f PDOP:%.1f Mode: %d ",satellite->hdop,satellite->vdop,satellite->pdop,satellite->mode);
	printf("# Sats: %d\n",satellite->num_sats);
}

int parse_nmea_sentence(char *string, char **fields, int max_fields)
{
	int i = 0;
	fields[i++] = string;

	while ((i < max_fields) && NULL != (string = strchr(string, ','))) {
		*string = '\0';
		fields[i++] = ++string;
	}

	return --i;
}

float GpsToDecimal(char* nmeaPos, char* quadrant)
{
	//Convert NMEA position to decimal deg. "ddmm.mmmm" or "dddmm.mmmm" = D+M/60, (-) if quad is 'W' or 'S'
  	float v= 0;

  	if(strlen(nmeaPos)>5){
    		char integerPart[3+1];
    		int digitCount= (nmeaPos[4]=='.' ? 2 : 3);
    		memcpy(integerPart, nmeaPos, digitCount);
    		integerPart[digitCount]= 0;
    		nmeaPos+= digitCount;
    		v= atoi(integerPart) + atof(nmeaPos)/60.;
    		if(quadrant[0]=='W' || quadrant[0]=='S')
      			v= -v;
  	}
  	return v;
}

int _initSatellite(satellite_t* satellite)
{
	int i=0;

 	satellite->latitude=0;
 	satellite->longitude=0;
 	satellite->alt=0;
 	satellite->fix=0;
 	satellite->status=0;
 	satellite->hdop=0;
 	satellite->vdop=0;
 	satellite->pdop=0;
 	satellite->num_sats=0;
	satellite->mode=0;

	for(i=0;i<7;i++){
		satellite->date[i]=0;
		satellite->time[i]=0;
	}
}

