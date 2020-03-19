#include "user.h"
#include "opengym-parameter.h"

namespace ns3 {

User::User ()
{}

User::User (uint16_t userId, uint16_t dataObjectId, uint64_t serviceTime, uint64_t targetDelay)
: m_userId {userId}
, m_finished {false}
, m_dataObjectId {dataObjectId}
, m_serviceTime{serviceTime}
, m_currentRequests{0}
, m_currentResults{0}
, m_targetDelay{targetDelay}
, m_currentGoodput{0}
, m_currentDelay{0}
, m_currentPackets{0}
{
	float requestPerMicroSecond = ((float)(FILESIZE/OR_PAGESIZE) / serviceTime);
	m_requestInterval = (uint64_t) (1/requestPerMicroSecond );
	m_totalRequests = (uint64_t) (FILESIZE/OR_PAGESIZE);
}

User::~User() {
}

void User::SetSendCallback(sendCallback scb){
	m_sendCallback = scb;
}

void User::SendRequest(){
	if(!m_finished){
		if(sendCounter.find(m_dataObjectId) == sendCounter.end())
			sendCounter[m_dataObjectId] = 0;
		else
			sendCounter[m_dataObjectId]++;
		m_sendCallback(m_userId);
		m_SendEvent = Simulator::Schedule(MicroSeconds (m_requestInterval), &User::SendRequest, this);
	}
};

void User::AfterGathering(){
	m_currentGoodput = 0;
	m_currentDelay = 0;
	m_currentPackets = 0;
}

void User::AfterGetPacket(uint64_t size, uint64_t delay){
	m_currentDelay += delay;
	m_currentGoodput += size;
	m_currentPackets++;
	m_currentResults++;

	if(receiveCounter.find(m_dataObjectId) == receiveCounter.end())
		receiveCounter[m_dataObjectId] = 0;
	else
		receiveCounter[m_dataObjectId]++;
	
	if(m_currentResults == m_totalRequests){
		m_finished = true;
	}
}

void User::CancelSendEvent(){
	Simulator::Cancel(m_SendEvent);
}


}
