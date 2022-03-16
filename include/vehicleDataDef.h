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

	typedef enum StationTypeLDM {
		StationType_LDM_unknown = 0,
		StationType_LDM_pedestrian = 1,
		StationType_LDM_cyclist	= 2,
		StationType_LDM_moped = 3,
		StationType_LDM_motorcycle = 4,
		StationType_LDM_passengerCar = 5,
		StationType_LDM_bus	= 6,
		StationType_LDM_lightTruck = 7,
		StationType_LDM_heavyTruck = 8,
		StationType_LDM_trailer = 9,
		StationType_LDM_specialVehicles	= 10,
		StationType_LDM_tram = 11,
		StationType_LDM_roadSideUnit = 15,
		StationType_LDM_specificCategoryVehicle1 = 100,
		StationType_LDM_specificCategoryVehicle2 = 101,
		StationType_LDM_specificCategoryVehicle3 = 102,
		StationType_LDM_specificCategoryVehicle4 = 103,
		StationType_LDM_specificCategoryVehicle5 = 104,
		StationType_LDM_specificCategoryVehicle6 = 105,
		StationType_LDM_specificCategoryVehicle7 = 106,
		StationType_LDM_specificCategoryVehicle8 = 107,
		StationType_LDM_specificCategoryVehicle9 = 108,
		StationType_LDM_detectedPedestrian = 110,
		StationType_LDM_detectedPassengerCar = 115,
		StationType_LDM_detectedTruck = 117,

		StationType_LDM_unspecified= 120
	} e_StationTypeLDM;

	// This structure contains all the data stored in the database for each vehicle (except for the PHPoints)
	typedef struct vehicleData {
		uint64_t stationID;
		double lat;
		double lon;
		double elevation;
		double heading; // Heading between 0 and 360 degrees
		double speed_ms;
		uint64_t gnTimestamp;
		long camTimestamp; // This is the CAM message GenerationDeltaTime
		uint64_t timestamp_us;
		uint64_t on_msg_timestamp_us;
		OptionalDataItem<long> vehicleWidth;
		OptionalDataItem<long> vehicleLength;
		e_StationTypeLDM stationType;

		std::string sourceQuadkey;

		// Low frequency container data
		OptionalDataItem<uint8_t> exteriorLights; // Bit string with exterior lights status
	} vehicleData_t;
}

#endif // VEHDATADEF_H
