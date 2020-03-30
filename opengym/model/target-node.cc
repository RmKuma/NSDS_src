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
 * Sender&receiver Application implementation for gcc ns3 module.
 *
 * @author Sugi Lee
 * @reference https://chromium.googlesource.com/external/webrtc
 *            (Commit Hash ID of WebRTC's git which we refer to is `3613fef7a2f0cae1b1e6352b690979d430626056`) 
 */

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/node.h"
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/socket-factory.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "target-node.h"
#include "opengym-parameter.h"

NS_LOG_COMPONENT_DEFINE ("TargetNode");


namespace ns3 {


TargetNode::TargetNode ()
{
	m_totalRx = 0;
	m_requestDelay_a = 2.6;
	m_requestDelay_b = 72;
	m_totalRxPackets = 0;
	m_totalTxPackets = 0;
	m_requestSocket = 0;
	m_resultSocket = 0;
	m_resultSocketConnected = false;
	m_requestSocketConnected = false;
	m_targetWorked = false;
	m_finished = false;
}

TargetNode::~TargetNode () {}

uint64_t TargetNode::GetTotalRx () const{
	return m_totalRx;
}

uint64_t TargetNode::GetTotalTxPackets () const{
	return m_totalTxPackets;

}
uint64_t TargetNode::GetTotalRxPackets () const{
	return m_totalRxPackets;
}

void TargetNode::SetTier(uint16_t tier){
	m_tier = tier;

	//Tier set delay
	m_requestDelay_a = (2 + (m_tier * 1)) * DELAY_UP;
	m_requestDelay_b = (70 + (m_tier * 20)) * DELAY_UP;
}

void TargetNode::StartApplication (){}

void TargetNode::StopApplication ()
{
	m_finished = true;
	Simulator::Cancel (m_getRequestEvent);

	std::cout << "TargetNode close" << std::endl;
}

void TargetNode::HandleRead (uint16_t userId, uint64_t timestamp, int16_t type){
	if(m_rxBuffer.size() <= 128){
		m_rxBuffer.push_back(std::make_tuple(userId, timestamp, type));	
		GetNextRequestFromBuffer();
	}
}

void TargetNode::GetNextRequestFromBuffer (){
	while(!m_finished){
		if(m_submissionQueue.size() >= QUEUE_DEPTH){
			break;
		}else{
			if(m_rxBuffer.size() > 0){
				m_submissionQueue.push_back(std::make_tuple(std::get<0>(m_rxBuffer.front()), std::get<1>(m_rxBuffer.front()), std::get<2>(m_rxBuffer.front())));
				m_rxBuffer.pop_front();
				m_getRequestEvent = Simulator::Schedule (MicroSeconds(m_requestDelay_a * (m_submissionQueue.size()-1) + m_requestDelay_b), &TargetNode::SendResponsePacket, this);
			}else{
				break;
			}
		}
	}
}

void TargetNode::SendResponsePacket(){
	uint16_t userId = std::get<0>(m_submissionQueue.front());
	uint64_t timestamp = std::get<1>(m_submissionQueue.front());
	int16_t type = std::get<2>(m_submissionQueue.front());

	m_submissionQueue.pop_front();
	m_requestCallback(userId, timestamp, type);	
	GetNextRequestFromBuffer();
}

}
