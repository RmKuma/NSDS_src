#include "nvme-header.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NVMeHeader);

NVMeHeader::NVMeHeader ()
: Header{}
, m_userId{0}
, m_timestamp{0}
{}

NVMeHeader::NVMeHeader (uint16_t userId)
: Header{}
, m_userId{userId}
, m_timestamp{0}
{}

NVMeHeader::~NVMeHeader () {}

TypeId NVMeHeader::GetTypeId ()
{
    static TypeId tid = TypeId ("NVMeHeader")
      .SetParent<Header> ()
      .AddConstructor<NVMeHeader> ()
    ;
    return tid;
}

TypeId NVMeHeader::GetInstanceTypeId () const
{
    return GetTypeId ();
}

uint32_t NVMeHeader::GetSerializedSize () const
{
    return sizeof (m_userId) + sizeof(m_timestamp);
}

void NVMeHeader::Serialize (Buffer::Iterator start) const
{
    start.WriteHtonU16 (m_userId);
	start.WriteHtonU64 (m_timestamp);
}

uint32_t NVMeHeader::Deserialize (Buffer::Iterator start)
{
    m_userId = start.ReadNtohU16 ();
    m_timestamp = start.ReadNtohU64 ();
    
	return GetSerializedSize ();
}

void NVMeHeader::Print (std::ostream& os) const
{
    os << "NVMeHeader " 
       << ", user ID= " << m_userId
       << ", timestamp= " << m_timestamp;
    os << std::endl;
}

uint16_t NVMeHeader::GetUserId () const
{
    return m_userId;
}

void NVMeHeader::SetUserId (uint16_t userId)
{
    m_userId = userId;
}

uint64_t NVMeHeader::GetTimestamp () const
{
    return m_timestamp;
}

void NVMeHeader::SetTimestamp (uint64_t timestamp)
{
    m_timestamp = timestamp;
}



}
