#ifndef GBCHEADER_H
#define GBCHEADER_H

#include <cstdint>
#include <cstring>
#include "utils.h"

namespace etsiDecoder {

class gbcHeader
{


public:
  gbcHeader();
  ~gbcHeader();
  void removeHeader(unsigned char * buffer);

  //Getters
  GNlpv_t GetLongPositionV(void) const {return m_sourcePV;}
  uint16_t GetSeqNumber(void) const {return m_seqNumber;}
  int32_t GetPosLong(void) const {return m_posLong;}
  int32_t GetPosLat(void) const {return m_posLat;}
  uint16_t GetDistA(void) const {return m_distA;}
  uint16_t GetDistB(void) const {return m_distB;}
  uint16_t GetAngle(void) const {return m_angle;}
  GeoArea_t GetGeoArea(void) const;

private:
  GNlpv_t m_sourcePV;  //!Source long position vector
  uint16_t m_seqNumber;
  int32_t m_posLong;
  int32_t m_posLat;
  uint16_t m_distA;
  uint16_t m_distB;
  uint16_t m_angle;
  uint16_t m_reserved; //! aux variable for reading reserved fields


};

}
#endif // GBCHEADER_H
