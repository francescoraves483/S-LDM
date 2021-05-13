//
// Created by carlos on 11/05/21.
//

#include "btp.h"
namespace etsiDecoder{
    btp::btp() = default;

    btp::~btp() = default;

    BTPDataIndication_t
    btp::decodeBTP(GNDataIndication_t dataIndication) {
        btpHeader header;
        BTPDataIndication_t btpDataIndication = {};

        btpDataIndication.data = dataIndication.data;

        header.removeHeader(btpDataIndication.data);
        btpDataIndication.data += 4;

        btpDataIndication.BTPType = dataIndication.upperProtocol;
        btpDataIndication.destPort = header.getDestPort ();

        if(btpDataIndication.BTPType == BTP_A)
        {
            btpDataIndication.sourcePort = header.getSourcePort ();
            btpDataIndication.destPInfo = 0;
        }
        else //BTP-B
        {
            btpDataIndication.destPInfo = header.getDestPortInfo ();
            btpDataIndication.sourcePort = 0;
        }
        //btpDataIndication.GnAddress = dataIndication.GnAddressDest;  // Left for gbc implementation
        btpDataIndication.GNTraClass = dataIndication.GNTraClass;
        btpDataIndication.GNRemPLife = dataIndication.GNRemainingLife;
        btpDataIndication.GNPositionV = dataIndication.SourcePV;
        btpDataIndication.data = dataIndication.data + 4;
        btpDataIndication.lenght = dataIndication.lenght - 4;

        return btpDataIndication;
    }
}