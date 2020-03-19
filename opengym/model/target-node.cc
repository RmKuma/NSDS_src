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
	m_requestDelay_a = 100 + (m_tier * 100);
	m_requestDelay_b = 500 + (m_tier * 500);
}

void TargetNode::StartApplication ()
{
	//Create Sockets
	
	if(!m_requestSocketConnected){
		m_requestSocket = Socket::CreateSocket( GetNode (), TcpSocketFactory::GetTypeId());
		auto local = InetSocketAddress{Ipv4Address::GetAny(), REQUESTPORT};
		if( m_requestSocket->Bind (local) == -1){
			std::cout << "Bind Failed" << std::endl;
		}
		m_requestSocket->Listen();
		m_requestSocket->ShutdownSend();
		m_requestSocketConnected = true;
	}

	m_requestSocket->SetRecvCallback (MakeCallback (&TargetNode::HandleRead, this));
	m_requestSocket->SetAcceptCallback (
	  MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
	  MakeCallback (&TargetNode::HandleAccept, this));

}

void TargetNode::StopApplication ()
{
	while(!m_requestSocketList.empty()){
		Ptr<Socket> acceptedSocket = m_requestSocketList.front();
		m_requestSocketList.pop_front();
		acceptedSocket->Close();
	}

	if (m_requestSocket){
		m_requestSocket->Close();
		m_requestSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket>> ());
	}
	
	if (m_resultSocket){
		m_resultSocket->Close();
	}
	
	m_finished = true;
	Simulator::Cancel (m_getRequestEvent);

	std::cout << "TargetNode close" << std::endl;
	// Need to cancel al3l event such as ...
}

void TargetNode::HandleRead (Ptr<Socket> socket){
	GetNextRequestFromBuffer();
}

void TargetNode::GetNextRequestFromBuffer (){
	//std::cout << "QueueSize : " << m_submissionQueue.size() << std::endl;
	while(!m_finished){
		if(m_submissionQueue.size() >= QUEUE_DEPTH){
			break;
		}else{
			Ptr<Socket> socket = m_requestSocketList.front();
			Ptr<Packet> packet;
			Address from;
			if(packet = socket->RecvFrom(from)){
				if(packet->GetSize () == 0)	return;
				m_totalRx += packet->GetSize();

				NVMeHeader header{};
				packet->RemoveHeader(header);
				m_submissionQueue.push_back(std::make_pair(header.GetUserId(), header.GetTimestamp()));
				m_getRequestEvent = Simulator::Schedule (MicroSeconds(m_requestDelay_a * (m_submissionQueue.size()-1) + m_requestDelay_b), &TargetNode::SendReadResultPacket, this);
			}else{
				break;
			}
		}
	}
}

void TargetNode::SendReadResultPacket(){
	NVMeHeader testHeader{};
	auto segmentSize = 120;
	auto bytesToSend = PAGESIZE;
	
	uint16_t userId = m_submissionQueue.front().first;
	uint64_t timestamp = m_submissionQueue.front().second;
	m_submissionQueue.pop_front();

	if(m_resultSocketConnected){
		NVMeHeader header{};
		header.SetUserId(userId);
		header.SetTimestamp(timestamp);
		Ptr<Packet> packet;
		packet = Create<Packet> (segmentSize-header.GetSerializedSize());
		packet->AddHeader (header);
		int actual = m_resultSocket->Send(packet);
		m_totalTxPackets++;
	}
	GetNextRequestFromBuffer();
}

void TargetNode::HandleAccept (Ptr<Socket> socket, const Address& from){
	if(!m_resultSocketConnected){
		std::cout << "new Socket" << std::endl;
		m_resultSocket = Socket::CreateSocket( GetNode (), TcpSocketFactory::GetTypeId());
		Ipv4Address hostAddr = InetSocketAddress::ConvertFrom(from).GetIpv4();
		m_resultSocket ->Connect(InetSocketAddress(hostAddr, RESULTPORT));
		m_resultSocketConnected = true;
	}
	socket->SetRecvCallback (MakeCallback (&TargetNode::HandleRead, this));
	m_requestSocketList.push_back(socket);
}


}
