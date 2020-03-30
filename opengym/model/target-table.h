#ifndef TARGET_TABLE_H
#define TARGET_TABLE_H

#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/address.h"
#include "opengym-parameter.h"
#include "migration.h"
#include <map>

namespace ns3 {

	class DataObject : public Object
	{
		private:
			uint16_t m_objectId;
			uint16_t m_target;
			uint64_t m_dataSize;
			float m_popularity;
			bool m_nowMigration;

		public:
			DataObject () {};
			DataObject (uint16_t id, uint64_t size, uint16_t target, float popul){
				m_objectId = id;
				m_dataSize = size;
				m_target = target;
				m_popularity = popul;
			};
			virtual ~DataObject () {};

			void SetPopularity(float popularity){
				m_popularity = popularity;
			};
			void SetNowMigration(bool now){
				m_nowMigration = now;
			};
			
			bool GetNowMigration(){
				return m_nowMigration;
			};
			uint16_t GetObjectId(){
				return m_objectId;
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

private:
	uint16_t m_tier;
	uint64_t m_currentSize;
	std::map<uint16_t, Ptr<DataObject>> dataObjectMap;

public:
	TargetElement (uint16_t tier, Ipv4Address ip, uint16_t port, uint64_t size) {
		m_tier = tier;
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

	bool CheckData(uint16_t dataId){
		std::map<uint16_t, Ptr<DataObject>>::iterator it = dataObjectMap.find(dataId);
		if(it != dataObjectMap.end())
			return true;
		else
			return false;
	}

	uint16_t GetTier(){
		return m_tier;
	}

	uint64_t GetCurrentSize(){
		return m_currentSize;
	}

	void SetCurrentSize(uint64_t size){
		m_currentSize = size;
	};
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

	//For Data Object
	std::map<uint16_t, Ptr<DataObject>> dataObjectMap; 
	uint16_t GetTargetOfDataObject(uint16_t dataId);
	void AddDataObjects(uint16_t numberOfData); 		
	uint16_t SelectData();

	//For Target Node
	std::map<uint16_t, Ptr<TargetElement>> targetMap;
	void AddTarget(uint16_t tier, Ipv4Address ip, uint16_t port);
	
	//For Migration
	typedef Callback<void, int16_t, uint16_t> sendHostReadCallback;
	void SetSendHostReadCallback(sendHostReadCallback scb);

	typedef Callback<void, uint16_t, uint16_t> sendHostWriteCallback;
	void SetSendHostWriteCallback(sendHostWriteCallback scb);

	std::map<uint16_t, Ptr<Migration>> migrationProgress; //key is dataObjectId
	
	void MigrationStart(uint16_t dataId, uint16_t destTargetId, uint64_t filesize);
	void MigrationFinish(uint16_t dataId, uint16_t sourceTarget, uint16_t destTarget, uint64_t filesize);

	void SendTTReadRequestPacket(uint16_t dataId);
	void SendTTWriteRequestPacket(uint16_t datId, uint16_t targetId);

	void GetReadResponsePacket(uint16_t dataId);
	void GetWriteResponsePacket(uint16_t dataId);


private:
	uint16_t m_totalTargets;
	uint16_t m_totalDataObjects;

	sendHostReadCallback m_sendHostReadCallback;
	sendHostWriteCallback m_sendHostWriteCallback;

};

}

#endif // DATA_OBJECT_TABLE_H

