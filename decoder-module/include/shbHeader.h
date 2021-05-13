//
// Created by carlos on 07/05/21.
//

#ifndef S_LDM_SHBHEADER_H
#define S_LDM_SHBHEADER_H

#include <cstdint>
#include <cstring>
#include "utils.h"

namespace etsiDecoder {
    class shbHeader {

    public:
        shbHeader();
        ~shbHeader();
        void removeHeader(unsigned char * buffer);

        //Getters
        [[nodiscard]] GNlpv_t GetLongPositionV() const {return m_sourcePV;}

    private:
        GNlpv_t m_sourcePV;  //!Source long position vector
        uint8_t m_reserved; //! aux variable for reading reserved fields
        uint32_t m_reserved32;

    };
}

#endif //S_LDM_SHBHEADER_H
