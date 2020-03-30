/******************************************************************************
 * Copyright 2016-2017 Cisco Systems, Inc.                                    *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License");            *
 * you may not use this file except in compliance with the License.           *
 *                                                                            *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 *     http://www.apache.org/licenses/LICENSE-2.0                             *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 ******************************************************************************/

/**
 * @file
 * Sender&receiver Application interface for gcc ns3 module.
 *
 * @author Sugi Lee
 * @reference https://chromium.googlesource.com/external/webrtc
 *            (Commit Hash ID of WebRTC's git which we refer to is `3613fef7a2f0cae1b1e6352b690979d430626056`) 
 */

#ifndef TARGET_NODE_H
#define TARGET_NODE_H

#include "ns3/address.h"
#include "ns3/socket.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "flow.h"
#include "nvme-header.h"
#include "nvme-tag.h"

#include <deque>
#include <map>
#include <memory>
#include <tuple>

namespace ns3 {

class TargetNode: public Application
{
public:

    TargetNode ();
    virtual ~TargetNode ();

	uint64_t GetTotalRx () const;
	uint64_t GetTotalRxPackets () const;
	uint64_t GetTotalTxPackets () const;
	void SetTier (uint16_t tier);

	typedef Callback<void, uint16_t, uint64_t, int16_t> requestCallback;
	requestCallback m_requestCallback;
	void SetRequestCallback(requestCallback reqcb){
		m_requestCallback = reqcb;
	};
	
	void HandleRead (uint16_t userId, uint64_t timestamp, int16_t type);
private:
    virtual void StartApplication ();
    virtual void StopApplication ();

    /*Send Methods*/
	//void EnqueueReadRequest(uint32_t flowNumber);
    //void SendReadRequestPacket ();
   
	void HandleAccept (Ptr<Socket> socket, const Address& from);
	void GetNextRequestFromBuffer ();
	void SendResponsePacket ();


private:
	bool m_finished;
	uint16_t m_tier;
	uint64_t m_requestDelay_a, m_requestDelay_b;
	uint64_t m_totalRx, m_totalRxPackets, m_totalTxPackets;

	Ptr<Socket> m_requestSocket;
	std::list<Ptr<Socket>> m_requestSocketList;
	Ptr<Socket> m_resultSocket;
	std::deque<std::tuple<uint16_t, uint64_t, int16_t>> m_submissionQueue;


	bool  m_requestSocketConnected;
	bool  m_resultSocketConnected;
	bool  m_targetWorked;

	EventId m_nextRequestEvent;
	EventId m_checkNextRequestEvent;
	EventId m_getRequestEvent;
	
	std::map<uint16_t, uint64_t> packetCounter;
	std::deque<std::tuple<uint16_t, uint64_t, int16_t>> m_rxBuffer;
};

}

#endif /* TARGET_NODE_H */
