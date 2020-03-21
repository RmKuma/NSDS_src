#ifndef TARGET_TABLE_H
#define TARGET_TABLE_H

#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/address.h"
#include "opengym-parameter.h"

#include <map>

namespace ns3 {

	class DataObject : public Object
	{
		private:
			uint16_t m_objectId;
			uint16_t m_target;
			uint64_t m_dataSize;
			float m_popularity;

		public:
			DataObject () {};
			DataObject (uint16_t id, uint64_t size, uint16_t target, float popul){
				m_objectId = id;
				m_dataSize = size;
				m_target = target;
				m_popularity = popul;
			};
			virtual ~DataObject () {};

			void SetTarget(uint16_t target){
				m_target = target;
			};
			void SetPopularity(float popularity){
				m_popularity = popularity;
			};


			uint16_t GetObjectId(){
				return m_objectId;
			};
			uint16_t GetTarget(){
				return m_target;
			};
			uint64_t GetSize(){
				return m_dataSize;
			};
			float    GetPopularity(){
				return m_popularity;
			};

	};

class TargetElement : public Object
{
public:
	uint64_t m_maxSize;
	uint64_t m_currentSize;

private:
	uint16_t m_tier;
	std::map<uint16_t, Ptr<DataObject>> dataObjectMap;
	Ipv4Address m_ip;
	uint16_t m_port;

public:
	TargetElement (uint16_t tier, Ipv4Address ip, uint16_t port, uint64_t size) {
		m_tier = tier;
		m_ip = ip;
		m_port = port;
		m_maxSize = size;
		m_currentSize = 0;
	};
	~TargetElement () {};

	bool CheckRemainingSpace(uint64_t size){
		return size <= (m_maxSize - m_currentSize);
	};

	void RemoveData(uint16_t dataObjectId){
		dataObjectMap.erase(dataObjectId);
		m_currentSize -= FILESIZE;
	}

	bool AddData(uint16_t dataObjectId, uint64_t dataObjectSize, Ptr<DataObject> dataObject){
		if (CheckRemainingSpace(dataObjectSize)){
			dataObjectMap[dataObjectId] = dataObject;
			m_currentSize += FILESIZE;
			return true;
		}else{
			return false;
		}
	}

	uint16_t GetTier(){
		return m_tier;
	}
	uint16_t GetPort(){
		return m_port;
	}
	Ipv4Address GetIp(){
		return m_ip;
	}
};

class TargetTable 
{
public:
 	TargetTable ();
	~TargetTable () {};

	uint16_t GetTotalTargets(){
		return m_totalTargets;
	};

	uint16_t GetTotalDataObjects(){
		return m_totalDataObjects;
	};

	void AddTarget(uint16_t tier, Ipv4Address ip, uint16_t port);
	void AddDataObjects(uint16_t numberOfData); 		
	uint16_t SelectData();
	//Data Object
	std::map<uint16_t, Ptr<DataObject>> dataObjectMap; 
	//Target
	std::map<uint16_t, Ptr<TargetElement>> targetMap;

private:
	uint16_t m_totalTargets;
	uint16_t m_totalDataObjects;

};

}

#define m_dataObjectTable ns3::DataObectTable::Instance()
#endif // DATA_OBJECT_TABLE_H

