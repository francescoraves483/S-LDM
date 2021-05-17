//
// Created by carlos on 06/05/21.
//

#ifndef S_LDM_GEONET_H
#define S_LDM_GEONET_H


#include <cstdint>
#include <map>
#include <set>
#include <mutex>
#include "basicHeader.h"
#include "commonHeader.h"
#include "shbHeader.h"
#include "gbcHeader.h"

namespace etsiDecoder
{
    class GeoNet {
    public:

        GeoNet();

        ~GeoNet();

        gnError_e decodeGN(unsigned char * packet, GNDataIndication_t* dataIndication);


    private:
        GNDataIndication_t* processSHB(GNDataIndication_t* dataIndication);

        GNDataIndication_t* processGBC(GNDataIndication_t* dataIndication, uint8_t shape);

        bool decodeLT(uint8_t lifeTime, double * seconds);

        //ETSI 302 636-4-1 ANNEX H: GeoNetworking protocol constans
        uint8_t m_GnPtotocolVersion = 1;
        uint8_t m_GnIfType = 1;
        uint32_t m_GnPaiInterval = 80;
        uint32_t m_GnMaxSduSize = 1398;
        uint8_t m_GnMaxGeoNetworkingHeaderSize = 88;
        uint8_t m_GnSecurity = 0; //!Disabled
        uint16_t m_GnDefaultTrafficClass = 0;
    };
}

#endif //S_LDM_GEONET_H
