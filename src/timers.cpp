#include "timers.h"
#include <math.h>
#include <unistd.h>
#include <sys/timerfd.h>

#include <errno.h>
#include <cstring>
#include <iostream>

#define POLL_DEFINE_JUNK_VARIABLE() long int junk
#define POLL_CLEAR_EVENT(clockFd) junk=read(clockFd,&junk,sizeof(junk))

bool 
Timer::start() {
	struct itimerspec new_value;
	time_t sec;
	long nanosec;

	if(m_time_ms<=0) {
		return false;
	}

	// Create monotonic (increasing) timer
	m_clock_fd=timerfd_create(CLOCK_MONOTONIC,NO_FLAGS_TIMER);
	if(m_clock_fd==-1) {
		return false;
	}

	// Convert time, in ms, to seconds and nanoseconds
	sec=(time_t) ((m_time_ms)/MILLISEC_TO_SEC);
	nanosec=MILLISEC_TO_NANOSEC*m_time_ms-sec*SEC_TO_NANOSEC;
	new_value.it_value.tv_nsec=nanosec;
	new_value.it_value.tv_sec=sec;
	new_value.it_interval.tv_nsec=nanosec;
	new_value.it_interval.tv_sec=sec;

	// Fill pollfd structure
	m_timerMon.fd=m_clock_fd;
	m_timerMon.revents=0;
	m_timerMon.events=POLLIN;

	// Start timer
	if(timerfd_settime(m_clock_fd,NO_FLAGS_TIMER,&new_value,NULL)==-1) {
		close(m_clock_fd);
		return false;
	}

	return true;
}

bool 
Timer::stop() {
	struct itimerspec new_value;

	new_value.it_value.tv_sec=0;
	new_value.it_value.tv_nsec=0;
	new_value.it_interval.tv_nsec=0;
	new_value.it_interval.tv_sec=0;

	// Stop (disarm) timer
	if(timerfd_settime(m_clock_fd,NO_FLAGS_TIMER,&new_value,NULL)==-1) {
		return false;
	}

	return true;
}

bool
Timer::rearm(uint64_t time_ms) {
	struct itimerspec new_value;
	time_t sec;
	long nanosec;

	// Convert time, in ms, to seconds and nanoseconds
	sec=(time_t) ((m_time_ms)/MILLISEC_TO_SEC);
	nanosec=MILLISEC_TO_NANOSEC*m_time_ms-sec*SEC_TO_NANOSEC;
	new_value.it_value.tv_nsec=nanosec;
	new_value.it_value.tv_sec=sec;
	new_value.it_interval.tv_nsec=nanosec;
	new_value.it_interval.tv_sec=sec;

	// Rearm timer with the new value
	if(timerfd_settime(m_clock_fd,NO_FLAGS_TIMER,&new_value,NULL)==-1) {
		close(m_clock_fd);
		return -2;
	}

	return 0;
}

bool
Timer::waitForExpiration() {
	POLL_DEFINE_JUNK_VARIABLE();

	if(poll(&m_timerMon,1,INDEFINITE_WAIT)>0) {
		POLL_CLEAR_EVENT(m_clock_fd);
		return true;
	}

	return false;
}