/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/opengym-helper.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/data-rate.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/bulk-send-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/log.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/point-to-point-module.h"

#include "ns3/opengym-parameter.h"
#include "ns3/target-node.h"
#include "ns3/host-node.h"
#include <map>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace ns3;

//
// Network Topology
//                 90us, 1G                     10us, 10G
// Tier1 NVMe -----------------  Router ====================== Host
// 				  180us, 1G     /  /  /
// Tier2 NVMe ------------------  /  /
//                270us, 1G      /  /
// Tier3 NVMe -------------------  /
//                360us, 1G       /
// Tier4 NVMe --------------------

NS_LOG_COMPONENT_DEFINE("TieringExample");

	int 
main (int argc, char *argv[])
{
	srand((unsigned int)time(NULL));

	bool verbose = true;
	uint16_t port = 8000;
	uint16_t numTargets = TARGETS;
	uint32_t numFlows = 4;

	float endTime = 200.0;
	int gym_port = 5555;
	bool heuristic = false;

	float epiRewardSum = 0;
	float epi = 0;

	CommandLine cmd;
	cmd.AddValue ("verbose", "Tell application to log if true", verbose);
	cmd.AddValue ("gymport", "Port number for gym communication", gym_port);
	cmd.AddValue ("heuristic", "Port number for gym communication", heuristic);
	cmd.Parse (argc,argv);

	while(1){
		std::cout << "1 Episode Start" << std::endl;

		LogComponentEnable ("HostNode", LOG_LEVEL_DEBUG);
		NS_LOG_INFO("Create router nodes.");
		NodeContainer RaH; // RaH : Router and Host
		RaH.Create(2);

		PointToPointHelper p2p_RaH;
		p2p_RaH.SetDeviceAttribute ("DataRate", StringValue ("600Mbps"));

		NetDeviceContainer dev_RaH = p2p_RaH.Install(RaH);

		InternetStackHelper internet;
		internet.Install (RaH);

		Ipv4AddressHelper ipv4;
		ipv4.SetBase ("10.1.1.0", "255.255.255.0");
		ipv4.Assign (dev_RaH);

		// disable tc because of bugs
		TrafficControlHelper tch;
		tch.Uninstall (dev_RaH);

		std::map<uint16_t, Ipv4Address> ipMap;
		std::map<uint16_t, Ptr<TargetNode>> appMap;

		for (uint16_t target = 0; target < numTargets; target++){
			NodeContainer NaR;
			NaR.Create(1);

			NodeContainer targetToRouter;
			targetToRouter.Add(NaR.Get(0));
			targetToRouter.Add(RaH.Get(0));

			PointToPointHelper p2p_NaR;
			p2p_NaR.SetDeviceAttribute ("DataRate", StringValue ("200Mbps"));

			NetDeviceContainer dev_NaR = p2p_NaR.Install(targetToRouter);

			internet.Install (NaR);

			std::string IpBase;
			IpBase = "10.1."+std::to_string(target+1+1)+".0";
			ipv4.SetBase(IpBase.c_str(), "255.255.255.0");

			Ipv4InterfaceContainer i = ipv4.Assign(dev_NaR);

			tch.Uninstall(dev_NaR);

			Ptr<TargetNode> targetApp = CreateObject<TargetNode> ();
			NaR.Get (0) -> AddApplication (targetApp);
			targetApp-> SetTier(target);
			targetApp-> SetStartTime(Seconds (1.0));
			targetApp-> SetStopTime(Seconds (endTime));

			appMap[target] = targetApp;
			ipMap[target] = i.GetAddress(0);
		}

		Ptr<HostNode> hostApp = CreateObject<HostNode> ();
		RaH.Get (1)-> AddApplication (hostApp);
		
		hostApp-> SetGymPort(gym_port);

		for (uint16_t target = 0; target < numTargets; target++){
			hostApp-> AddTarget(target, ipMap[target], port, appMap[target]);
		}

		for (uint32_t flow = 0; flow < numFlows; flow++){
			//hostApp-> AddFlow(flow%numTargets, INITRATE); // Inital Tier is 0 and Inital Sending Rate is 100Mbps
		}

		hostApp->SetStartTime (Seconds (1.0));
		hostApp->SetStopTime (Seconds (endTime));

		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
		Simulator::Stop (Seconds (endTime+0.001));
		
		Simulator::Run ();
		Simulator::Destroy ();
	
		epiRewardSum += hostApp->GetAverageReward();
		epi++;
		std::cout << "AVG REWARD : " << epiRewardSum / epi << std::endl;
	}
	std::cout << "Simulator ENDED" << std::endl;
}


