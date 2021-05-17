//
// Created by carlos on 10/05/21.
//

#ifndef S_LDM_UTILS_H
#define S_LDM_UTILS_H


#include <cstdint>

namespace etsiDecoder {
	typedef enum {
		GN_OK = 0,
		GN_VERSION_ERROR = 1,
		GN_SECURED_ERROR = 2,
		GN_LIFETIME_ERROR = 3,
		GN_HOP_LIMIT_ERROR = 4,
		GN_TYPE_ERROR = 5,
	} gnError_e;

	typedef enum {
		BTP_OK = 0,
		BTP_ERROR = 1
	} btpError_e;

	typedef struct longPositionVector{
	    char GnAddress[8]; //! Address
	    uint32_t TST; //! TimeSTamp at which lat and long were acquired by GeoAdhoc router
	    int32_t latitude;
	    int32_t longitude;
	    bool positionAccuracy : 1;
	    int16_t speed :15;
	    uint16_t heading;
	} GNlpv_t;

	typedef enum {
	    ANY_TT=0,
	    BEACON=1,
	    GUC=2,
	    GAC=3,
	    GBC=4,
	    TSB=5,
	    LS=6,
	} TransportType_t;

	typedef enum {
	    UNSPECIFIED=0,
	    ITS_G5=1
	} CommProfile_t;

	typedef enum {
	    ANY_UP=0,
	    BTP_A=1,
	    BTP_B=2,
	    GN6ASL=3
	} BTPType_t;

	typedef struct _geoarea {
	    int32_t posLong;
	    int32_t posLat;
	    uint16_t distA;
	    uint16_t distB;
	    uint16_t angle;
	    uint8_t shape;
	} GeoArea_t;


	typedef enum {
	    ACCEPTED=0,
	    MAX_LENGHT_EXCEEDED = 1,
	    MAX_LIFE_EXCEEDED = 2,
	    REP_INTERVAL_LOW = 3,
	    UNSUPPORTED_TRA_CLASS = 4,
	    MAX_GEOAREA_EXCEEDED = 5,
	    UNSPECIFIED_ERROR =6
	} GNDataConfirm_t;

	typedef struct dataIndication {
	    uint8_t BTPType;
	    uint16_t destPort;
	    uint16_t sourcePort;
	    uint16_t destPInfo;
	    uint8_t GNType; // GN Packet transport type -- GeoUnicast, SHB, TSB, GeoBroadcast or GeoAnycast
	    GeoArea_t GnAddress; // GN destination adress -- destination adress(GeoUnicast) or geo. area (GeoBroadcast or GeoAnycast)
	    GNlpv_t GNPositionV; // GN Posistion vector

	    uint16_t GNSecurityR; // GN Security Report /OPCIONAL/
	    uint16_t GNCertID; //GN Certificate ID /OPCIONAL/
	    uint16_t GNPermissions; // GN Permissions /OPCIONAL/
	    uint16_t GNMaxRepInt; // GN maximum repetition Interval /OPCIONAL/
	    uint8_t GNTraClass; // GN Traffic Class
	    double GNRemPLife; // GN Reamianing Packet Lifetime /OPCIONAL/
	    uint32_t lenght; // Payload size
	    unsigned char* data;
	} BTPDataIndication_t;

	typedef struct gndataIndication {
	    uint8_t upperProtocol;
	    GNlpv_t SourcePV;
	    GeoArea_t GnAddressDest; // GN destination adress -- destination adress(GeoUnicast) or geo. area (GeoBroadcast or GeoAnycast)
	    uint8_t GNTraClass; // GN Traffic Class
	    double GNRemainingLife; //GN Remaining Packet Lifetime in [s] /OPCIONAL/
	    int16_t GNRemainingHL; // GN Remaining Hop Limit /OPCIONAL/
	    uint8_t GNType; // GN Packet transport type -- GeoUnicast, SHB, TSB, GeoBroadcast or GeoAnycast
	    uint32_t lenght; // Payload size
	    unsigned char* data; // Payload
	} GNDataIndication_t;

	inline uint16_t swap_16bit(uint16_t us)
	{
	    return (uint16_t)(((us & 0xFF00) >> 8) |
		                    ((us & 0x00FF) << 8));
	}

	inline uint32_t swap_32bit(uint32_t ul)
	{
	    return (uint32_t)(((ul & 0xFF000000) >> 24) |
		                   ((ul & 0x00FF0000) >>  8) |
		                   ((ul & 0x0000FF00) <<  8) |
		                   ((ul & 0x000000FF) << 24));
	}
}

#endif //S_LDM_UTILS_H
