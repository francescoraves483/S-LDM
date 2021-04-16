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

	    	int getCardinality() {return m_card;};

		private:
			std::unordered_map<uint32_t,std::pair<std::shared_mutex*,std::unordered_map<uint32_t,returnedVehicleData_t>>> m_ldmmap;
			uint64_t m_card;
			std::shared_mutex m_mainmapmut;
	};
}

#endif // LDMMAP_H