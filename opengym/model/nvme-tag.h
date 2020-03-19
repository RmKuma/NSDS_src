#ifndef NVME_TAG_H
#define NVME_TAG_H

#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/type-id.h"
#include <map>
#include <set>

namespace ns3 {

class NVMeTag : public Tag
{
public:
    NVMeTag ();
    NVMeTag (uint16_t userId);
    virtual ~NVMeTag ();

    static ns3::TypeId GetTypeId ();
    virtual ns3::TypeId GetInstanceTypeId () const;
    virtual uint32_t GetSerializedSize () const;
    virtual void Serialize (ns3::TagBuffer start) const;
    virtual void Deserialize (ns3::TagBuffer start);
    virtual void Print (std::ostream& os) const;
    
	uint64_t GetTimestamp () const;
    void SetTimestamp (uint64_t timestamp);

protected:
    uint64_t m_timestamp;
};

}

#endif /* NVME_TAG_H */
