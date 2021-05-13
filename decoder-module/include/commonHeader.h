//
// Created by carlos on 07/05/21.
//

#ifndef S_LDM_COMMONHEADER_H
#define S_LDM_COMMONHEADER_H

#include <cstdint>
#include "utils.h"

namespace etsiDecoder {
	class commonHeader {
		public:
			commonHeader();
			~commonHeader();
			void removeHeader(unsigned char * buffer);

		//Getters
		[[nodiscard]] uint8_t GetNextHeader() const {return m_nextHeader;}
		[[nodiscard]] uint8_t GetHeaderType() const {return m_headerType;}
		[[nodiscard]] uint8_t GetHeaderSubType() const {return m_headerSubType;}
		[[nodiscard]] uint8_t GetTrafficClass() const {return m_trafficClass;}
		[[nodiscard]] bool GetFlag() const {return m_flag;}
		[[nodiscard]] uint16_t GetPayload() const {return m_payload;}
		[[nodiscard]] uint8_t GetMaxHopLimit() const {return m_maxHopLimit;}


	private:
			uint8_t m_nextHeader : 4;
			uint8_t m_headerType : 4;
			uint8_t m_headerSubType : 4;
			uint8_t m_trafficClass;
			bool m_flag : 1;
			uint16_t m_payload;
			uint8_t m_maxHopLimit;
			uint8_t m_reserved;
	};
}

#endif //S_LDM_COMMONHEADER_H
