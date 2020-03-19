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

#ifndef HOST_NODE_H
#define HOST_NODE_H

#include "ns3/address.h"
#include "ns3/socket.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "nvme-header.h"
#include "nvme-tag.h"
#include "zmq.hpp"
#include "ns3/json.h"
#include "target-table.h"
#include "opengym-parameter.h"
#include "user.h"
#include "target-node.h"

#include <string>
#include <deque>
#include <map>
#include <memory>
#include <random>
#include <cmath>
#include <algorithm>

namespace ns3 {

class HostNode: public Application
{
public:

    HostNode ();
    virtual ~HostNode ();

 	uint64_t GetTotalTx () const;
	uint64_t GetTotalTxPackets () const;
	uint64_t GetTotalRx () const;
	
	//void AddFlow(uint16_t tier, uint32_t initRate);
	void AddTarget(uint16_t tier, Ipv4Address targetIp, uint16_t targetPort);

	void SetGymPort(uint16_t gymPort);
	void SendReset();
	void CreateUser(uint16_t userId);
	void PrintResult();

private:
    virtual void StartApplication ();
    virtual void StopApplication ();

    
	/*Send Methods*/
	//void EnqueueReadRequest(uint32_t flowNumber);
	void HeuristicAction_knapsack(uint64_t obsArray[][6]);
    void SendReadRequestPacket (uint16_t userId);
   	void SendTargetFromBuffer(uint16_t target);

    /*Recv Methods*/
	void HandleRead (Ptr<Socket> socket);
    void HandleAccept(Ptr<Socket> socket, const Address& from);

	void Observe();
	void Migration(uint16_t dataObject, uint16_t destination_target);
	//void Action();
	void PopularityChangeStart();
	void PopularityChangeSmooth(uint16_t smoothing);
	//void SendRateChange();

	void SendObs(uint64_t obsArray[][6], int32_t rew);
	//void GetAction();
	
private:
	uint16_t m_numTargets;
	uint32_t m_numFlows;
	uint64_t m_totalTx, m_totalTxPackets;
	uint64_t m_totalRx;

	TargetTable m_targetTable{};

	std::map<uint32_t, Ptr<User>> m_users;
	std::map<uint16_t, Ptr<Socket>> m_sockets;

	std::map<uint16_t, float> 	m_changePopularity;

	std::map<uint32_t, EventId> m_flowEvents;	// Event

	typedef std::pair<Ipv4Address, uint16_t> targetAddress;
	std::map<uint16_t, targetAddress> m_targets;
	std::map<uint16_t, std::deque<Ptr<Packet>>> m_targetSendBuffer;
	std::map<uint16_t, bool> m_targetSendEvent;
	std::map<uint16_t, bool>  m_connecteds;
	std::map<uint64_t, uint16_t> m_recvChecker;

	EventId m_readRequestEnqueueEvent;			// Event
	EventId m_SendRateChangeEvent;				// Event
	EventId m_PopularityChangeEvent;			// Event
	EventId m_PopularityChangeSmoothEvent;		// Event
	EventId m_ObsEvent;							// Event

	Ptr<Socket> m_resultSocket;
	std::list<Ptr<Socket>> m_resultSocketList;

	std::default_random_engine gen;
	std::normal_distribution<double> serviceTimeDistri{SERVICETIMEAVG, SERVICETIMESTD}; 
	std::normal_distribution<double> targetDelayDistri{1800, 200.0};
	std::normal_distribution<double> popularityDistri{0, 0.01};

	zmq::context_t _context{1};
	zmq::socket_t _socket{_context, ZMQ_REP};
	uint16_t m_gymPort;

	uint32_t m_logCount;
	uint32_t m_epiCount;
};

}

#endif /* HOST_NODE_H */
