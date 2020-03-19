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

	// Check
	std::cout << "Target : " << m_totalTargets-1 << " " << ip << " " << port  << " " << target->GetTier()<< std::endl;
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

	for(uint16_t data = 0; data < m_totalDataObjects; data++){
		std::cout << data << " : " << dataObjectMap[data]->GetPopularity()/totalPopul << " ";
	}

	float check = dataObjectMap[0]->GetPopularity()/totalPopul ;
	int index = 0;
	while(r > check){
		index++;
		check += dataObjectMap[index]->GetPopularity()/totalPopul;
	}
	
	std::cout << index << " " << r << " " << check << std::endl;

	return index;
}




}
