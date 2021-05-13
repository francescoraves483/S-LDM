//
// Created by carlos on 11/05/21.
//

#ifndef S_LDM_BTPHEADER_H
#define S_LDM_BTPHEADER_H

#include <cstdint>
#include <cstring>
#include "utils.h"

namespace etsiDecoder {
    class btpHeader {
    public:
        btpHeader();
        ~btpHeader();
        void removeHeader(unsigned char * buffer);

        //getters
        [[nodiscard]] uint16_t getDestPort() const {return m_destinationPort;}
        [[nodiscard]] uint16_t getSourcePort() const {return m_source_destInfo;}
        [[nodiscard]] uint16_t getDestPortInfo() const {return m_source_destInfo;}
    private:
        uint16_t m_destinationPort; //!< Destination port
        uint16_t m_source_destInfo; //!< Source port/Destination port info
    };
}
#endif //S_LDM_BTPHEADER_H
