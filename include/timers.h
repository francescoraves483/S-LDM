#ifndef TIMERS_H_INCLUDED
#define TIMERS_H_INCLUDED

#include <inttypes.h>
#include <poll.h>

#define NO_FLAGS_TIMER 0
#define INDEFINITE_WAIT -1

// Time conversion constants
#define SEC_TO_NANOSEC 1000000000
#define MILLISEC_TO_NANOSEC 1000000
#define SEC_TO_MICROSEC 1000000
#define SEC_TO_MILLISEC 1000
#define MILLISEC_TO_SEC 1000
#define MILLISEC_TO_MICROSEC 1000
#define MICROSEC_TO_NANOSEC 1000
#define MICROSEC_TO_MILLISEC 1000

class Timer {
	public:
		Timer(uint64_t time_ms):
			m_time_ms(time_ms) {};

		bool start();
		bool stop();
		bool rearm(uint64_t time_ms);
		bool waitForExpiration();
	private:
		struct pollfd m_timerMon;
		int m_clock_fd;
		uint64_t m_time_ms;
};



#endif // TIMERS_H_INCLUDED