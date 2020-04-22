#include "target-table.h"
#include <cmath>
#include <cstdlib>

namespace ns3 {

TargetTable::TargetTable()
: m_totalTargets{0}
{}


void TargetTable::AddTarget(uint16_t tier, Ipv4Address ip, uint16_t port){
	Ptr<TargetElement> target;
	if(tier == TARGETS-1)
		target = new TargetElement(tier, ip, port, DATAS * FILESIZE);
	else
		target = new TargetElement(tier, ip, port, TARGETSIZE);
	targetMap[m_totalTargets++] = target;
}

void TargetTable::AddDataObjects(uint16_t numberOfData){
	m_totalDataObjects = numberOfData;
	// Count Total number of tiers
	uint16_t totalNumTiers = 0;
	for(uint16_t tier = 0; tier < NUMOFTIER ; tier++){
		totalNumTiers += TIERS[tier];
	}

	NS_ASSERT(totalNumTiers == m_totalTargets);
	NS_ASSERT((TARGETSIZE / FILESIZE) * m_totalTargets >= numberOfData);
	
	// Initial zipf variable
	float total = 0;
	for(uint16_t data = 0; data < numberOfData; data++){
		total += (1/std::pow(data+1, ZIPFALPHA));
	}

	// Make data objects
	int target = 0;
	uint16_t random_var = rand()%numberOfData;

	for(uint16_t data = 0; data < numberOfData; data++){
		float popul = (1/std::pow((random_var + data)%numberOfData+1, ZIPFALPHA))/total;
		Ptr<DataObject> dataObject = new DataObject(data, FILESIZE, target, popul);
		// Add data object to proper target
		targetMap[target]->AddData(dataObject->GetObjectId(), dataObject->GetSize(), dataObject);
		target = (target+1) % m_totalTargets;
		dataObjectMap[data] = dataObject;
	}
}

uint16_t TargetTable::SelectData(){
	float r = (rand()%10000)*0.0001f;
	float totalPopul = 0;
	for(uint16_t data = 0; data < m_totalDataObjects; data++){
		totalPopul += dataObjectMap[data]->GetPopularity();
	}
	float check = dataObjectMap[0]->GetPopularity()/totalPopul ;
	int index = 0;
	while(r > check){
		index++;
		check += dataObjectMap[index]->GetPopularity()/totalPopul;
	}
	return index;
}

uint16_t TargetTable::GetTargetOfDataObject(uint16_t dataId){
	for(auto it=targetMap.begin(); it!= targetMap.end(); it++){
		if(it->second->CheckData(dataId)){
			return it->second->GetTier();
		}
	}
}


// For Migration

void TargetTable::MigrationStart(uint16_t dataId, uint16_t destTargetId, uint64_t filesize){
	std::cout << "MIG START // dataId : " << dataId << ", dest : "<< destTargetId << std::endl;
	
	// secure migration space
	if(targetMap[destTargetId]->CheckRemainingSpace(filesize)){
		targetMap[destTargetId]->SetCurrentSize(targetMap[destTargetId]->GetCurrentSize() + filesize);
	};

	// Set data's state to NowMigration
	dataObjectMap[dataId]->SetNowMigration(true);

	// Make New entry of migration progress
	Ptr<Migration> migration = new Migration(dataId, GetTargetOfDataObject(dataId), destTargetId,  MIGRATIONTIME, filesize); 

	// Set Callback
	migration->SetSendTTReadCallback(MakeCallback(&TargetTable::SendTTReadRequestPacket, this));
	migration->SetSendTTWriteCallback(MakeCallback(&TargetTable::SendTTWriteRequestPacket, this));
	migration->SetSendTTFinishCallback(MakeCallback(&TargetTable::MigrationFinish, this));

	// Start Event
	migration->SendReadRequest();
	migrationProgress[dataId] = migration;

}

void TargetTable::MigrationFinish(uint16_t dataId, uint16_t sourceTarget, uint16_t destTarget, uint64_t filesize){
	std::cout << "MIG FINISH // dataI?d : " << dataId << ",dest : "<< destTarget << std::endl;
	// Remove secured space
	targetMap[destTarget]->SetCurrentSize(targetMap[destTarget]->GetCurrentSize() - filesize);

	// Migration
	targetMap[sourceTarget]->RemoveData(dataId);
	targetMap[destTarget]->AddData(dataId, filesize, dataObjectMap[dataId]); 
}

void TargetTable::MigrationDelete(uint16_t dataId){
	// Change data's NowMigration
	dataObjectMap[dataId]->SetNowMigration(false);
	
	// Delete Migration Progress instance
	migrationProgress.erase(dataId);	
};

void TargetTable::SetSendHostReadCallback(sendHostReadCallback scb){
	m_sendHostReadCallback = scb;
};

void TargetTable::SetSendHostWriteCallback(sendHostWriteCallback scb){
	m_sendHostWriteCallback = scb;
};

// Migration comm. from Migration Object

void TargetTable::SendTTReadRequestPacket(uint16_t dataId){
	m_sendHostReadCallback(1,dataId); //1 is Migration
};

void TargetTable::SendTTWriteRequestPacket(uint16_t dataId, uint16_t targetId){
	m_sendHostWriteCallback(dataId, targetId);
};


// Migration comm. from Host

void TargetTable::GetReadResponsePacket(uint16_t dataId){
	auto it = migrationProgress.find(dataId);
	if(it != migrationProgress.end())
		migrationProgress[dataId]->GetReadResponse();
};

void TargetTable::GetWriteResponsePacket(uint16_t dataId){
	auto it = migrationProgress.find(dataId);
	if(it != migrationProgress.end())
		migrationProgress[dataId]->GetWriteResponse();
};

}
