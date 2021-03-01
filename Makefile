all : 

	gcc -o gpsTrack gpsTrack.c openSerial.c readGps.c
	
clean :

	rm -rf gpsTrack
