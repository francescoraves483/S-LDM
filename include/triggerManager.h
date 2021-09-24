#ifndef TRIGGERMAN_H
#define TRIGGERMAN_H

#include <mutex>
#include <list>

#include "LDMmap.h"
extern "C" {
	#include "CAM.h"
	#include "options.h"
}

// Simple, indicator-based, trigger manager
class indicatorTriggerManager {
	public:
		indicatorTriggerManager(ldmmap::LDMMap *db_ptr, options_t *opts_ptr) : m_db_ptr(db_ptr), m_opts_ptr(opts_ptr) {m_left_indicator_enabled=false;}
		
		void setDBpointer(ldmmap::LDMMap *db_ptr) {m_db_ptr = db_ptr;}
		bool checkAndTrigger(double lat, double lon, uint64_t refVehStationID, uint8_t exteriorLightsStatus);
		void notifyOpTermination(uint64_t stationID);

		// If set to true, the REST client will consider as triggering condition also the left indicator, other then the right one (default behaviour)
		void setLeftTurnIndicatorEnable(bool m_enable) {m_left_indicator_enabled=m_enable;}

	// "protected" just in case new classes will be derived from this one
	protected:
		std::mutex m_already_triggered_mutex;
		std::list<uint64_t> m_already_triggered; // List of vehicles for which the operations after triggering are in progress

	private:
		ldmmap::LDMMap *m_db_ptr;
		options_t *m_opts_ptr;

		bool m_left_indicator_enabled;
};

#endif // TRIGGERMAN_H