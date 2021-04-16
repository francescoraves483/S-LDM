#include "metadata-header.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include <arpa/inet.h>

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE ("GNMetadataHeader");
  NS_OBJECT_ENSURE_REGISTERED (GNMetadataHeader);

  static uint64_t swap64(uint64_t unsignedvalue, uint32_t (*swap_byte_order)(uint32_t)) {
          #if __BYTE_ORDER == __BIG_ENDIAN
          return unsignedvalue;
          #elif __BYTE_ORDER == __LITTLE_ENDIAN
          uint32_t low;
          uint32_t high;
          high=(uint32_t) (unsignedvalue>>32);
          low=(uint32_t) (unsignedvalue & ((1ULL << 32) - 1));
          return ((uint64_t) swap_byte_order(low))<<32 | ((uint64_t) swap_byte_order(high));
          #else
          #error "The system seems to be neither little endian nor big endian..."
          #endif
  }
  uint64_t hton64 (uint64_t hostu64) {
    return swap64(hostu64,&htonl);
  }


  GNMetadataHeader::GNMetadataHeader()
  {
    NS_LOG_FUNCTION (this);
  }

  GNMetadataHeader::~GNMetadataHeader()
  {
    NS_LOG_FUNCTION (this);
  }

  TypeId
  GNMetadataHeader::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::GNMetadataHeader")
      .SetParent<Header> ()
      .SetGroupName ("Automotive")
      .AddConstructor<GNMetadataHeader> ();
    return tid;
  }

  TypeId
  GNMetadataHeader::GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

  uint32_t
  GNMetadataHeader::GetSerializedSize (void) const
  {
    return 16;
  }

  void
  GNMetadataHeader::Print (std::ostream &os) const
  {
    os << "MetadataHeader";
  }

  void
  GNMetadataHeader::Serialize (Buffer::Iterator start) const
  {
    //ETSI EN 302 636-4-1 [9.8.4]
    Buffer::Iterator i = start;

    GNmetadata_t gnmetadata;
    gnmetadata.lat = htonl(m_lat);
    gnmetadata.lon = htonl(m_lon);
    gnmetadata.stationID = hton64(m_station_id);

    i.Write((uint8_t *)&gnmetadata,sizeof(GNmetadata_t));
  }

  uint32_t
  GNMetadataHeader::Deserialize (Buffer::Iterator start)
  {
    Buffer::Iterator i = start;
    GNmetadata_t gnmetadata;
    i.Read ((uint8_t *)&gnmetadata,sizeof(GNmetadata_t));

    m_lat=gnmetadata.lat;
    m_lon=gnmetadata.lon;
    m_station_id=gnmetadata.stationID;

    return GetSerializedSize ();
  }

}

