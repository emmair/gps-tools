#ifndef READGPS_H
#define READGPS_H

typedef struct {
 	float 	latitude;
 	float 	longitude;
 	double 	alt;
 	char 	fix;
 	char 	date[7];
 	char 	time[7];
 	char 	status;
 	float 	hdop;
 	float 	vdop;
 	float 	pdop;
 	int 	num_sats;
	char 	mode;
	char	satsUsed[14];
}satellite_t;

typedef struct {
	int	num;
	int	elevation;
	int	azimuth;
	int	snr;
	char	inUse;
}vehicle_t;

int _initSatellite(satellite_t* satellite);

int readGps(int fd, satellite_t* satellite, char print);
float GpsToDecimal(char* nmeaPos, char* quadrant);
int parse_nmea_sentence(char *string, char **fields, int max_fields);

int printGPS(satellite_t* satellite, char print);
int printStats(satellite_t* satellite, char print);
int printAll(satellite_t* satellite, char print);

#endif



