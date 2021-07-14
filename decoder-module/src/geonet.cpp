//
// Created by carlos on 06/05/21.
//
#include <iostream>
#include "geonet.h"

namespace etsiDecoder {
    GeoNet::GeoNet() {
    }
    GeoNet::~GeoNet() = default;

    bool
    GeoNet::decodeLT (uint8_t lifeTime, double*seconds)
    {
        uint8_t base,multiplier;

        base = lifeTime & 0x03;
        multiplier = (lifeTime & 0xFC) >> 2;

        switch (base)
        {
            case 0:
                *seconds = multiplier * 0.050;
                break;
            case 1:
                *seconds = multiplier * 1; // Put 1 just for completion
                break;
            case 2:
                *seconds = multiplier * 10.0;
                break;
            case 3:
                *seconds = multiplier * 100.0;
                break;
            default:
                return false;
                break;
        };

        return true;
    }

    gnError_e
    GeoNet::decodeGN(unsigned char *packet, GNDataIndication_t* dataIndication)
    {
        basicHeader basicH;
        commonHeader commonH;

        dataIndication->data = packet;

        basicH.removeHeader(dataIndication->data);
        dataIndication->data += 4;
        dataIndication->GNRemainingLife = basicH.GetLifeTime ();
        dataIndication->GNRemainingHL = basicH.GetRemainingHL ();

        //Basic Header Procesing according to ETSI EN 302 636-4-1 [10.3.3]
        //1)Check version field
        if(basicH.GetVersion() != m_GnPtotocolVersion && basicH.GetVersion() != 0)
        {
            std::cerr<< "[ERROR] [Decoder] Incorrect version of GN protocol" << std::endl;
            return GN_VERSION_ERROR;

        } 
        // This warning can be useful, but, as in the 5G-CARMEN tests all the packets have a GeoNetworking version equal to "0",
        // it has been commented out not to make the logs grow too much
        // else if(basicH.GetVersion() == 0) {
            // std::cerr<< "[WARN] [Decoder] Unexpected GeoNetworking version \"0\"" << std::endl;
        // }
        //2)Check NH field
        if(basicH.GetNextHeader()==2) //a) if NH=0 or NH=1 proceed with common header procesing
        {
            //Secured packet
            std::cerr << "[ERROR] [Decoder] Secured packet not supported" << std::endl;
            return GN_SECURED_ERROR;
        }
        if(!decodeLT(basicH.GetLifeTime(),&dataIndication->GNRemainingLife))
        {
            std::cerr << "[ERROR] [Decoder] Unable to decode lifetime field" << std::endl;
            return GN_LIFETIME_ERROR;
        }
        //Common Header Processing according to ETSI EN 302 636-4-1 [10.3.5]
        commonH.removeHeader(dataIndication->data);
        dataIndication->data += 8;
        dataIndication->upperProtocol = commonH.GetNextHeader (); //!Information needed for step 7
        dataIndication->GNTraClass = commonH.GetTrafficClass (); //!Information needed for step 7
        //1) Check MHL field
        if(commonH.GetMaxHopLimit() < basicH.GetRemainingHL())
        {
            std::cerr << "[ERROR] [Decoder] Max hop limit greater than remaining hop limit" << std::endl; //a) if MHL<RHL discard packet and omit execution of further steps
            return GN_HOP_LIMIT_ERROR;
        }
        //2) process the BC forwarding buffer, for now not implemented (SCF in traffic class disabled)
        //3) check HT field
        dataIndication->GNType = commonH.GetHeaderType();
        dataIndication->lenght = commonH.GetPayload ();

        switch(dataIndication->GNType)
        {
            case GBC:
                dataIndication = processGBC (dataIndication, commonH.GetHeaderSubType ());
                break;
            case TSB:
                if((commonH.GetHeaderSubType ()==0)) dataIndication = processSHB(dataIndication);
                else {
                    std::cerr << "[ERROR] [Decoder] GeoNet packet not supported" << std::endl;
                    return GN_TYPE_ERROR;
                  }
                break;
            default:
                std::cerr << "[ERROR] [Decoder] GeoNet packet not supported. GNType: " << static_cast<unsigned int>(dataIndication->GNType) << std::endl;
                return GN_TYPE_ERROR;
        }
        return GN_OK;
    }


    GNDataIndication_t*
    GeoNet::processSHB (GNDataIndication_t* dataIndication)
    {
        shbHeader shbH;

        shbH.removeHeader(dataIndication->data);
        dataIndication->data += 28;
        dataIndication->SourcePV = shbH.GetLongPositionV ();
        dataIndication->GNType = TSB;
        

        //7) Pass the payload to the upper protocol entity
        return dataIndication;
    }

    GNDataIndication_t*
    GeoNet::processGBC (GNDataIndication_t* dataIndication, uint8_t shape)
    {
        gbcHeader gbcH;

        gbcH.removeHeader(dataIndication->data);
        dataIndication->data += 44;
        dataIndication->SourcePV = gbcH.GetLongPositionV ();
        dataIndication->GnAddressDest = gbcH.GetGeoArea ();
        dataIndication->GnAddressDest.shape = shape;

        //3)Determine function F as specified in ETSI EN 302 931
        /*Not implemented for this decoder*/

        //Pass the payload to the upper protocol entity
        dataIndication->GNType = GBC;

        return dataIndication;
    }
}
