#ifndef CHRONOAPP_H_
#define CHRONOAPP_H_

#include "ns3/ndn-app.h"
#include <boost/tuple/tuple.hpp>
#include "digest-log.h"

namespace ns3 {

class ChronoApp : public ndn::App
{
public:
	static TypeId GetTypeId();
	void StartApplication();
	void StopApplication();
	void OnInterest(Ptr<const ndn::Interest> interest);
	void OnData(Ptr<const ndn::Data> contentObject);
private:
	void ConfigFib();
	void SplitString(std::string &name, char indent);
	void ProcessRecoveryDigest(Ptr<const ndn::Interest> interest, std::string digest);
	bool ProcessDigest(Ptr<const ndn::Interest> interest, std::string digest);
	void ProcessAllPendingInterests();
	void DelayAndRecovery(Ptr<const ndn::Interest> &interest, std::string digest);
	void SendData(std::string name, const std::string msg);
	void ProcessRecoveryData(Ptr<const ndn::Data> contentObject);
	void ProcessSyncData(Ptr<const ndn::Data> contentObject);

	std::string m_root_digest;
	Digest m_digest;
	DigestLog m_digest_log;
	/*
	 * Content, Name, Seq
	 */
	ChronoMessages m_messages;
	bool m_has_pending_interest;
	std::string m_name;
};

} //ns3
#endif
