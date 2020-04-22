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
#include "ns3/address.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/socket-factory.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/default-deleter.h"

#include "host-node.h"

NS_LOG_COMPONENT_DEFINE ("HostNode");

namespace ns3 {


HostNode::HostNode ()
: m_numTargets{0}
, m_numFlows{0}
, m_totalTx{0}
, m_totalTxPackets{0}
, m_totalRx{0}
, m_logCount{0}
, m_epiCount{0}
, m_gymPort{5555}
, m_totalReward{0}
, m_totalSteps{0}
{}

HostNode::~HostNode () {}

uint64_t HostNode::GetTotalTx() const{
	return m_totalTx;
}

uint64_t HostNode::GetTotalTxPackets() const{
	return m_totalTxPackets;
}

uint64_t HostNode::GetTotalRx() const{
	return m_totalRx;
}

void HostNode::SetGymPort(uint16_t gymPort){
	m_gymPort = gymPort;
}

void HostNode::AddTarget(uint16_t target, Ipv4Address targetIp, uint16_t targetPort, Ptr<TargetNode> targetNode){
	m_targetTable.AddTarget(target, targetIp, targetPort);
	m_targetPtrs[target] = targetNode;
}

void HostNode::StartApplication ()
{
	//Bind Socket For Gym Communication
	_socket.bind("tcp://*:" + std::to_string(m_gymPort));
	std::cout << "Bind Socket" << std::endl;

	//set callback for targetTable
	m_targetTable.SetSendHostReadCallback(MakeCallback(&HostNode::SendReadRequestPacket, this));
	m_targetTable.SetSendHostWriteCallback(MakeCallback(&HostNode::SendWriteRequestPacket,this));

	//Create Sockets between host and each target
	for(uint16_t target = 0; target < m_targetTable.GetTotalTargets(); target++){
		/*
		if(!m_sockets[target]){
			m_sockets[target] = Socket::CreateSocket( GetNode (), TcpSocketFactory::GetTypeId());
		}
		NS_ASSERT(m_sockets[target]);
		m_sockets[target]->Connect(InetSocketAddress(m_targetTable.targetMap[target]->GetIp(), m_targetTable.targetMap[target]->GetPort()));
		*/

		m_targetSendEvent[target] = false;
		m_targetPtrs[target]->SetRequestCallback(MakeCallback(&HostNode::HandleRead, this));
	}

	// Create Data Objects
	m_targetTable.AddDataObjects(DATAS);
	for(int user = 0; user < USERS; user++){
		CreateUser(user);
		m_users[user]->SendRequest();
	}

	// Observation event scheduling
	m_ObsEvent = Simulator::Schedule(MilliSeconds (OBS_INTERVAL), &HostNode::Observe, this);
	
	// Changing Popularity of Data objects event scheduling
	m_PopularityChangeEvent = Simulator::Schedule(MilliSeconds (PC_INTERVAL), &HostNode::PopularityChangeStart, this);

	std::cout << "Host Node Ready to Start" << std::endl;
}

void HostNode::CreateUser(uint16_t userId){	
	// Select Data User want to receive
	uint16_t dataObjectId = m_targetTable.SelectData();
	uint64_t serviceTime = std::min(std::max((uint64_t)std::round(serviceTimeDistri(gen)), (uint64_t)MINSERVICETIME),(uint64_t)MAXSERVICETIME);
	uint64_t targetDelay = std::min(std::max((uint64_t)std::round(targetDelayDistri(gen)), (uint64_t)1000), (uint64_t)2600);
	Ptr<User> user = new User(userId, dataObjectId, serviceTime, targetDelay);
	// Create socket
	uint16_t target = m_targetTable.GetTargetOfDataObject(dataObjectId);
	user->SetSendCallback(MakeCallback(&HostNode::SendReadRequestPacket,this));
	//std::cout << "User " << userId << ", dataObjectId : " <<  dataObjectId <<  ", data position " << target << ", Total Requests : " << user->GetTotalRequests() << ", serviceTime : " << serviceTime << ", targetDelay : " << targetDelay << std::endl;
	m_users[userId] = user;
}


void HostNode::StopApplication ()
{
	// Need to cancel all events.
	for(uint16_t user = 0; user < USERS; user++){
		m_users[user]->CancelSendEvent();
	}
	//	Simulator::Cancel (m_readRequestEnqueueEvent);
	Simulator::Cancel (m_PopularityChangeSmoothEvent);
	Simulator::Cancel (m_PopularityChangeEvent);
	Simulator::Cancel (m_ObsEvent);

	// Close Socket connected with python agent
	SendReset();
	_socket.close();
	
	std::cout << "stop host application" << std::endl;
}

void HostNode::SendReadRequestPacket (int16_t type, uint16_t Id)
{
	uint16_t dataObjectId;
	if(type == 0){
		// Send Read Request by User
		dataObjectId = m_users[Id]->GetDataObjectId();
	}else if(type == 1){
		// Send Read Request for Migration
		// Migration's read request use DataId as Id
		dataObjectId = Id;
	}
	
	uint16_t target = m_targetTable.GetTargetOfDataObject(dataObjectId);
    uint64_t  nowUs = Simulator::Now ().GetMicroSeconds ();
   	NS_ASSERT (nowUs >= 0); 

	Simulator::ScheduleNow(&TargetNode::HandleRead,  m_targetPtrs[target], Id, nowUs, type);	
}

void HostNode::SendWriteRequestPacket (uint16_t dataId, uint16_t targetId){
	uint16_t dataObjectId = dataId;
	uint16_t target = targetId;
    uint64_t  nowUs = Simulator::Now ().GetMicroSeconds ();
   	NS_ASSERT (nowUs >= 0); 

	Simulator::ScheduleNow(&TargetNode::HandleRead,  m_targetPtrs[target], dataObjectId, nowUs, 2);	
}


void HostNode::HandleRead (uint16_t userId, uint64_t timestamp, int16_t type){
    uint64_t nowUs = Simulator::Now ().GetMicroSeconds ();
	if(type == 0){
		// Read response for User
		m_users[userId]->AfterGetPacket(OR_PAGESIZE, (uint32_t)(nowUs-timestamp + 20));
	}else if(type == 1){
		// Read response for Migration
		m_targetTable.GetReadResponsePacket(userId);		
	}else if(type == 2){
		// Write response for Migration
		m_targetTable.GetWriteResponsePacket(userId);
	}
}

void HostNode::Migration(uint16_t dataObject, uint16_t destination_target){
	uint16_t currentTarget = m_targetTable.GetTargetOfDataObject(dataObject);
	
	if(currentTarget == destination_target) return;

	if(m_targetTable.targetMap[destination_target]->CheckRemainingSpace(FILESIZE) && !m_targetTable.dataObjectMap[dataObject]->GetNowMigration()){
		m_targetTable.MigrationStart(dataObject, destination_target, FILESIZE);
	}
}

void HostNode::Observe (){
	m_logCount = (m_logCount+1)%1;
	m_epiCount++;

	uint64_t obs [USERS][5] = {0};
	uint64_t datas[DATAS][6] = {0};
	uint64_t mig [DATAS][6] ={0};
	uint64_t x = 0, y = 0;
	double successedUser = 0;

	//std::cout << "===============================================" << std::endl;
	for(uint16_t data = 0; data < DATAS; data++){
		uint64_t check =0;
		for(uint16_t user=0;user<USERS;user++)
			if(m_users[user]->GetDataObjectId() == data) check++;
		datas[data][0] = check;
		datas[data][1] = m_targetTable.GetTargetOfDataObject(data);
		
		//MIN Target Delay
		uint64_t min_tdelay = 999999;
		for(uint16_t user=0;user<USERS;user++)
			if(m_users[user]->GetDataObjectId() == data && m_users[user]->GetTargetDelay() < min_tdelay) min_tdelay = m_users[user]->GetTargetDelay();
		datas[data][2] = min_tdelay;
		
		//MIN Delay
		uint64_t min_delay = 999999;
		for(uint16_t user=0;user<USERS;user++)
			if(m_users[user]->GetDataObjectId() == data && m_users[user]->GetCurrentDelay() < min_delay) min_delay = m_users[user]->GetCurrentDelay();
		datas[data][3] = min_delay;

		//AVG target throughput
		uint64_t targetThroughputSum = 0;
		for(uint16_t user=0;user<USERS;user++)
			if(m_users[user]->GetDataObjectId() == data) targetThroughputSum += m_users[user]->GetTargetThroughput();
		if(datas[data][0]) 	datas[data][4] = targetThroughputSum/datas[data][0];

		//AVG throughput
		uint64_t throughputSum = 0;
		for(uint16_t user=0;user<USERS;user++)
			if(m_users[user]->GetDataObjectId() == data) throughputSum += m_users[user]->GetCurrentGoodput();
		if(datas[data][0])	datas[data][5] = throughputSum/datas[data][0];
		std::cout << "Data " << data << " N : " << datas[data][0] <<
										" POS : " << datas[data][1] << 
										" TD : " << datas[data][2] <<
										" D : " << datas[data][3] <<
										" TT : " << datas[data][4] <<
										" T : " << datas[data][5] <<
										" P : " << m_targetTable.dataObjectMap[data]->GetPopularity() << 
										std::endl;
	}
	
	//std::cout << "===============================================" << std::endl <<  "================== users ======================" << std::endl;

	float reward= 0;

	for(uint16_t user = 0; user < USERS; user++){
		/*std::cout << " User " << user << 
		             " DATA : " << m_users[user]->GetDataObjectId() <<
					 " SER : " << m_users[user]->GetServiceTime() << 
					 " TD : " << m_users[user]->GetTargetDelay() <<
					 " D : " << m_users[user]->GetCurrentDelay() <<
					 " TT : " << m_users[user]->GetTargetThroughput() << 
					 " T : " << m_users[user]->GetCurrentGoodput() << std::endl;
		*/
		reward += std::min((float)1.0, (float)m_users[user]->GetTargetDelay()/(float)m_users[user]->GetCurrentDelay()) * 0.5 + std::min((float)1.0,(float)m_users[user]->GetCurrentGoodput()/(float)m_users[user]->GetTargetThroughput()) * 0.5;
		
		obs[user][0] = m_users[user]->GetDataObjectId();
		obs[user][1] = m_users[user]->GetCurrentDelay();
		obs[user][2] = m_users[user]->GetCurrentPackets();
		obs[user][3] = m_users[user]->GetGeneratedTime();
		obs[user][4] = m_users[user]->GetEndTime();

		m_users[user]->AfterGathering();
	
		if(m_users[user]->IsFinished()){
			m_users[user]->Dispose();
			CreateUser(user);
			m_users[user]->SendRequest();
		}	
	}
	for(uint16_t data = 0; data < DATAS; data++){
		if(m_targetTable.dataObjectMap[data]->GetNowMigration()){
			mig[data][0] = m_targetTable.migrationProgress[data]->GetGeneratedTime();
			mig[data][1] = m_targetTable.migrationProgress[data]->GetEndTime();
			mig[data][2] = m_targetTable.migrationProgress[data]->GetReadTarget();
			mig[data][3] = m_targetTable.migrationProgress[data]->GetWriteTarget();
			mig[data][4] = m_targetTable.migrationProgress[data]->GetCurrentReadPackets();
			mig[data][5] = m_targetTable.migrationProgress[data]->GetCurrentWritePackets();
				
			m_targetTable.migrationProgress[data]->AfterGathering();

			if(m_targetTable.migrationProgress[data]->IsFinished())
				m_targetTable.MigrationDelete(data);
		}
	}

	reward /= USERS;
	std::cout << "=============reward : " << reward << "=================" << std::endl;
	
	m_totalSteps++;
	m_totalReward += reward;

	//HeuristicAction_knapsack(datas);
	SendObs(datas, obs, mig, reward);	
	m_ObsEvent = Simulator::Schedule(MilliSeconds (OBS_INTERVAL), &HostNode::Observe, this);

}

void HostNode::HeuristicAction_knapsack(uint64_t datas[][6]){
	std::deque<uint16_t> index;
	uint64_t totalThroughput = 0;

	while(index.size() != DATAS){
		uint64_t max = 0;
		uint16_t idx = 0;
		for(uint16_t data = 0; data < DATAS; data++){
			if(max <= (datas[data][4] * datas[data][0]) && std::find(index.begin(), index.end(), data) == index.end()){
				max = datas[data][4] * datas[data][0];
				idx = data;
			}
		}
		index.push_back(idx);	
	}
	
	for(uint16_t data = 0; data < DATAS; data++){
		totalThroughput += (datas[data][4] * datas[data][0]);
	}
/*	
	for(int i = 0; i < index.size(); i++)
		std::cout << index[i];
	std::cout << std::endl;
*/

	uint16_t target = 0;
	uint64_t throughputCheck = 0;
	while(index.size() != 0){
		uint16_t idx = index.front();
		Migration(idx, target);
		throughputCheck += (datas[idx][4] * datas[idx][0]);
		if(throughputCheck >= totalThroughput/TARGETS ){
			throughputCheck = 0;
			target++;
		}
		index.pop_front();
	}
}

void HostNode::PopularityChangeStart(){
	for(uint16_t data = 0; data < DATAS; data++)
		m_changePopularity[data] = popularityDistri(gen);
	m_PopularityChangeEvent = Simulator::Schedule(MilliSeconds (PC_INTERVAL), &HostNode::PopularityChangeStart, this);
	PopularityChangeSmooth(SMOOTHING);
}

void HostNode::PopularityChangeSmooth(uint16_t smoothing){
	if(smoothing != 0){
		for(uint16_t data = 0; data < DATAS; data++){
			float newPopularity = m_targetTable.dataObjectMap[data]->GetPopularity() + m_changePopularity[data];
			m_changePopularity[data] /= 2;
			m_targetTable.dataObjectMap[data]->SetPopularity(std::max(newPopularity,(float)0.0));		
		}
		m_PopularityChangeSmoothEvent = Simulator::Schedule(MilliSeconds (PC_INTERVAL/5), &HostNode::PopularityChangeSmooth, this, smoothing-1);
	}
}

void HostNode::PrintResult(){
	std::cout << "User Send Packet" << std::endl;
	for(uint16_t user = 0 ; user < USERS; user++){
		std::cout << "user " << user << std::endl;
		for(uint16_t data = 0; data < DATAS; data++){
			if(m_users[user]->sendCounter.find(data) != m_users[user]->sendCounter.end() &&
			   m_users[user]->receiveCounter.find(data) != m_users[user]->receiveCounter.end()){
				std::cout << "Send : " << m_users[user]->sendCounter[data] 
						<< "Receive : " << m_users[user]->receiveCounter[data] << std::endl;
				
			}		
		}
	}
}

/* ======================================= OPEN AI GYM COMMUNICATION ======================================= */

void HostNode::SendReset(){
	std::cout << " Send Reset " << std::endl;

	//Read actionJson String
	zmq::message_t action;
	_socket.recv(&action);
	std::cout << "Get reset";
	std::string actionStr = std::string(static_cast<char*>(action.data()), action.size());
	
	Json::Value obsJson;
	obsJson["obs"] = 0;
	obsJson["mig"] = 0;
	obsJson["user"] = 0;
	obsJson["reward"] = 0;
	obsJson["done"] = 1;
	Json::FastWriter writer;
	std::string str = writer.write(obsJson);

	zmq::message_t observation(str.size());
	memcpy(observation.data(), str.data(), str.size());
	_socket.send(observation);
	std::cout << "Send reset" << std::endl;
}

void HostNode::SendObs(uint64_t obsArray[][6], uint64_t userArray[][5], uint64_t migArray[][6], float reward){

	//Read actionJson String
	zmq::message_t action;
	_socket.recv(&action);
	std::cout << "Get Action";
	std::string actionStr = std::string(static_cast<char*>(action.data()), action.size());

	//Parse action
	Json::Reader reader;
	Json::Value actionJson;
	if(actionStr != "reset"){
		reader.parse(actionStr, actionJson);
		if(!m_logCount){
			
			std::cout << "Action is : " ;
			for(uint16_t data = 0; data < DATAS; data++){
				uint16_t changedTier = (uint16_t)(actionJson["action"][data].asInt());
				Migration(data, changedTier);
				std::cout << actionJson["action"][data] << " ";
			}
			std::cout << std::endl;
			
		}
	}
	
	Json::Value obs;
	for(uint16_t data = 0; data < DATAS; data++){
		Json::Value datajson;
		for(int i = 0; i < 6; i++)
			datajson.append(obsArray[data][i]);
		obs.append(datajson);
	}
	
	Json::Value mig;
	for(uint16_t data = 0; data < DATAS; data++){
		Json::Value migjson;
		for(int i = 0; i < 6; i++)
			migjson.append(migArray[data][i]);
		mig.append(migjson);
	}
	
	Json::Value user_obs;
	for(uint16_t user = 0; user < USERS; user++){
		Json::Value userjson;
		for(int i = 0; i < 5; i++)
			userjson.append(userArray[user][i]);
		user_obs.append(userjson);
	}


	Json::Value obsJson;
	obsJson["obs"] = obs;
	obsJson["mig"] = mig;
	obsJson["user"] = user_obs;
	obsJson["reward"] = reward;
	obsJson["done"] = 0;
	Json::FastWriter writer;
	std::string str = writer.write(obsJson);

	zmq::message_t observation(str.size());
	memcpy(observation.data(), str.data(), str.size());
	_socket.send(observation);
	std::cout << "Send Obs" << std::endl;
}

/* ======================================= OPEN AI GYM COMMUNICATION ======================================= */


}
