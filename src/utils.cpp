#include "utils.h"
#include <unistd.h>

uint64_t get_timestamp_us(void) {
	time_t seconds;
	uint64_t microseconds;
	struct timespec now;

	if(clock_gettime(CLOCK_REALTIME, &now) == -1) {
		perror("Cannot get the current timestamp to be inserted inside the JSON file");
		return -1;
	}

	seconds=now.tv_sec;
	microseconds=round(now.tv_nsec/1e3);

	// milliseconds, due to the rounding operation, shall not exceed 999999
	if(microseconds > 999999) {
		seconds++;
		microseconds=0;
	}

	return seconds*1000000+microseconds;
}

int timer_fd_create(struct pollfd &pollfd,int &clockFd,uint64_t time_us) {
	struct itimerspec new_value;
	time_t sec;
	long nanosec;

	// Create monotonic (increasing) timer
	clockFd=timerfd_create(CLOCK_MONOTONIC,0);
	if(clockFd==-1) {
		return -1;
	}

	// Convert time, in us, to seconds and nanoseconds
	sec=(time_t) ((time_us)/1000000);
	nanosec=1000*time_us-sec*1000000000;
	new_value.it_value.tv_nsec=nanosec;
	new_value.it_value.tv_sec=sec;
	new_value.it_interval.tv_nsec=nanosec;
	new_value.it_interval.tv_sec=sec;

	// Fill pollfd structure
	pollfd.fd=clockFd;
	pollfd.revents=0;
	pollfd.events=POLLIN;

	// Start timer
	if(timerfd_settime(clockFd,0,&new_value,NULL)==-1) {
		close(clockFd);
		return -2;
	}

	return 0;
}

std::string exteriorLights_bit_to_string(uint8_t extLights) {
	std::string extLightsStr="";
	const char *bitnames[]={"lowBeamHeadlightsOn", "highBeamHeadlightsOn", "leftTurnSignalOn", 
							"rightTurnSignalOn", "daytimeRunningLightsOn", "reverseLightOn", 
							"fogLightOn", "parkingLightsOn"};

	for(int i=0;i<8;i++) {
		if(extLights & (1 << (7 - i))) {
			if(extLightsStr.length()!=0) {
				extLightsStr += ",";
			}
			extLightsStr += bitnames[i];
		}
	}

	if(extLightsStr=="") {
		extLightsStr="off";
	}

	return extLightsStr;
}