/*
 * ChronoSync Chat App
 *
 * Interest: 	/broadcast/chronoapp/<digest>
 * 		/broadcast/chronoapp/recovery/<digest>
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
	ConfigFib();
	Init();
}
void ChronoApp::Init()
{
	m_has_pending_interest = false;
	m_name = Names::FindName(GetObject());
}
void ChronoApp::StopApplication()
{
}
void ChronoApp::OnInterest(Ptr<const ndn::Interest> interest)
{
	ndn::App::OnInterest(interest);
	std::string name = interest->GetName().toUri();
	SplitString(name, '/');
	std::stringstream ss(name);
	std::string stmp;
	ss>>stmp;
	if (stmp == "broadcast") {
		ss>>stmp;
		if (stmp == "chronoapp") {
			ss>>stmp;
			/* recovery interest */
			if (stmp == "recovery") {
				ss>>stmp;
				ProcessRecoveryDigest(interest, stmp);
			}
			/* sync interest */
			else {
				ProcessDigest(interest, stmp);
			}
		}
	}
	/* /<name>/chronoapp/<seq> */
	else {
		ss>>stmp;
		ss>>stmp;
		std::string data_str = m_messages.Get(m_name, stmp);
		SendData(interest->GetName().toUri(), data_str);
	}
}
/*
 * process recovery digest
 */
void ChronoApp::ProcessRecoveryDigest(Ptr<const ndn::Interest> interest, std::string digest)
{
}
/*
 * process digest
 */
bool ChronoApp::ProcessDigest(Ptr<const ndn::Interest> interest, std::string digest)
{
	if (digest == m_root_digest)
		m_has_pending_interest = true;
	else {
		int index = m_digest_log.FindDigest(digest);
		if (index == -1) {
			Simulator::Schedule(Seconds(2), &ChronoApp::DelayAndRecovery, this, interest, digest);
		} else {
			std::string data_str = m_digest_log.GetFromIndexAsXML(index);
			SendData(interest->GetName().toUri(), data_str);
			m_has_pending_interest = false;
		}
	}
}
/*
 * Nearly same as ProcessDigest
 * Do this repeated work because I can't find suitable cancel function.
 */
void ChronoApp::DelayAndRecovery(Ptr<const ndn::Interest> &interest, std::string digest)
{
	if (digest == m_root_digest)
		m_has_pending_interest = true;
	else {
		int index = m_digest_log.FindDigest(digest);
		if (index == -1) {
			//the different
			;
		} else {
			std::string data_str = m_digest_log.GetFromIndexAsXML(index);
			SendData(interest->GetName().toUri(), data_str);
		}
	}
}
void ChronoApp::SendData(std::string name, const std::string msg)
{
	NS_LOG_DEBUG("Sending data for " << name);
	Ptr<Packet> packet = Create<Packet>(msg);
	Ptr<ndn::Data> data = Create<ndn::Data>(packet);
	data->SetName(Create<ndn::Name>(name));
	m_transmittedDatas(data, this, m_face);
	m_face->ReceiveData(data);
}

/* 
 * process all pending interest
 */
void ChronoApp::ProcessAllPendingInterests()
{
}
void ChronoApp::OnData(Ptr<const ndn::Data> contentObject)
{
	std::string name = contentObject->GetName().toUri();
	SplitString(name, '/');
	std::stringstream ss(name);
	std::string stmp;
	ss>>stmp;
	if (stmp=="broadcast") {
		ss>>stmp;
		if (stmp=="chronoapp") {
			ss>>stmp;
			/* recovery data */
			if (stmp=="recovery") {
				ss>>stmp;
				ProcessRecoveryData(contentObject, stmp);
			}
			/* sync data */
			else {
				ProcessSyncData(contentObject, stmp);
			}
		}
	}
	else {
		std::string name(stmp);
		int seq;
		ss>>stmp;
		ss>>seq;
		ProcessTrueData(contentObject, name, seq);
	}
}
/*
 * Format:
 * 	/broadcast/chronoapp/recovery/<digest>
 * return:
 * 	<packet>
 * 		<li> <name> ... </name> <seq> ... </seq> </li>
 * 		<li> ...... </li>
 * 	</packet>
 */
void ChronoApp::ProcessRecoveryData(const Ptr<const ndn::Data> &contentObject)
{
	ProcessSyncData(contentObject);
}
/*
 * Format:
 * 	/broadcast/chronoapp/<digest>
 * return:
 * 	<packet>
 * 		<li> <name> ... </name> <seq> ... </seq> </li>
 * 		<li> ... </li>
 * 	</packet>
 * Send:
 * 	/<name>/chronoapp/<seq>
 */
void ChronoApp::ProcessSyncData(const Ptr<const ndn::Data> &contentObject)
{
	int size = contentObject->GetPayload()->GetSize();
	const unsigned char *data = contentObject->GetPayload()->PeekData();
	std::string msg(reinterpret_cast<const char *>(data), size);
	pugi::xml_document doc;
	doc.load(msg);
	pugi::xml_node node = doc.child("packet").child("li");
	for (;node;node=node.next_sibling()) {
		//m_messages.Push(node.child("name").text().get(), node.child("seq").text().as_int());
		std::string name = node.child("name").text().get();
		int seq = node.child("name").text().as_int();
		SendInterest("/"+name+"/chronoapp/"+std::to_string(seq));
	}
}
/*
 * Format:
 * 	/<name>/chronoapp/<seq>
 * return:
 * 	<content>
 * update;
 */
void ChronoApp::ProcessTrueData(const Ptr<const ndn::Data> &contentObject, std::string name, int seq)
{
	int size = contentObject->GetPayload()->GetSize();
	const unsigned char *data = contentObject->GetPayload()->PeekData();
	std::string msg(reinterpret_cast<const char *>(data), size);
	UpdateLocal(name, seq, msg);
}
void ChronoApp::UpdateLocal(std::string name, int seq, std::string msg)
{
	m_messages.Push(name, seq, msg);
	m_digest
}
void ChronoApp::ConfigFib()
{
	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();
	Ptr<ndn::Name> prefix = Create<ndn::Name>("/broadcast/chronoapp");
	fib->Add(*prefix, m_face, 0);
}
void ChronoApp::SplitString(std::string &name, char indent)
{
	int index;
	for (;;) {
		index = name.find(indent);
		if (index == -1)
			break;
		else 
			name[index] = indent;
	}
}




}
