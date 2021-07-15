#ifndef VEHDATADEF_H
#define VEHDATADEF_H

#include <unordered_map>
#include <vector>
#include <shared_mutex>

#define vehicleDataVector_t(name) std::vector<ldmmap::vehicleData_t> name;

// Facility macro to convert from DEG to RAD
#define DEG_2_RAD(val) ((val)*M_PI/180.0)

namespace ldmmap {
	// Class to store optional data
	// If the data is not available, m_available is 'false' and no actual data is stored (getData() does not return any meaningful data)
	// If the data is available (isAvailable() returns 'true'), then the actual data can be retrieved with getData()
	template <class T> class OptionalDataItem
	{
		private:
			bool m_available;
			T m_dataitem;

		public:
			OptionalDataItem(T data): m_dataitem(data) {m_available=true;}
			OptionalDataItem(bool availability) {m_available=availability;}
			OptionalDataItem() {m_available=false;}
			T getData() {return m_dataitem;}
			bool isAvailable() {return m_available;}
			T setData(T data) {m_dataitem=data; m_available=true;}
	};

	// This structure contains all the data stored in the database for each vehicle (except for the PHPoints)
	typedef struct vehicleData {
		uint64_t stationID;
		double lat;
		double lon;
		double elevation;
		double heading; // Angles must be specified between 180 and -180 degrees
		double speed_ms;
		uint64_t gnTimestamp;
		long camTimestamp; // This is the CAM message GenerationDeltaTime
		uint64_t timestamp_us;
		OptionalDataItem<long> vehicleWidth;
		OptionalDataItem<long> vehicleLength;

		std::string sourceQuadkey;

		// Low frequency container data
		OptionalDataItem<uint8_t> exteriorLights; // Bit string with exterior lights status
	} vehicleData_t;
}

#endif // VEHDATADEF_H