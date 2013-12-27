/*
 * FILENAME: chrono.app.h
 */
#ifndef CHRONOAPP_H_
#define CHRONOAPP_H_

#include "ns3/ndn-app.h"
#include <boost/tuple/tuple.hpp>
#include "chrono-digest-log.h"
#include "chrono-digest-tree.h"
#include "chrono-message.h"

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
	void Init();

	int GetNameType(std::string name);

	void ProcessSyncInterest(Ptr<const ndn::Interest> interest);
	void ProcessRecoveryInterest(Ptr<const ndn::Interest> interest);
	void ProcessDataInterest(Ptr<const ndn::Interest> interest);

	void ProcessSyncData(Ptr<const ndn::Data> contentObject);
	void ProcessRecoveryData(Ptr<const ndn::Data> contentObject);
	void ProcessDataData(Ptr<const ndn::Data> contentObject);

	void SendSyncInterest();
	void SendSyncData(const ndn::Name &interest_name, const std::string &name, int seq);
	void SendSyncData(std::string interest_name, const std::string &name, int seq);
	void SendRecoveryInterest(double key);
	std::string GetStringFromData(const Ptr<const ndn::Data> &contentObject);
	void UpdateAll(std::string name, int seq);
	void ProcessPendingInterest(std::string name, int seq);
	void ProcessPendingRecovery();

	void SendData(const std::string &name, const std::string &msg);
	void SendData(const ndn::Name &name, const std::string &msg);
	void SendInterest(const std::string &name);
	void SendInterest(const ndn::Name &name);

	void SyncInterestPeriod();
	void GenMessage();
	void GenMessagePeriod();

	double GetRand();

	ChronoMessage m_messages;
	DigestTree m_digest_tree;
	DigestLog m_digest_log;
	std::string m_name;
	int m_seq;
	std::string m_pending_interest;
	std::map<double, std::string> m_pending_recovery_interest;

	void ReadEnv();
	int m_env_SIPSO;
	int m_env_SIPSF;
	int m_env_SIPO;
	int m_env_SIPF;
	int m_env_GMPSO;
	int m_env_GMPSF;
	int m_env_GMPO;
	int m_env_GMPF;
	int m_env_TW;
	int m_env_ILT;
};

} //ns3
#endif
