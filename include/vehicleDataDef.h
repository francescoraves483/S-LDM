#ifndef VEHDATADEF_H
#define VEHDATADEF_H

#include <unordered_map>
#include <vector>
#include <shared_mutex>

#define vehicleDataVector_t(name) std::vector<ldmmap::vehicleData_t> name;

// Facility macro to convert from DEG to RAD
#define DEG_2_RAD(val) ((val)*M_PI/180.0)

namespace ldmmap {
	// This structure contains all the data stored in the database for each vehicle (except for the PHPoints)
	typedef struct vehicleData {
		uint64_t stationID;
		double lat;
		double lon;
		double elevation;
		double heading; // Angles must be specified between 180 and -180 degrees
		double speed_ms;
		uint64_t gnTimestamp;
		uint64_t timestamp_us;
	} vehicleData_t;
}

#endif // VEHDATADEF_H