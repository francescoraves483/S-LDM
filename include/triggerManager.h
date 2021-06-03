#ifndef TRIGGERMAN_H
#define TRIGGERMAN_H

#include <mutex>
#include <list>

#include "LDMmap.h"
extern "C" {
	#include "CAM.h"
}

// Simple, indicator-based, trigger manager
class indicatorTriggerManager {
	public:
		indicatorTriggerManager(ldmmap::LDMMap *db_ptr) : m_db_ptr(db_ptr) {}
		
		void setDBpointer(ldmmap::LDMMap *db_ptr) {m_db_ptr = db_ptr;}
		bool checkAndTrigger(double lat, double lon, uint64_t refVehStationID, uint8_t exteriorLightsStatus);
		void notifyOpTermination(uint64_t stationID);

	// "protected" just in case new classes will be derived from this one
	protected:
		std::mutex m_already_triggered_mutex;
		std::list<uint64_t> m_already_triggered; // List of vehicles for which the operations after triggering are in progress

	private:
		ldmmap::LDMMap *m_db_ptr;
};

#endif // TRIGGERMAN_H