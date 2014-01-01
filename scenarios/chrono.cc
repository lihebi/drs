#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "user-reader.h"
#include "hebi.h"

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
	int seed = hebi::GetEnvAsInt("CHRONO_SEED");
	RngSeedManager::SetSeed (seed);
	CommandLine cmd;
	cmd.Parse(argc, argv);
	/* topology */
	AnnotatedTopologyReader topologyReader("", 25);
	topologyReader.SetFileName("topo/grid.txt");
	topologyReader.Read();
	/* NDN stack */
	ndn::StackHelper ndnHelper;
	ndnHelper.SetContentStore("ns3::ndn::cs::Fifo", "MaxSize", "10000");
	ndnHelper.SetForwardingStrategy("ns3::ndn::fw::Flooding");
	ndnHelper.InstallAll();
	/* node container */
	NodeContainer allNodes = topologyReader.GetNodes();
	/* create app */
	ndn::AppHelper appHelper("ChronoApp");
	appHelper.Install(allNodes);
	/* routing configure */
	ndn::GlobalRoutingHelper routingHelper;
	routingHelper.InstallAll();
	for (int i=0;i<allNodes.GetN();i++) {
		Ptr<Node> node = allNodes.Get(i);
		routingHelper.AddOrigins("/"+Names::FindName(node), node);
		routingHelper.AddOrigins("/broadcast/chronoapp", node);
	}
	ndn::GlobalRoutingHelper::CalculateRoutes();

	/* simulator action */
	int stop = hebi::GetEnvAsInt("CHRONO_STOP_TIME");
	Simulator::Stop(Seconds(stop));
	ndn::L3AggregateTracer::InstallAll("aggregate-trace.txt", Seconds(1.0));
	ndn::L3RateTracer::InstallAll("rate-trace.txt", Seconds(1.0));
	Simulator::Run();
	Simulator::Destroy();

	return 0;
}
