#ifndef USER_H
#define USER_H

#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "nvme-header.h"
#include "opengym-parameter.h"

#include <map>

namespace ns3 {

class User : public Object
{
public:
	User ();
	User (uint16_t userId, uint16_t dataObjectId, uint64_t serviceTime, uint64_t targetDelay);
	virtual ~User ();
	
	bool IsFinished(){
		return m_finished;
	}

	uint16_t GetUserId(){
		return m_userId;
	};
	
	uint16_t GetDataObjectId(){
		return m_dataObjectId;
	};

	uint64_t GetServiceTime(){
		return m_serviceTime;
	};

	uint64_t GetTotalRequests(){
		return m_totalRequests;
	};

	uint64_t GetTargetDelay(){
		return m_targetDelay;
	};

	uint64_t GetCurrentDelay(){
		if(m_currentPackets > 0)
			return m_currentDelay/m_currentPackets;
		else
			return 999999;
	}

	uint64_t GetCurrentResults(){
		return m_currentResults;
	}

	uint64_t GetCurrentGoodput(){
		return m_currentGoodput;
	}

	uint64_t GetGeneratedTime(){
		return m_generatedTime;
	}

	uint64_t GetEndTime(){
		return m_endTime;
	}


	uint64_t GetCurrentPackets(){
		return m_currentPackets;
	}

	void SetTargetDelay(uint64_t delay){
		m_targetDelay = delay;
	};

	uint64_t GetTargetThroughput(){
		return ((float)FILESIZE*1000.0f)/((float)(m_serviceTime)/1000000.0);
	}
	
	void PauseSending(){
		if(m_currentSending){
			Simulator::Cancel(m_SendEvent);
			m_currentSending = false;
		}
	}

	void ResumeSending(){
		if(!m_currentSending){
			SendRequest();
			m_currentSending = true;
		}
	}
	
	typedef Callback<void, int16_t, uint16_t> sendCallback;
	void SetSendCallback(sendCallback scb);	
	
	void SendRequest();
	void AfterGathering();
	void AfterGetPacket(uint64_t size, uint64_t delay);
	void CancelSendEvent();

	std::map<uint16_t, uint64_t> sendCounter;
	std::map<uint16_t, uint64_t> receiveCounter;

private:
	bool m_finished;
	bool m_currentSending;
	uint64_t m_generatedTime;
	uint64_t m_endTime;
	uint16_t m_userId;
	uint16_t m_dataObjectId;
	uint64_t m_serviceTime;
	uint64_t m_requestInterval;   
	uint64_t m_totalRequests;		// Total amount of requests to send
	uint64_t m_currentRequests;		// amount of requests sent before gathering
	uint64_t m_currentResults;		// amount of results received before gathering
	uint64_t m_targetDelay;		
	uint64_t m_currentGoodput;
	uint64_t m_currentDelay;
	uint64_t m_currentPackets;
	sendCallback m_sendCallback;
	EventId m_SendEvent;

};

}

#endif /* NVMEOF_FLOW_H */

