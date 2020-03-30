#include "migration.h"

namespace ns3 {

Migration::Migration ()
{}

Migration::Migration ( uint16_t dataObjectId, uint16_t readTarget, uint16_t writeTarget, uint64_t serviceTime, uint64_t fileSize)
: m_finished {false}
, m_dataObjectId {dataObjectId}
, m_serviceTime{serviceTime}
, m_currentRequests{0}
, m_fileSize{fileSize}
, m_readTarget{readTarget}
, m_writeTarget{writeTarget}
{
	float requestPerMicroSecond = (((float)m_fileSize * 1000.0f)/(float)OR_PAGESIZE) / m_serviceTime;
	m_requestInterval = (uint64_t) (1/requestPerMicroSecond );
	m_totalRequests = (uint64_t) ((float)m_fileSize * 1000.0f / (float)OR_PAGESIZE);

}

Migration::~Migration() {
}

void Migration::SetSendTTReadCallback(sendTTReadCallback scb){
	m_sendTTReadCallback = scb;
}

void Migration::SetSendTTWriteCallback(sendTTWriteCallback scb){
	m_sendTTWriteCallback = scb;
}

void Migration::SetSendTTFinishCallback(sendTTFinishCallback scb){
	m_sendTTFinishCallback = scb;
}

void Migration::SendReadRequest(){
	if(!m_finished){
		m_sendTTReadCallback(m_dataObjectId);
		m_SendEvent = Simulator::Schedule(MicroSeconds (m_requestInterval), &Migration::SendReadRequest, this);
	}
}

void Migration::GetReadResponse(){
	if(!m_finished){
		m_sendTTWriteCallback(m_dataObjectId, m_writeTarget);
	}
}

void Migration::GetWriteResponse(){
	m_currentRequests++;
	if(m_currentRequests == m_totalRequests){
		m_finished = true;
		m_sendTTFinishCallback(m_dataObjectId, m_readTarget, m_writeTarget, m_fileSize);
		CancelSendEvent();
	}
}

void Migration::CancelSendEvent(){
	Simulator::Cancel(m_SendEvent);
}


}
