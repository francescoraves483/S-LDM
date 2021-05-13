//
// Created by carlos on 11/05/21.
//

#include "btpHeader.h"
namespace etsiDecoder {
    btpHeader::btpHeader() {
        m_source_destInfo = 0;
        m_destinationPort = 0;
    }

    btpHeader::~btpHeader() = default;

    void btpHeader::removeHeader(unsigned char * buffer) {

        memcpy(&m_destinationPort, buffer, sizeof(uint16_t));
        buffer += 2;
        m_destinationPort = swap_16bit(m_destinationPort);

        memcpy(&m_source_destInfo, buffer, sizeof(uint16_t));
        buffer += 2;
        m_source_destInfo = swap_16bit(m_source_destInfo);
    }
}