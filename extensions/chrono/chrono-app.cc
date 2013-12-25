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
	Init();
	ConfigFib();
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
	m_seq = 0;
}
void ChronoApp::StopApplication()
{
}
int ChronoApp::GetNameType(std::string name)
{
	hebi::SplitStringByIndent(name, '/');
	std::stringstream ss(name);
	std::string stmp;
	ss>>stmp;
	if (stmp!="broadcast") return 3; // Data Interest: /<name>/   /<seq>
	ss>>stmp;
	ss>>stmp;
	if (stmp=="recovery") return 2; // Recovery Interest: /   /   /recovery/<digest>
	return 1; //Sync Interest: /   /   /<digest>
}
void ChronoApp::OnInterest(Ptr<const ndn::Interest> interest)
{
	int type = GetNameType(interest->GetName().toUri());
	switch(type) {
		case 0: ProcessSyncInterest(interest); break;
		case 1: ProcessRecoveryInterest(interest); break;
		case 2: ProcessDataInterest(interest); break;
	}
}
void ChronoApp::OnData(Ptr<const ndn::Data> contentObject)
{
	int type = GetNameType(contentObject->GetName().toUri());
	switch(type) {
		case 0: ProcessSyncData(contentObject);
		case 1: ProcessRecoveryData(contentObject);
		case 2: ProcessDataData(contentObject);
	}
}
void ChronoApp::SendSyncData(const ndn::Name &interest_name, const std::string &name, int seq)
{
	std::string data_str = "<name>"+name+"</name><seq>"+std::to_string(seq)+"</seq>";
	SendData(interest_name, data_str);
}
void ChronoApp::ProcessSyncInterest(Ptr<const ndn::Interest> interest)
{
	std::string digest = hebi::GetSubStringByIndent(interest->GetName().toUri(), '/', 3);
	if (digest == m_digest_tree.GetRootDigest()) {
		m_pending_interest = interest;
	} else if (m_digest_log.HasDigest(digest)) {
		boost::tuple<std::string, int> t = m_digest_log.GetNameAndSeq(digest);
		SendSyncData(interest->GetName(), t.get<0>(), t.get<1>());
	} else {
		long _time = hebi::GetPosixTime_Milli(); //they should possibly be different..
		m_pending_recovery_interest[_time] = interest;
		Simulator::Schedule(Seconds(CHRONO_TW), &ChronoApp::SendRecoveryInterest, this, _time);
	}
}
void ChronoApp::SendRecoveryInterest(long key)
{
	if (m_pending_recovery_interest[key] != NULL) {
		std::string digest = hebi::GetSubStringByIndent(m_pending_recovery_interest[key]->GetName().toUri(), '/', 3);
		SendInterest("/broadcast/chronoapp/recovery/"+digest);
	}
	m_pending_recovery_interest.erase(key);
}
void ChronoApp::ProcessRecoveryInterest(Ptr<const ndn::Interest> interest)
{
	std::string digest = hebi::GetSubStringByIndent(interest->GetName().toUri(), '/', 4);
	if (digest == m_digest_tree.GetRootDigest() || m_digest_log.HasDigest(digest)) {
		std::string data_str = m_digest_tree.GetCurrentStateAsXML();
		SendData(interest->GetName(), data_str);
	}
}
void ChronoApp::ProcessDataInterest(Ptr<const ndn::Interest> interest)
{
	int seq = atoi((hebi::GetSubStringByIndent(interest->GetName().toUri(), '/', 3)).c_str());
	SendData(interest->GetName(), m_messages.Get(seq));
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
std::string ChronoApp::GetStringFromData(const Ptr<const ndn::Data> &contentObject)
{
	int size = contentObject->GetPayload()->GetSize();
	const unsigned char *data = contentObject->GetPayload()->PeekData();
	std::string s(reinterpret_cast<const char*>(data), size);
	return s;
}
void ChronoApp::ProcessRecoveryData(Ptr<const ndn::Data> contentObject)
{
	std::string xml = GetStringFromData(contentObject);
	std::vector<boost::tuple<std::string, int> > v = m_digest_tree.CompareCurrentStateByXML(xml);
	for (int i=0;i<v.size();i++) {
		SendInterest("/"+v[i].get<0>()+"/chronoapp/"+std::to_string(v[i].get<1>()));
	}
}
void ChronoApp::ProcessDataData(Ptr<const ndn::Data> contentObject)
{
	std::string name = hebi::GetSubStringByIndent(contentObject->GetName().toUri(), '/', 1);
	int seq = atoi((hebi::GetSubStringByIndent(contentObject->GetName().toUri(), '/', 3)).c_str());
	std::string content = GetStringFromData(contentObject); //log it into file
	UpdateAll(name, seq);
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
void ChronoApp::SendSyncInterest()
{
	SendInterest("/broadcast/chronoapp/"+m_digest_tree.GetRootDigest());
}
void ChronoApp::ProcessPendingInterest(std::string name, int seq)
{
	if (m_pending_interest != NULL) {
		SendSyncData(m_pending_interest->GetName(), name, seq);
	}
}
void ChronoApp::ProcessPendingRecovery()
{
	typedef std::map<long, Ptr<const ndn::Interest> > map_type;
	BOOST_FOREACH(map_type::value_type &m, m_pending_recovery_interest) {
		std::string digest = hebi::GetSubStringByIndent(m.second->GetName().toUri(), '/', 3);
		if (digest == m_digest_tree.GetRootDigest()) {
			m_pending_interest = m.second;
			m.second = NULL;
		} else if (m_digest_log.HasDigest(digest)) {
			boost::tuple<std::string, int> t = m_digest_log.GetNameAndSeq(digest);
			SendSyncData(m.second->GetName(), t.get<0>(), t.get<1>());
			m.second = NULL;
		}
	}
}




void ChronoApp::SendData(const std::string &name, const std::string &msg)
{
	const ndn::Name _name(name);
	SendData(_name, msg);
}
void ChronoApp::SendData(const ndn::Name &name, const std::string &msg)
{
	Ptr<Packet> packet = Create<Packet>(msg);
	Ptr<ndn::Data> data = Create<ndn::Data>(packet);
	data->SetName(name);
	m_transmittedDatas(data, this, m_face);
	m_face->ReceiveData(data);
}
void ChronoApp::SendInterest(const std::string &name)
{
	SendInterest(Create<ndn::Name>(name));
}
void ChronoApp::SendInterest(const Ptr<ndn::Name> &name)
{
	Ptr<ndn::Interest> interest = Create<ndn::Interest>();
	UniformVariable rand(0, std::numeric_limits<uint32_t>::max());
	interest->SetNonce(rand.GetValue());
	interest->SetName(name);
	interest->SetInterestLifetime(Seconds(1.0)); //????
	m_transmittedInterests(interest, this, m_face);
	m_face->ReceiveInterest(interest);
}

void ChronoApp::GenMessagePeriod()
{
	GenMessage();
	Simulator::Schedule(Seconds(1), &ChronoApp::GenMessagePeriod, this);
}
void ChronoApp::GenMessage()
{
	std::string msg = "Hello, I'm "+m_name+" at "+ std::to_string(hebi::GetPosixTime_TotalMilli());
	m_messages.Insert(m_seq, msg);
	UpdateAll(m_name, m_seq);
	m_seq++;
}
void ChronoApp::SyncInterestPeriod()
{
	SendSyncInterest();
	Simulator::Schedule(Seconds(SYNC_RESEND_PERIOD), &ChronoApp::SyncInterestPeriod, this);
}


} //ns3
