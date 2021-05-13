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

namespace etsiDecoder
{
    class GeoNet {
    public:

        typedef struct LocTableEntry {
            /**
            *   ETSI EN 302 636-4-1 [8.1.2]
            */
            unsigned char GN_ADDR[8];
            unsigned char LL_ADDR[6];
            uint8_t type;
            uint8_t version;
            GNlpv_t lpv; //! long position vector
            bool LS_PENDING;
            bool IS_NEIGHBOUR;
            std::set<uint16_t> DPL; //! Duplicate packet list
            long timestamp;
            uint32_t PDR;
        } GNLocTE;


        GeoNet();

        ~GeoNet();

        GNDataIndication_t decodeGN(unsigned char * packet);


    private:
        GNDataIndication_t processSHB(GNDataIndication_t dataIndication);

        //void processGBC(GNDataIndication_t dataIndication, char* address, uint8_t shape);

        double decodeLT(uint8_t lifeTime);
        bool DPD(uint16_t seqNumber,char * address);
        static bool DAD(char *address);


        std::map<char [8], GNLocTE> m_GNLocT;//! ETSI EN 302 636-4-1 [8.1]
        //std::map<char [8], Timer> m_GNLocTTimer; //! Timer for every new entry in th Location Table

        //std::map<GNDataRequest_t, std::pair<Timer, Timer>> m_Repetition_packets;//! Timers for packets with repetition interval enabled


        std::mutex m_LocT_Mutex;
        unsigned char m_GNAddress [8] = {
                0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        uint16_t m_seqNumber; //! ETSI EN 302 636-4-1 [8.3]

        //ETSI 302 636-4-1 ANNEX H: GeoNetworking protocol constans
        unsigned char m_GnLocalGnAddr[8];
        uint8_t m_GnLocalAddrCongMethod = 1; //! MANAGED
        uint8_t m_GnPtotocolVersion = 1;
        bool m_GnIsMobile = true; //!To set wether if Mobile(1) or Stationary(0)
        uint8_t m_GnIfType = 1;
        double m_GnMinUpdateFrequencyEPV = 1000;
        uint32_t m_GnPaiInterval = 80;
        uint32_t m_GnMaxSduSize = 1398;
        uint8_t m_GnMaxGeoNetworkingHeaderSize = 88;
        uint8_t m_GnLifeTimeLocTE = 20; //! seconds
        uint8_t m_GnSecurity = 0; //!Disabled
        uint8_t m_GnSnDecapResultHandling = 0; //!STRICT
        uint8_t m_GnLocationServiceMaxRetrans = 10;
        uint16_t m_GnLocationServiceRetransmitTimer = 1000;
        uint16_t m_GnLocationServicePacketBufferSize = 1024;
        uint16_t m_GnBeaconServiceRetransmitTimer = 3000;
        uint16_t m_GnBeaconServiceMaxJItter = m_GnBeaconServiceRetransmitTimer / 4;
        uint8_t m_GnDefaultHopLimit = 10;
        uint8_t m_GnDPLLength = 8;
        uint16_t m_GNMaxPacketLifetime = 600;
        uint8_t m_GnDefaultPacketLifetime = 60; // seconds (0xf2)
        uint16_t m_GNMaxPacketDataRate = 100;
        uint16_t m_GNMaxPacketDataRateEmaBeta = 90;
        uint16_t m_GNMaxGeoAreaSize = 10;
        uint16_t m_GNMinPacketRepetitionInterval = 100;
        uint16_t m_GNNonAreaForwardingAlgorithm = 1; //GREEDY
        uint16_t m_GNAreaForwardingAlgorithm = 1;
        uint16_t m_GNCbfMinTime = 1;
        uint16_t m_GNCbfMaxTime = 100;
        uint16_t m_GnDefaultMaxCommunicationRange = 1000;
        uint16_t m_GnBroadcastCBFDefSectorAngle = 30;
        uint16_t m_GnUcForwardingPacketBufferSize = 256;
        uint16_t m_GnBcForwardingPacketBufferSize = 1024;
        uint16_t m_FnCbfPacketBufferSize = 256;
        uint16_t m_GnDefaultTrafficClass = 0;
        bool m_RSU_epv_set = false;
    };
}

#endif //S_LDM_GEONET_H
