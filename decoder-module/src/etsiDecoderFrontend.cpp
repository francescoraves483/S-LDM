#include <iostream>
#include "etsiDecoderFrontend.h"
#include "geonet.h"
#include "btp.h"
#include "basicHeader.h"
#include "commonHeader.h"
#include "shbHeader.h"

extern "C" {
	#include "CAM.h"
	#include "DENM.h"
}

namespace etsiDecoder {
	decoderFrontend::decoderFrontend() {
		m_print_pkt = false;
	}

	int decoderFrontend::decodeEtsi(uint8_t *buffer,size_t buflen,etsiDecodedData_t &decoded_data) {
		if(buflen<=0) {
			return ETSI_DECODER_ERROR;
		}

		if(m_print_pkt==true) {
			std::cout << "Packet content :" << std::endl;
			for(uint32_t i=0;i<buflen;i++) {
				std::printf("%02X ",buffer[i]);
			}
			std::cout << std::endl;
		}

		GeoNet geonet;
		btp BTP;
		GNDataIndication_t gndataIndication;
		BTPDataIndication_t btpDataIndication;

		gndataIndication = geonet.decodeGN(buffer);

		btpDataIndication = BTP.decodeBTP(gndataIndication);

		if(m_print_pkt==true) {
			std::cout << "ETSI packet content :" << std::endl;
			for(uint32_t i=0;i<btpDataIndication.lenght;i++) {
				std::printf("%02X ",btpDataIndication.data[i]);
			}
			std::cout << std::endl;
		}

		decoded_data.gnTimestamp = gndataIndication.SourcePV.TST;

		void *decoded_=nullptr;
		asn_dec_rval_t decode_result;

		if(btpDataIndication.destPort == CA_PORT) {
			decoded_data.type = ETSI_DECODED_CAM;

			decode_result = asn_decode(0, ATS_UNALIGNED_BASIC_PER, &asn_DEF_CAM, &decoded_, btpDataIndication.data, btpDataIndication.lenght);

			if(decode_result.code!=RC_OK || decoded_==nullptr) {
				std::cerr << "Warning: unable to decode a received CAM." << std::endl;
				if(decoded_) free(decoded_);
				return ETSI_DECODER_ERROR;
			}
		} else if(btpDataIndication.destPort == DEN_PORT) {
			decoded_data.type = ETSI_DECODED_DENM;

			decode_result = asn_decode(0, ATS_UNALIGNED_BASIC_PER, &asn_DEF_DENM, &decoded_, btpDataIndication.data, btpDataIndication.lenght);

			if(decode_result.code!=RC_OK || decoded_==nullptr) {
				std::cerr << "Warning: unable to decode a received DENM." << std::endl;
				if(decoded_) free(decoded_);
				return ETSI_DECODER_ERROR;
			}
		// Only CAMs and DENMs are supported for the time being
		} else {
			decoded_data.type = ETSI_DECODED_ERROR;
			return ETSI_DECODER_ERROR;
		}

		decoded_data.decoded_msg = decoded_;

		return ETSI_DECODER_OK;
	}
}