#ifndef MIGRATION_H
#define MIGRATION_H

#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "opengym-parameter.h"

#include <map>

namespace ns3 {

class Migration : public Object
{
public:
	Migration ();
	Migration (uint16_t dataObjectId, uint16_t readTarget, uint16_t writeTarget, uint64_t serviceTime, uint64_t filesize);
	virtual ~Migration ();
	
	bool IsFinished(){
		return m_finished;
	}
	
	uint16_t GetDataObjectId(){
		return m_dataObjectId;
	};

	uint64_t GetServiceTime(){
		return m_serviceTime;
	};

	uint64_t GetTotalRequests(){
		return m_totalRequests;
	};

	uint64_t GetTargetThroughput(){
		return (float)m_fileSize/((float)(m_serviceTime)/1000000.0);
	}
	
	typedef Callback<void, uint16_t> sendTTReadCallback;
	void SetSendTTReadCallback(sendTTReadCallback scb);	
	
	typedef Callback<void, uint16_t, uint16_t> sendTTWriteCallback;
	void SetSendTTWriteCallback(sendTTWriteCallback scb);	
	
	typedef Callback<void, uint16_t, uint16_t, uint16_t, uint64_t> sendTTFinishCallback;
	void SetSendTTFinishCallback(sendTTFinishCallback scb);	

	void SendReadRequest();
	void GetReadResponse();
	void GetWriteResponse();

	void CancelSendEvent();

private:
	bool m_finished;				// 
	uint16_t m_readTarget;			// Sourced Target
	uint16_t m_writeTarget; 		// Destination Target
	uint16_t m_dataObjectId;
	uint64_t m_serviceTime;
	uint64_t m_requestInterval;   
	uint64_t m_totalRequests;		// Total amount of requests to send
	uint64_t m_currentRequests;		// amount of requests sent before gathering
	uint64_t m_fileSize;

	sendTTReadCallback m_sendTTReadCallback;
	sendTTFinishCallback m_sendTTFinishCallback;
	sendTTWriteCallback m_sendTTWriteCallback;
	EventId m_SendEvent;

};

}

#endif /* NVMEOF_FLOW_H */

