#ifndef LDMMAP_H
#define LDMMAP_H

#include <unordered_map>
#include <vector>
#include <shared_mutex>
#include "PHpoints.h"
#include "vehicleDataDef.h"

namespace ldmmap {
	class LDMMap {
		public:
	    	typedef enum {
	    		LDMMAP_OK,
	    		LDMMAP_UPDATED,
	    		LDMMAP_ITEM_NOT_FOUND,
	    		LDMMAP_MAP_FULL,
	    		LDMMAP_UNKNOWN_ERROR
	    	} LDMMap_error_t;

	    	typedef struct {
	    		vehicleData_t vehData;
	    		PHpoints *phData;
	    	} returnedVehicleData_t;

	    	LDMMap();
	    	~LDMMap();

	    	void clear();
	    	LDMMap_error_t insert(vehicleData_t newVehicleData);
	    	LDMMap_error_t remove(uint64_t stationID);
	    	LDMMap_error_t lookup(uint64_t stationID, returnedVehicleData_t &retVehicleData);
	    	LDMMap_error_t rangeSelect(double range_m, double lat, double lon, std::vector<returnedVehicleData_t> &selectedVehicles);
	    	// This function deletes from the database all the entries older than time_milliseconds ms
	    	// The entries are deleted if their age is > time_milliseconds ms if greater_equal == false, 
	    	// or >= time_milliseconds ms if greater_equal == true
	    	// This function performs a full database read operation
	    	void deleteOlderThan(double time_milliseconds);
	    	void printAllContents(std::string label = "");
	    	void executeOnAllContents(void (*oper_fcn)(vehicleData_t,void *),void *additional_args);

	    	// This setter sets two facility (private) variables which are used solely for passing a central 
	    	// latitude and longitude to the vehicle visualizer, extracting them from an LDMMap object
	    	// The same applies to the corresponding getter method
	    	void setCentralLatLon(double lat, double lon) {m_central_lat = lat; m_central_lon = lon;}
	    	std::pair<double,double> getCentralLatLon() {return std::make_pair(m_central_lat,m_central_lon);}

	    	int getCardinality() {return m_card;};

		private:
			std::unordered_map<uint32_t,std::pair<std::shared_mutex*,std::unordered_map<uint32_t,returnedVehicleData_t>>> m_ldmmap;
			uint64_t m_card;
			std::shared_mutex m_mainmapmut;
			double m_central_lat;
			double m_central_lon;
	};
}

#endif // LDMMAP_H