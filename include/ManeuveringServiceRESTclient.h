#ifndef MSRESTCLIENT_H
#define MSRESTCLIENT_H

// Microsoft C++ REST API includes
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

#include "LDMmap.h" 

// IMPORTANT NOTE: the ManeuveringServiceRestClient objects should always be created with "new" as they include a "self-destroy" mechanism which relies on "delete"
// Creating a ManeuveringServiceRestClient object on the stack, without new, may result in trying to destroy an already destroyed object when the object goes out of scope

class ManeuveringServiceRestClient {
	public:
		// ManeuveringServiceRestClient(double lat, double lon, uint64_t refVehStationID, ldmmap::LDMMap *db_ptr) :
		// 	 m_range_m(m_range_m_default), m_db_ptr(db_ptr), m_lat(lat), m_lon(lon), m_refVehStationID(refVehStationID), m_notify_fcn(nullptr) {m_thread_running=false; m_srv_addr="http://localhost"; m_port=8000;};

		ManeuveringServiceRestClient(uint64_t refVehStationID, ldmmap::LDMMap *db_ptr) :
			m_range_m(m_range_m_default), m_db_ptr(db_ptr), m_refVehStationID(refVehStationID), m_notify_fcn(nullptr) {m_thread_running=false; m_srv_addr="http://localhost"; m_port=8000;};

		ManeuveringServiceRestClient(uint64_t refVehStationID, ldmmap::LDMMap *db_ptr, std::string address, long port) :
			m_range_m(m_range_m_default), m_db_ptr(db_ptr), m_refVehStationID(refVehStationID), m_notify_fcn(nullptr) {m_thread_running=false; m_srv_addr=address; m_port=port;};

		void setNotifyFunction(std::function<void(uint64_t)> notify_fcn) {m_notify_fcn = notify_fcn;}
		void inline callNotifyFunction(void);

		int startRESTthread(void);
		void setPeriodicInterval(double interval_sec) {m_interval_sec = interval_sec;}
		double getPeriodicInterval(void) {return m_interval_sec;}

		// This method is used internally by the thread callback function to signal the REST thread termination
		void setThreadRunningStatus(bool status) {m_thread_running = status;}

		// This method returns 'true' if the REST thread is currently running and transmitting data to the Maneuvering Service,
		// 'false' otherwise
		bool getRunningStatus(void) {return m_thread_running;};

		void setServerAddress(std::string address) {m_srv_addr = address;}
		void setServerPort(long port) {m_port = port;}
		void setServerAddressPort(std::string address, long port) {m_srv_addr = address; m_port = port;}

		void changeContextRange(double range_m) {m_range_m=range_m;}
		double getContextRange(void) {return m_range_m;}

		std::string inline getServerFullAddress(void);

		web::json::value make_SLDM_json(int eventID);
	private:
		web::json::value make_vehicle(uint64_t stationID, 
			double lat, 
			double lon, 
			std::string turnindicator, 
			uint16_t CAM_tstamp,
			uint64_t GN_tstamp,
			ldmmap::OptionalDataItem<long> car_length_mm,
			ldmmap::OptionalDataItem<long> car_width_mm,
			double speed_ms,
			ldmmap::PHpoints *path_history,
			std::string &src_quadk,
			double relative_dist_m,
			ldmmap::e_StationTypeLDM stationType,
			uint64_t diff_ref_tstamp,
			uint64_t diff_rec_tstamp,
			uint64_t cam_rec_tstamp,
			uint64_t db_up_tstamp,
			double heading
			);

		const double m_range_m_default = 100.0;
		double m_range_m;
		ldmmap::LDMMap *m_db_ptr;
		std::string m_srv_addr;
		long m_port;
		double m_interval_sec = 1.0;

		// Not needed, as the rangeSelect() on the database is performed based on the stationID and not on a fixed point
		// double m_lat = 0.0;
		// double m_lon = 0.0;

		uint64_t m_refVehStationID;

		std::atomic<bool> m_thread_running;

		std::function<void(uint64_t)> m_notify_fcn;
};

#endif // MSRESTCLIENT_H
