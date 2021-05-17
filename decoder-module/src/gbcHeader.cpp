#include "gbcHeader.h"

namespace etsiDecoder {

gbcHeader::gbcHeader()
{
  m_sourcePV = { };  //!Source long position vector
  m_seqNumber = 0;
  m_posLong = 0;
  m_posLat = 0;
  m_distA = 0;
  m_distB = 0;
  m_angle = 0;
  m_reserved = 0; //! aux variable for reading reserved fields
}

gbcHeader::~gbcHeader() = default;

void gbcHeader::removeHeader(unsigned char *buffer) {

  //Sequence Number
  memcpy(&m_seqNumber, buffer, sizeof(uint16_t));
  buffer += 2;
  m_seqNumber = swap_16bit(m_seqNumber);
  //Reserved *this is not really needed*
  memcpy(&m_reserved, buffer, sizeof(uint16_t));
  buffer += 2;
  m_reserved = swap_16bit(m_reserved);

  //Source long position vector
  memcpy(m_sourcePV.GnAddress, buffer, 8);
  buffer += 8;
  memcpy(&m_sourcePV.TST, buffer, sizeof(uint32_t));
  buffer += 4;
  m_sourcePV.TST = swap_32bit(m_sourcePV.TST);
  memcpy(&m_sourcePV.latitude, buffer, sizeof(uint32_t));
  buffer += 4;
  m_sourcePV.latitude = swap_32bit(m_sourcePV.latitude);
  memcpy(&m_sourcePV.longitude, buffer, sizeof(uint32_t));
  buffer += 4;
  m_sourcePV.longitude = swap_32bit(m_sourcePV.longitude);
  uint16_t pai_speed = 0;
  memcpy(&pai_speed, buffer, sizeof(uint16_t));
  buffer += 2;
  pai_speed = swap_16bit(pai_speed);
  m_sourcePV.positionAccuracy = pai_speed >> 15;
  m_sourcePV.speed = pai_speed & 0x7f;
  memcpy(&m_sourcePV.heading, buffer, sizeof(uint16_t));
  buffer += 2;
  m_sourcePV.heading = swap_16bit(m_sourcePV.heading);

  //GeoArea position latitude
  memcpy(&m_posLat, buffer, sizeof(uint32_t));
  buffer += 4;
  m_posLat = swap_32bit(m_posLat);
  //GeoArea position longitude
  memcpy(&m_posLong, buffer, sizeof(uint32_t));
  buffer += 4;
  m_posLong = swap_32bit(m_posLong);
  //Distance A
  memcpy(&m_distA, buffer, sizeof(uint16_t));
  buffer += 2;
  m_distA = swap_16bit(m_distA);
  //Distance B
  memcpy(&m_distB, buffer, sizeof(uint16_t));
  buffer += 2;
  m_distB = swap_16bit(m_distB);
  //Angle
  memcpy(&m_angle, buffer, sizeof(uint16_t));
  buffer += 2;
  m_angle = swap_16bit(m_angle);
  //Reserved *this is not really needed* maybe could be used to check if indeed is 0x0000
  memcpy(&m_reserved, buffer, sizeof(uint16_t));
  buffer += 2;
  m_reserved = swap_32bit(m_reserved);
}

GeoArea_t
gbcHeader::GetGeoArea() const
{
  GeoArea_t retval;
  retval.posLat = m_posLat;
  retval.posLong = m_posLong;
  retval.distA = m_distA;
  retval.distB = m_distB;
  retval.angle = m_angle;
  return retval;
}
}
