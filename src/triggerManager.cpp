#include "triggerManager.h"
#include "ManeuveringServiceRESTclient.h"

bool indicatorTriggerManager::checkAndTrigger(double lat, double lon, uint64_t refVehStationID, uint8_t exteriorLightsStatus) {
	bool retval = false;

	if(!m_db_ptr || !m_opts_ptr) {
		return false;
	}

	// 5 = 7 - 2 is used to check bit 2, i.e. if "leftTurnSignalOn" is set
	// 4 = 7 - 3 is used to check bit 3, i.e. if "rightTurnSignalOn" is set
	// The default logic is to always trigger when a right turn indicator is set
	// Instead, we should trigger the transmission of REST data when a left turn indicator is set only if the m_left_indicator_enabled flag is set to true
	// This flag is 'false' by default and it can be set to 'true' with setLeftTurnIndicatorEnable()
	if(exteriorLightsStatus & (1 << 4) || (m_left_indicator_enabled==true && exteriorLightsStatus & (1 << 5))) {
	// if(exteriorLightsStatus & (1 << 4)) {
		// Avoid triggering multiple times for the same vehicle
		m_already_triggered_mutex.lock();
		if(std::find(m_already_triggered.begin(), m_already_triggered.end(), refVehStationID) == m_already_triggered.end()) {
			// This constructor for the Maneuvering Service REST client is no more available as the database should be read
			// with a given stationID as reference (i.e., generating the context around a moving vehicle), and not considering
			// a fixed (lat,lon) central point
			// ManeuveringServiceRestClient *ms_restclient = new(std::nothrow) ManeuveringServiceRestClient(lat,lon,refVehStationID,m_db_ptr);

			std::cout << "[INDICATOR TRIGGER MANAGER] Contacting the REST server at: " << options_string_pop(m_opts_ptr->ms_rest_addr) << ":" << m_opts_ptr->ms_rest_port << std::endl;
			
			ManeuveringServiceRestClient *ms_restclient = 
				new(std::nothrow) ManeuveringServiceRestClient(refVehStationID,m_db_ptr,std::string(options_string_pop(m_opts_ptr->ms_rest_addr)),m_opts_ptr->ms_rest_port);

			if(ms_restclient!=nullptr) {
				ms_restclient->changeContextRange(m_opts_ptr->context_radius);
				ms_restclient->setPeriodicInterval(m_opts_ptr->ms_rest_periodicity);
				
				ms_restclient->setNotifyFunction(std::bind(&indicatorTriggerManager::notifyOpTermination,this,std::placeholders::_1));
				
				m_already_triggered.push_back(refVehStationID);
				
				ms_restclient->startRESTthread();
				
				retval = true;
			} else {
				std::cerr << "[ERROR] Cannot create a new REST client towards the Maneuvering Service. bad_alloc error." << std::endl;
				retval = false;
			}
		}
		m_already_triggered_mutex.unlock();
	}

	return retval;
};

void indicatorTriggerManager::notifyOpTermination(uint64_t stationID) {
	m_already_triggered_mutex.lock();
	m_already_triggered.remove(stationID);
	m_already_triggered_mutex.unlock();
}