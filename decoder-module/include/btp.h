//
// Created by carlos on 11/05/21.
//

#ifndef S_LDM_BTP_H
#define S_LDM_BTP_H
#include "utils.h"
#include "btpHeader.h"
#include <cstdint>
#include <cstring>

#define CA_PORT 2001
#define DEN_PORT 2002

namespace etsiDecoder {
    class btp {

    public :
        btp();
        ~btp();
        btpError_e decodeBTP(GNDataIndication_t dataIndication, BTPDataIndication_t* btpDataIndication);

    };
}
#endif //S_LDM_BTP_H
