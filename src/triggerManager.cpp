#include "triggerManager.h"
#include "ManeuveringServiceRESTclient.h"

bool indicatorTriggerManager::checkAndTrigger(double lat, double lon, uint64_t refVehStationID, uint8_t exteriorLightsStatus) {
	bool retval = false;

	if(!m_db_ptr) {
		return false;
	}

	// 5 = 7 - 2 is used to check bit 2, i.e. if "leftTurnSignalOn" is set
	// 4 = 7 - 3 is used to check bit 3, i.e. if "rightTurnSignalOn" is set
	// if(exteriorLightsStatus & (1 << 5) || exteriorLightsStatus & (1 << 4)) {
	if(exteriorLightsStatus & (1 << 4)) {
		// Avoid triggering multiple times for the same vehicle
		if(std::find(m_already_triggered.begin(), m_already_triggered.end(), refVehStationID) == m_already_triggered.end()) {
			ManeuveringServiceRestClient *ms_restclient = new(std::nothrow) ManeuveringServiceRestClient(lat,lon,refVehStationID,m_db_ptr);

			if(ms_restclient!=nullptr) {
				ms_restclient->setNotifyFunction(std::bind(&indicatorTriggerManager::notifyOpTermination,this,std::placeholders::_1));

				m_already_triggered_mutex.lock();
				m_already_triggered.push_back(refVehStationID);
				m_already_triggered_mutex.unlock();

				ms_restclient->startRESTthread();
				
				retval = true;
			} else {
				std::cerr << "[ERROR] Cannot create a new REST client towards the Maneuvering Service. bad_alloc error." << std::endl;
				retval = false;
			}
		}
	}

	return retval;
};

void indicatorTriggerManager::notifyOpTermination(uint64_t stationID) {
	m_already_triggered_mutex.lock();
	m_already_triggered.remove(stationID);
	m_already_triggered_mutex.unlock();
}