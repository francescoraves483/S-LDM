//
// Created by carlos on 06/05/21.
//
#include <iostream>
#include "geonet.h"

namespace etsiDecoder {
    GeoNet::GeoNet() {
    }
    GeoNet::~GeoNet() = default;

    double
    GeoNet::decodeLT (uint8_t lifeTime)
    {
        uint8_t base,multiplier;
        double seconds;

        base = lifeTime & 0x03;
        multiplier = (lifeTime & 0xFC) >> 2;

        switch (base)
        {
            case 0:
                seconds = multiplier * 0.050;
                break;
            case 1:
                seconds = multiplier * 1; // Put 1 just for completion
                break;
            case 2:
                seconds = multiplier * 10.0;
                break;
            case 3:
                seconds = multiplier * 100.0;
                break;
            default:
                std::cout << "GeoNet: UNABLE TO DECODE LIFETIME FIELD " << std::endl;
                break;
        };

        return seconds;
    }

    GNDataIndication_t
    GeoNet::decodeGN(unsigned char *packet)
    {
        GNDataIndication_t dataIndication = {};
        char* from;
        basicHeader basicH;
        commonHeader commonH;

        dataIndication.data = packet;

        basicH.removeHeader(dataIndication.data);
        dataIndication.data += 4;
        dataIndication.GNRemainingLife = basicH.GetLifeTime ();
        dataIndication.GNRemainingHL = basicH.GetRemainingHL ();

        //Basic Header Procesing according to ETSI EN 302 636-4-1 [10.3.3]
        //1)Check version field
        if(basicH.GetVersion() != m_GnPtotocolVersion)
        {
            std::cout << "Incorrect version of GN protocol" << std::endl;
        }
        //2)Check NH field
        if(basicH.GetNextHeader()==2) //a) if NH=0 or NH=1 proceed with common header procesing
        {
            //Secured packet
            std::cout << "Secured packet not supported" << std::endl;
        }
        dataIndication.GNRemainingLife = decodeLT(basicH.GetLifeTime ());

        //Common Header Processing according to ETSI EN 302 636-4-1 [10.3.5]
        commonH.removeHeader(dataIndication.data);
        dataIndication.data += 8;
        dataIndication.upperProtocol = commonH.GetNextHeader (); //!Information needed for step 7
        dataIndication.GNTraClass = commonH.GetTrafficClass (); //!Information needed for step 7
        //1) Check MHL field
        if(commonH.GetMaxHopLimit() < basicH.GetRemainingHL())
        {
            std::cout << "Max hop limit greater than remaining hop limit" << std::endl; //a) if MHL<RHL discard packet and omit execution of further steps
        }
        //2) process the BC forwarding buffer, for now not implemented (SCF in traffic class disabled)
        //3) check HT field
        dataIndication.GNType = commonH.GetHeaderType();
        dataIndication.lenght = commonH.GetPayload ();

        switch(dataIndication.GNType)
        {
            case BEACON:
                //if(commonHeader.GetHeaderSubType ()==0) processSHB (dataIndication,from);
                std::cout << "BEACON packet not supported" << std::endl;
                break;
            case GBC:
                //processGBC (dataIndication,from,commonHeader.GetHeaderSubType ());
                std::cout << "GBC packet not supported" << std::endl;
                break;
            case TSB:
                if((commonH.GetHeaderSubType ()==0)) dataIndication = processSHB(dataIndication);
                break;
            default:
                std::cout << "GeoNet packet not supported" << std::endl;
        }
        return dataIndication;
    }

    bool
    GeoNet::DAD(char * address)
    {
/*        unsigned char source[8];
        memcpy(source,address,8);
        if(std::equal(std::begin(source), std::end(source), std::begin(m_GNAddress)) &&
            std::equal(std::begin(source)+2, std::end(source), std::begin(m_GNAddress)+2))
        {
            //ETSI EN 302 636-4-1 [10.2.1.5] : If conflict is detected, request new MID field
            //m_GnLocalGnAddr = getGNMac48(m_socket_tx->GetNode ()->GetDevice (0)->GetAddress ());
            //m_GNAddress = m_GNAddress.MakeManagedconfiguredAddress (m_GnLocalGnAddr,m_stationtype);
            std::cout << "Duplicate address detected" << std::endl;
            return true;
        }
        else
        {
            return false;
        }*/
        return false;
    }

    //Not implemented yet
    bool
    GeoNet::DPD(uint16_t seqNumber,char * address)
    {
/*        std::set<uint16_t>::iterator it = m_GNLocT[address].DPL.find(seqNumber);
        if(it == m_GNLocT[address].DPL.end ())
        {
            //If entry doesnt exist, the packet is not a duplicate and should be added to the list
            m_GNLocT[address].DPL.insert (seqNumber);
            return false;
        }
        else
        {
            return true;//Packet is a duplicate
        }*/

        return false;
    }

    GNDataIndication_t
    GeoNet::processSHB (GNDataIndication_t dataIndication)
    {
        shbHeader shbH;

        shbH.removeHeader(dataIndication.data);
        dataIndication.data += 28;
        dataIndication.SourcePV = shbH.GetLongPositionV ();
        

        //7) Pass the payload to the upper protocol entity
        return dataIndication;
    }
}