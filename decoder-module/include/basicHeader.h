//
// Created by carlos on 07/05/21.
//

#ifndef S_LDM_BASICHEADER_H
#define S_LDM_BASICHEADER_H

#include <cstdint>

namespace etsiDecoder {
    class basicHeader {
    public:
        basicHeader();
        ~basicHeader();
        void removeHeader(unsigned char * buffer);

        //Getters
        [[nodiscard]] uint8_t GetVersion() const {return m_version;}
        [[nodiscard]] uint8_t GetNextHeader() const {return m_nextHeader;}
        [[nodiscard]] uint8_t GetLifeTime() const {return m_lifeTime;}
        [[nodiscard]] uint8_t GetRemainingHL() const{return m_remainingHopLimit;}

    private:
            uint8_t m_version: 4;
            uint8_t m_nextHeader: 4;
            uint8_t m_reserved;
            uint8_t m_lifeTime;
            uint8_t m_remainingHopLimit;
        };
}

#endif //S_LDM_BASICHEADER_H
