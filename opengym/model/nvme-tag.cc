#include "nvme-tag.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NVMeTag);

NVMeTag::NVMeTag ()
: Tag{}
, m_timestamp{0}
{}

NVMeTag::~NVMeTag () {}

TypeId NVMeTag::GetTypeId ()
{
    static TypeId tid = TypeId ("NVMeTag")
      .SetParent<Tag> ()
      .AddConstructor<NVMeTag> ()
    ;
    return tid;
}

TypeId NVMeTag::GetInstanceTypeId () const
{
    return GetTypeId ();
}

uint32_t NVMeTag::GetSerializedSize () const
{
    return sizeof (m_timestamp);
}

void NVMeTag::Serialize (ns3::TagBuffer start) const
{
    start.WriteU64 (m_timestamp);
}

void NVMeTag::Deserialize (ns3::TagBuffer start)
{
    m_timestamp = start.ReadU64 ();
}

void NVMeTag::Print (std::ostream& os) const
{
    os << "NVMeTag " 
       << ", timestamp = " << m_timestamp;
    os << std::endl;
}

uint64_t NVMeTag::GetTimestamp () const
{
    return m_timestamp;
}

void NVMeTag::SetTimestamp (uint64_t timestamp)
{
    m_timestamp = timestamp;
}


}
