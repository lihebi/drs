/*
 * FILENAME: chrono-app.c
 * ChronoSync Chat App
 *
 * Interest: 	/broadcast/chronoapp/<digest>			SYNC INTEREST
 * 			<name> ... </name> <seq> ... </seq>
 * 		/broadcast/chronoapp/recovery/<digest>		RECOVERY INTEREST
 * 			<li> <name> ... </name> <seq> ... </seq> </li>
 * 			<li> <name> ... </name> <seq> ... </seq> </li>
 * 		/<name>/   /<seq>				DATA INTEREST
 * 			content
 */
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"

#include "ns3/ndn-app-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-data.h"

#include "ns3/ndn-fib.h"
#include "ns3/random-variable.h"
#include "ns3/core-module.h"
#include "chrono-app.h"
#include "../hebi.h"
#include<boost/foreach.hpp>

NS_LOG_COMPONENT_DEFINE("ChronoApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(ChronoApp);


TypeId ChronoApp::GetTypeId()
{
	static TypeId tid = TypeId("ChronoApp")
		.SetParent<ndn::App>()
		.AddConstructor<ChronoApp>()
		;
	return tid;
}
void ChronoApp::StartApplication()
{
	ndn::App::StartApplication();
	ReadEnv();
	Init();
	ConfigFib();
	Simulator::Schedule(Seconds(0), &ChronoApp::SyncInterestPeriod, this);
	Simulator::Schedule(Seconds(m_env_GMPSO+m_env_GMPSF*GetRand()), &ChronoApp::GenMessagePeriod, this);
}
void ChronoApp::ReadEnv()
{
	m_env_SIPSO = hebi::GetEnvAsDouble("CHRONO_SYNC_INTEREST_PERIOD_START_OFFSET");
	m_env_SIPSF = hebi::GetEnvAsDouble("CHRONO_SYNC_INTEREST_PERIOD_START_FACTOR");
	m_env_SIPO = hebi::GetEnvAsDouble("CHRONO_SYNC_INTEREST_PERIOD_OFFSET");
	m_env_SIPF = hebi::GetEnvAsDouble("CHRONO_SYNC_INTEREST_PERIOD_FACTOR");
	m_env_GMPSO = hebi::GetEnvAsDouble("CHRONO_GEN_MESSAGE_PERIOD_START_OFFSET");
	m_env_GMPSF = hebi::GetEnvAsDouble("CHRONO_GEN_MESSAGE_PERIOD_START_FACTOR");
	m_env_GMPO = hebi::GetEnvAsDouble("CHRONO_GEN_MESSAGE_PERIOD_OFFSET");
	m_env_GMPF = hebi::GetEnvAsDouble("CHRONO_GEN_MESSAGE_PERIOD_FACTOR");
	m_env_TW = hebi::GetEnvAsDouble("CHRONO_TW");
	m_env_ILT = hebi::GetEnvAsDouble("CHRONO_INTEREST_LIFE_TIME");
}
void ChronoApp::ConfigFib()
{
	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();
	Ptr<ndn::Name> prefix = Create<ndn::Name>("/broadcast/chronoapp");
	fib->Add(*prefix, m_face, 0);
	prefix = Create<ndn::Name>("/"+m_name);
	fib->Add(*prefix, m_face, 0);
}
void ChronoApp::Init()
{
	m_name = Names::FindName(GetNode());
	/* from 1 because the default map<string, int> is 0, can not compare */
	m_seq = 1;
	m_digest_tree.InitRootDigest();
}
/*===========================================================================================
 * 				Periods
 *==========================================================================================*/

void ChronoApp::GenMessagePeriod()
{
	GenMessage();
	Simulator::Schedule(Seconds(m_env_GMPO+m_env_GMPF*GetRand()), &ChronoApp::GenMessagePeriod, this);
}
void ChronoApp::GenMessage()
{
	std::string msg = "Hello, I'm "+m_name+" at "+ std::to_string(Simulator::Now().GetSeconds());
	NS_LOG_DEBUG("Gened Msg: "<<msg<<", My seq is "<<m_seq);
	m_messages.Insert(m_seq, msg);
	UpdateAll(m_name, m_seq);
	m_seq++;
}
void ChronoApp::SyncInterestPeriod()
{
	SendSyncInterest();
	Simulator::Schedule(Seconds(m_env_SIPO+m_env_SIPF*GetRand()), &ChronoApp::SyncInterestPeriod, this);
}
void ChronoApp::StopApplication()
{
}
/*======================================================================================
 * 			On Interest And Data
 *=====================================================================================*/
int ChronoApp::GetNameType(std::string name)
{
	hebi::SplitStringByIndent(name, '/');
	std::stringstream ss(name);
	std::string stmp;
	ss>>stmp;
	if (stmp!="broadcast") return 2; // Data Interest: /<name>/   /<seq>
	ss>>stmp;
	ss>>stmp;
	if (stmp=="recovery") return 1; // Recovery Interest: /   /   /recovery/<digest>
	return 0; //Sync Interest: /   /   /<digest>
}
void ChronoApp::OnInterest(Ptr<const ndn::Interest> interest)
{
	NS_LOG_DEBUG("Received Interest for: "<<interest->GetName());
	int type = GetNameType(interest->GetName().toUri());
	switch(type) {
		case 0: ProcessSyncInterest(interest); break;
		case 1: ProcessRecoveryInterest(interest); break;
		case 2: ProcessDataInterest(interest); break;
	}
}
void ChronoApp::OnData(Ptr<const ndn::Data> contentObject)
{
	NS_LOG_DEBUG("Received Data for: "<<contentObject->GetName());
	int type = GetNameType(contentObject->GetName().toUri());
	switch(type) {
		case 0: ProcessSyncData(contentObject); break;
		case 1: ProcessRecoveryData(contentObject); break;
		case 2: ProcessDataData(contentObject); break;
	}
}
/*=======================================================================
 * 			Process 3x2
 *======================================================================*/
/*--------------------------
 * 	Sync Interest
 *-------------------------*/
void ChronoApp::ProcessSyncInterest(Ptr<const ndn::Interest> interest)
{
	std::string digest = hebi::GetSubStringByIndent(interest->GetName().toUri(), '/', 3);
	if (digest == m_digest_tree.GetRootDigest()) {
		NS_LOG_DEBUG("The same as mine, save as Pending Interest.");
		m_pending_interest = interest->GetName().toUri();
	} else if (m_digest_log.HasDigest(digest)) {
		boost::tuple<std::string, int> t = m_digest_log.GetNameAndSeq(digest);
		SendSyncData(interest->GetName(), t.get<0>(), t.get<1>());
	} else {
		NS_LOG_DEBUG("Not Know, schedule for recovery After "<<m_env_TW<<"s");
		double _time = hebi::ConvertDouble(Simulator::Now().GetSeconds()); //they should possibly be different..
		m_pending_recovery_interest[_time] = interest->GetName().toUri();
		Simulator::Schedule(Seconds(m_env_TW), &ChronoApp::SendRecoveryInterest, this, _time);
	}
}
void ChronoApp::ProcessSyncData(Ptr<const ndn::Data> contentObject)
{
	int size = contentObject->GetPayload()->GetSize();
	const unsigned char *data = contentObject->GetPayload()->PeekData();
	pugi::xml_document doc;
	doc.load_buffer(data, size);
	std::string name = doc.child("name").text().get();
	std::string seq = doc.child("seq").text().get();
		SendInterest("/"+name+"/chronoapp/"+seq);
}
/*----------------------------
 * 	Recovery Interest
 *---------------------------*/
void ChronoApp::ProcessRecoveryInterest(Ptr<const ndn::Interest> interest)
{
	std::string digest = hebi::GetSubStringByIndent(interest->GetName().toUri(), '/', 4);
	if (digest == m_digest_tree.GetRootDigest() || m_digest_log.HasDigest(digest)) {
		std::string data_str = m_digest_tree.GetCurrentStateAsXML();
		SendData(interest->GetName(), data_str);
	}
}
void ChronoApp::ProcessRecoveryData(Ptr<const ndn::Data> contentObject)
{
	/* need to exclude that already have */
	std::string xml = GetStringFromData(contentObject);
	std::vector<boost::tuple<std::string, int> > v = m_digest_tree.CompareCurrentStateByXML(xml);
	for (int i=0;i<v.size();i++) {
		if (m_digest_tree.GetSeqByName(v[i].get<0>()) < v[i].get<1>()) {
			SendInterest("/"+v[i].get<0>()+"/chronoapp/"+std::to_string(v[i].get<1>()));
		}
	}
}
/*----------------------------
 * 	Data Interest
 *---------------------------*/
void ChronoApp::ProcessDataInterest(Ptr<const ndn::Interest> interest)
{
	int seq = atoi((hebi::GetSubStringByIndent(interest->GetName().toUri(), '/', 3)).c_str());
	SendData(interest->GetName(), m_messages.Get(seq));
}
void ChronoApp::ProcessDataData(Ptr<const ndn::Data> contentObject)
{
	std::string name = hebi::GetSubStringByIndent(contentObject->GetName().toUri(), '/', 1);
	int seq = atoi((hebi::GetSubStringByIndent(contentObject->GetName().toUri(), '/', 3)).c_str());
	std::string content = GetStringFromData(contentObject); //log it into file
	if (m_digest_tree.GetSeqByName(name)<seq) //DEBUGED
		NS_LOG_DEBUG("THE CONTENT: "<<content);
	UpdateAll(name, seq);
}
/*=======================================================================================================
 * 						Utils
 *======================================================================================================*/
/*-------------------------------
 * 	Send Interest and Data
 *------------------------------*/
void ChronoApp::SendSyncData(std::string interest_name, const std::string &name, int seq)
{
	ndn::Name _interest_name(interest_name);
	SendSyncData(_interest_name, name, seq);
}
void ChronoApp::SendSyncData(const ndn::Name &interest_name, const std::string &name, int seq)
{
	std::string data_str = "<name>"+name+"</name><seq>"+std::to_string(seq)+"</seq>";
	SendData(interest_name, data_str);
}
void ChronoApp::SendRecoveryInterest(double key)
{
	if (m_pending_recovery_interest[key] != "") {
		std::string digest = hebi::GetSubStringByIndent(m_pending_recovery_interest[key], '/', 3);
		SendInterest("/broadcast/chronoapp/recovery/"+digest);
	}
	m_pending_recovery_interest.erase(key);
}
void ChronoApp::SendSyncInterest()
{
	SendInterest("/broadcast/chronoapp/"+m_digest_tree.GetRootDigest());
}
/*---------------------------------
 * 	Process Pending
 *--------------------------------*/
void ChronoApp::ProcessPendingInterest(std::string name, int seq)
{
	if (m_pending_interest != "") {
		SendSyncData(m_pending_interest, name, seq);
	}
}
void ChronoApp::ProcessPendingRecovery()
{
	typedef std::map<double, std::string> map_type;
	BOOST_FOREACH(map_type::value_type &m, m_pending_recovery_interest) {
		if (m.second=="") continue;
		std::string digest = hebi::GetSubStringByIndent(m.second, '/', 3);
		if (digest == m_digest_tree.GetRootDigest()) {
			m_pending_interest = m.second;
			m.second = "";
		} else if (m_digest_log.HasDigest(digest)) {
			boost::tuple<std::string, int> t = m_digest_log.GetNameAndSeq(digest);
			SendSyncData(m.second, t.get<0>(), t.get<1>());
			m.second = "";
		}
	}
}
/*------------------------------------
 * 	Other
 *-----------------------------------*/
std::string ChronoApp::GetStringFromData(const Ptr<const ndn::Data> &contentObject)
{
	int size = contentObject->GetPayload()->GetSize();
	const unsigned char *data = contentObject->GetPayload()->PeekData();
	std::string s(reinterpret_cast<const char*>(data), size);
	return s;
}
void ChronoApp::UpdateAll(std::string name, int seq)
{
	std::string oldDigest = m_digest_tree.InsertAndUpdate(name, seq);
	if (oldDigest!="") {
		SendSyncInterest();
		m_digest_log.Insert(oldDigest, name, seq);
		ProcessPendingInterest(name, seq);
		ProcessPendingRecovery();
	}
}
double ChronoApp::GetRand()
{
	UniformVariable rand(0, std::numeric_limits<uint32_t>::max());
	return rand.GetValue()/std::numeric_limits<uint32_t>::max();
}

/*===============================================================================
 * 				Stable Functions
 *==============================================================================*/

void ChronoApp::SendData(const std::string &name, const std::string &msg)
{
	const ndn::Name _name(name);
	SendData(_name, msg);
}
void ChronoApp::SendData(const ndn::Name &name, const std::string &msg)
{
	NS_LOG_DEBUG("Send Data for: "<<name);
	Ptr<Packet> packet = Create<Packet>(msg);
	Ptr<ndn::Data> data = Create<ndn::Data>(packet);
	data->SetName(name);
	m_transmittedDatas(data, this, m_face);
	m_face->ReceiveData(data);
}
void ChronoApp::SendInterest(const std::string &name)
{
	const ndn::Name _name(name);
	SendInterest(_name);
}
void ChronoApp::SendInterest(const ndn::Name &name)
{
	NS_LOG_DEBUG("Send Interest for: "<<name);
	Ptr<ndn::Interest> interest = Create<ndn::Interest>();
	UniformVariable rand(0, std::numeric_limits<uint32_t>::max());
	interest->SetNonce(rand.GetValue());
	interest->SetName(name);
	interest->SetInterestLifetime(Seconds(m_env_ILT)); //????
	m_transmittedInterests(interest, this, m_face);
	m_face->ReceiveInterest(interest);
}



} //ns3
