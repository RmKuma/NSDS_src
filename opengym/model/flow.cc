#include "flow.h"
#include "opengym-parameter.h"

namespace ns3 {

Flow::Flow ()
: m_sendRate {0}
, m_flowNumber {0}
{}

Flow::Flow (uint32_t flowNumber)
: m_sendRate {0}
, m_flowNumber {flowNumber}
, m_currentGoodput{0}
, m_targetDelay{INITDELAY}
, m_currentDelay{0}
, m_currentPackets{0}
{}

Flow::~Flow () {}

void Flow::SetRate(uint32_t sendRate){
	m_sendRate = sendRate;
	
	// Size of Result Packet is fixed as 4KB
	// sending interval is sendRate/(4096 * 8)
	
	auto packetpersecond= m_sendRate /(4096 * 8);
	
	m_intervalMicroSeconds = 1000000 / packetpersecond;
}

uint32_t Flow::GetFlowNumber(){
	return m_flowNumber;
}

uint32_t Flow::GetRate(){
	return m_sendRate;
}


uint32_t Flow::GetCurrentGoodput(){
	return m_currentGoodput*8;
}

void Flow::SetTargetDelay(uint32_t delay){
	m_targetDelay = delay;
}

uint32_t Flow::GetTargetDelay(){
	return m_targetDelay;
}


uint32_t Flow::GetCurrentDelay(){
	return m_currentDelay;
}

uint32_t Flow::GetCurrentPackets(){
	return m_currentPackets;
}

void Flow::AfterGathering(){
	m_currentGoodput = 0;
	m_currentDelay = 0;
	m_currentPackets = 0;
}

void Flow::AfterGetPacket(uint32_t size, uint32_t delay){
	m_currentGoodput += size;
	m_currentDelay += delay;
	m_currentPackets ++;
}



void Flow::SetTier(uint16_t tier){
	m_tier = tier;
}

uint16_t Flow::GetTier(){
	return m_tier;
}

uint32_t Flow::GetInterval(){
	return m_intervalMicroSeconds;
}

void Flow::SetSendCallback(sendCallback scb){
	m_sendCallback = scb;
}

void Flow::SendReadRequestPacket(){
	m_sendCallback(m_flowNumber);
	m_SendEvent = Simulator::Schedule (MicroSeconds(m_intervalMicroSeconds), &Flow::SendReadRequestPacket, this);
}

EventId Flow::GetSendEvent(){
	return m_SendEvent;
}

void Flow::CancelSendEvent(){
	Simulator::Cancel (m_SendEvent);
}



}
