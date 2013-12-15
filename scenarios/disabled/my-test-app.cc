#include "test-app.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "UserReader.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("MyTestApp");

int main(int argc, char** argv)
{
	Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
	Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
	Config::SetDefault("ns3::DropTailQueue::MaxPackets", StringValue("20"));

	CommandLine cmd;
	cmd.Parse(argc, argv);
	/* topology */
	AnnotatedTopologyReader topologyReader("", 25);
	topologyReader.SetFileName("scenarios/topo.txt");
	topologyReader.Read();
	UserReader userReader;
	userReader.SetFilename("scenarios/user.txt");
	userReader.Read();
	/* NDN stack */
	ndn::StackHelper ndnHelper;
	ndnHelper.SetContentStore("ns3::ndn::cs::Fifo", "MaxSize", "10000");
	ndnHelper.SetForwardingStrategy("ns3::ndn::fw::BestRoute");
	ndnHelper.InstallAll();
	/* routing */
	ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
	ndnGlobalRoutingHelper.InstallAll();
	/* app participants */
	NodeContainer participantNodes;
	for (int i=0;i<userReader.m_users.size();i++) {
		participantNodes.Add(Names::Find<Node>(userReader.m_users[i].node));
	}
	/* create app */
	ndn::AppHelper serverHelper("ServerApp");
	ndn::AppHelper clientHelper("ClientApp");
	//configure
	serverHelper.Install(participantNodes.Get(0));
	serverHelper.Install(participantNodes.Get(1));
	/* routing configure */
	for (int i=0;i<userReader.m_users.size();i++) {
		ndnGlobalRoutingHelper.AddOrigins(userReader.m_users[i].prefix, participantNodes.Get(i));
		ndnGlobalRoutingHelper.AddOrigins("/broadcast/chatapp", participantNodes.Get(i));
	}
	ndn::GlobalRoutingHelper::CalculateRoutes();

	/*
	Simulator::Schedule(Seconds(1), my_printer, Names::Find<Node>("Node11"), Seconds(1)); 
	Simulator::Schedule(Seconds(1), my_printer, Names::Find<Node>("Node12"), Seconds(1)); 
	Simulator::Schedule(Seconds(1), my_printer, Names::Find<Node>("Node21"), Seconds(1)); 
	Simulator::Schedule(Seconds(1), my_printer, Names::Find<Node>("Node22"), Seconds(1)); 
	Simulator::Schedule(Seconds(1), my_printer, Names::Find<Node>("Node23"), Seconds(1)); 
	*/
	/* simulator action */
	Simulator::Stop(Seconds(20.0));
	ndn::L3AggregateTracer::InstallAll("aggregate-trace.txt", Seconds(1.0));
	ndn::L3RateTracer::InstallAll("rate-trace.txt", Seconds(1.0));
	Simulator::Run();
	Simulator::Destroy();

	return 0;
}
