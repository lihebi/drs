#ifndef CHRONOAPP_H_
#define CHRONOAPP_H_

#include "ns3/ndn-app.h"
#include <boost/tuple/tuple.hpp>
#include "chrono-digest-log.h"
#include "chrono-digest-tree.h"

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

	int GetNameType(std::string name);

	void ProcessSyncInterest(const Ptr<ndn::Interest> &interest);
	void ProcessRecoveryInterest(const Ptr<ndn::Interest> &interest);
	void ProcessDataInterest(const Ptr<ndn::Interest> &interest);

	void ProcessSyncData(const Ptr<ndn::Data> &contentObject);
	void ProcessRecoveryData(const Ptr<ndn::Data> &contentObject);
	void ProcessDataData(const Ptr<ndn::Data> &contentObject);

	void SendSyncData(const ndn::Name &interest_name, const std::string &name, int seq);
	void SendRecoveryInterest(int index);
	std::string GetStringFromData(const Ptr<const ndn::Data> &contentObject);
	void UpdateAll(std::string name, int seq);

	void SendData(const std::string &name, const std::string &msg);
	void SendData(const ndn::Name &name, const std::string &msg);
	void SendInterest(cosnt std::string &name)
	void SendInterest(const Ptr<ndn::Name> &name)

	void SyncInterestPeriod();
	void GenMessage();
	void GenMessagePeriod();

	ChronoMessage m_messages;
	DigestTree m_digest_tree;
	DigestLog m_digest_log;
	std::string m_name;
	int m_seq;
	Ptr<ndn::Interest> m_pending_interest;
	std::map<long, Ptr<ndn::Interest> > m_pending_recovery_interest;

	const int CHRONO_TW = 2;
	const int SYNC_RESEND_PERIOD = 5;
};

} //ns3
#endif
