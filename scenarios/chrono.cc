#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "user-reader.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("lihebi");

void my_printer(Ptr<Node> node, Time next)
{
	NS_LOG_DEBUG("\tprefix, interface");
	NS_LOG_DEBUG((node->GetId()));
	Ptr<ndn::Fib> fib = node->GetObject<ndn::Fib>();
	for (Ptr<ndn::fib::Entry> entry = fib->Begin(); entry!=fib->End(); entry = fib->Next(entry)) {
		NS_LOG_DEBUG("\t: " << entry->GetPrefix() << " " << entry->FindBestCandidate());
	}
	Simulator::Schedule(next, my_printer, node, next);
}

int
main(int argc, char** argv)
{
	CommandLine cmd;
	cmd.Parse(argc, argv);
	/* topology */
	AnnotatedTopologyReader topologyReader("", 25);
	topologyReader.SetFileName("topo/topo.txt");
	topologyReader.Read();
	UserReader userReader;
	userReader.SetFilename("topo/user.txt");
	userReader.Read();
	/* NDN stack */
	ndn::StackHelper ndnHelper;
	ndnHelper.SetContentStore("ns3::ndn::cs::Fifo", "MaxSize", "10000");
	ndnHelper.SetForwardingStrategy("ns3::ndn::fw::Flooding");
	ndnHelper.InstallAll();
	/* node container */
	NodeContainer allNodes = topologyReader.GetNodes();
	NodeContainer chatterNodes;
	for (int i=0;i<userReader.m_users.size();i++) {
		chatterNodes.Add(Names::Find<Node>(userReader.m_users[i].node));
	}
	/* create app */
	ndn::AppHelper appHelper("ChronoApp");
	appHelper.Install(chatterNodes);
	/*
	srand(time(NULL));
	for (int i=0;i<chatterNodes.GetN();i++) {
		appHelper.Install(chatterNodes.Get(i)).Start(Seconds(5*(double)rand()/RAND_MAX));
	}*/
	/* routing configure */
	ndn::GlobalRoutingHelper routingHelper;
	routingHelper.InstallAll();
	for (int i=0;i<chatterNodes.GetN();i++) {
		Ptr<Node> node = chatterNodes.Get(i);
		routingHelper.AddOrigins("/"+Names::FindName(node), node);
		routingHelper.AddOrigins("/broadcast/chronoapp", node);
	}
	ndn::GlobalRoutingHelper::CalculateRoutes();

	/* simulator action */
	Simulator::Stop(Seconds(250.0));
	ndn::L3AggregateTracer::InstallAll("aggregate-trace.txt", Seconds(1.0));
	ndn::L3RateTracer::InstallAll("rate-trace.txt", Seconds(1.0));
	Simulator::Run();
	Simulator::Destroy();

	return 0;
}
