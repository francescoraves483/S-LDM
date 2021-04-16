#ifndef METADATA_HEADER_H
#define METADATA_HEADER_H
#include <stdint.h>
#include <string>
#include "ns3/header.h"

namespace ns3 {


class GNMetadataHeader : public Header
{
  public:
    typedef struct _GNmetadata {
        uint64_t stationID;
        int32_t lat;
        int32_t lon;
    } GNmetadata_t;

    GNMetadataHeader();
    ~GNMetadataHeader();
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);

    //Setters
    void SetLat(int32_t lat) {m_lat = lat;}
    void SetLon(int32_t lon) {m_lon = lon;}
    void SetStationID(uint64_t station_id) {m_station_id = station_id;}

    //Getters
    int32_t GetLat() {return m_lat;}
    int32_t GetLon() {return m_lon;}
    uint64_t GetStationID() {return m_station_id;}

  private:
    int32_t m_lat;
    int32_t m_lon;
    uint64_t m_station_id;
  };
}
#endif // METADATA_HEADER_H
