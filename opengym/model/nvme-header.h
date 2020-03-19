#ifndef NVME_HEADER_H
#define NVME_HEADER_H

#include "ns3/header.h"
#include "ns3/type-id.h"
#include <map>
#include <set>

namespace ns3 {

//------------------------- FLOW HEADER ---------------------------//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |           user  ID            |     
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

class NVMeHeader : public Header
{
public:
    NVMeHeader ();
    NVMeHeader (uint16_t userId);
    virtual ~NVMeHeader ();

    static ns3::TypeId GetTypeId ();
    virtual ns3::TypeId GetInstanceTypeId () const;
    virtual uint32_t GetSerializedSize () const;
    virtual void Serialize (ns3::Buffer::Iterator start) const;
    virtual uint32_t Deserialize (ns3::Buffer::Iterator start);
    virtual void Print (std::ostream& os) const;
    
   	uint16_t GetUserId () const;
    void SetUserId (uint16_t userId);	
   	uint64_t GetTimestamp () const;
    void SetTimestamp (uint64_t timestamp);	


protected:
    uint16_t m_userId;
	uint64_t m_timestamp;
};

}

#endif /* NVME_HEADER_H */
