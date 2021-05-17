//
// Created by carlos on 11/05/21.
//
#include <iostream>
#include "btp.h"
namespace etsiDecoder{
	btp::btp() = default;

	btp::~btp() = default;

	btpError_e
	btp::decodeBTP(GNDataIndication_t dataIndication, BTPDataIndication_t* btpDataIndication) {
		btpHeader header;


		btpDataIndication->data = dataIndication.data;

		header.removeHeader(btpDataIndication->data);
		btpDataIndication->data += 4;

		btpDataIndication->BTPType = dataIndication.upperProtocol;

		if((header.getDestPort ()!= CA_PORT) && (header.getDestPort ()!= DEN_PORT))
		{
			std::cerr << "[ERROR] [Decoder] BTP port not supported" << std::endl;
			return BTP_ERROR;
		}

		btpDataIndication->destPort = header.getDestPort ();

		if(btpDataIndication->BTPType == BTP_A)
		{
			btpDataIndication->sourcePort = header.getSourcePort ();
			btpDataIndication->destPInfo = 0;
		}
		else if(btpDataIndication->BTPType == BTP_B)  //BTP-B
		{
			btpDataIndication->destPInfo = header.getDestPortInfo ();
			btpDataIndication->sourcePort = 0;
		}
		else
		{
			std::cerr << "[ERROR] [Decoder] Incorrect transport protocol " << std::endl;
			return BTP_ERROR;
		}

		btpDataIndication->GnAddress = dataIndication.GnAddressDest;
		btpDataIndication->GNTraClass = dataIndication.GNTraClass;
		btpDataIndication->GNRemPLife = dataIndication.GNRemainingLife;
		btpDataIndication->GNPositionV = dataIndication.SourcePV;
		btpDataIndication->data = dataIndication.data + 4;
		btpDataIndication->lenght = dataIndication.lenght - 4;

		return BTP_OK;
	}
}
