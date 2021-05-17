#ifndef S_LDM_ETSIDECODERFRONTEND_H
#define S_LDM_ETSIDECODERFRONTEND_H

#include <cinttypes>
#include <cstddef>

#define ETSI_DECODER_OK 	0
#define ETSI_DECODER_ERROR 	1

namespace etsiDecoder {
	typedef enum {
		ETSI_DECODED_ERROR,
		ETSI_DECODED_CAM,
		ETSI_DECODED_DENM,
		ETSI_DECODED_IVIM,
		ETSI_DECODED_CPM
	} etsiDecodedType_e;

	typedef struct etsiDecodedData {
		void *decoded_msg;
		etsiDecodedType_e type;

		uint32_t gnTimestamp;
		//For DENMs GeoArea
		int32_t posLong;
		int32_t posLat;
		uint16_t distA;
		uint16_t distB;
		uint16_t angle;
	} etsiDecodedData_t;

	class decoderFrontend {
		public:
			decoderFrontend();
			int decodeEtsi(uint8_t *buffer,size_t buflen,etsiDecodedData_t &decoded_data);
			void setPrintPacket(bool print_pkt) {m_print_pkt=print_pkt;}

		private:
			bool m_print_pkt;
	};
}

#endif // ETSIDECODERFRONTEND_H
