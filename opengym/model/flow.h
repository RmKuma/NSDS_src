#ifndef NVMEOF_FLOW_H
#define NVMEOF_FLOW_H

#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"


namespace ns3 {

class Flow : public Object
{
public:
	Flow ();
	Flow (uint32_t flowNumber);
	virtual ~Flow ();
	
	void SetRate(uint32_t sendRate);	//set 
	uint32_t GetFlowNumber();
	void SetTier(uint16_t tier);
	uint16_t GetTier();
	uint32_t GetInterval();
	void SendReadRequestPacket();
	
	typedef Callback<void, uint32_t> sendCallback;
	sendCallback m_sendCallback;
	void SetSendCallback(sendCallback scb);

	uint32_t GetRate();
	uint32_t GetCurrentGoodput();
	void SetTargetDelay(uint32_t delay);
	uint32_t GetTargetDelay();
	uint32_t GetCurrentDelay();
	uint32_t GetCurrentPackets();

	void AfterGathering();
	void AfterGetPacket(uint32_t size, uint32_t delay);
	
	EventId GetSendEvent();
	void CancelSendEvent();

private:
	// Result packet size : 16KB
	// Request packet size : No consider

	uint16_t m_tier;
	uint32_t m_flowNumber;
	float m_sendRate;
	uint32_t m_intervalMicroSeconds;
	
	//uint32_t m_targetSendingRate;
	uint32_t m_currentGoodput;
	uint32_t m_targetDelay;
	uint32_t m_currentDelay;
	uint32_t m_currentPackets;

	EventId m_SendEvent;

};

}

#endif /* NVMEOF_FLOW_H */

