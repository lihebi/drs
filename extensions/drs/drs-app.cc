/*
 * Need Improve:
 * 	1. use boost::posix_time instead of time(NULL)
 * Next Step:
 * 	1. add tracer
 * 	2. do experiment
 * 	3. implement ChronoSync
 */
#include "drs-app.h"
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
#include "../hebi.h"
#include<boost/foreach.hpp>

NS_LOG_COMPONENT_DEFINE("DRSApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(DRSApp);

TypeId DRSApp::GetTypeId()
{
	static TypeId tid = TypeId("DRSApp")
		.SetParent<ndn::App>()
		.AddConstructor<DRSApp>()
		;
	return tid;
}
/*
 * 1. Init(): set private variables
 * 2. Config(): set fib: 1. /broadcast/drsapp 2. /<m_name>/drsapp
 * 3. start Period: check and gen server
 * 4. start Gen Message Period: gen message
 */
void DRSApp::StartApplication()
{
	NS_LOG_DEBUG(Simulator::Now().GetSeconds());
	ndn::App::StartApplication();
	Init();
	ConfigFib();
	Simulator::Schedule(Seconds(2), &DRSApp::GenServerPeriod, this);
	Simulator::Schedule(Seconds(20), &DRSApp::GenMessagePeriod, this);
	Simulator::Schedule(Seconds(20), &DRSApp::AnythingNewInterestPeriod, this);
}
void DRSApp::Init()
{
	m_server = "";
	m_level = 0;
	m_name = Names::FindName(GetNode());
}
void DRSApp::ConfigFib()
{
	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();
	Ptr<ndn::Name> prefix = Create<ndn::Name>("/broadcast/drsapp");
	fib->Add(*prefix, m_face, 0);
	prefix = Create<ndn::Name>("/"+m_name);
	fib->Add(*prefix, m_face, 0);
}
/*===================================================================================
 * 				Periods
 *==================================================================================*/
void DRSApp::GenServerPeriod()
{
	UniformVariable rand(0, std::numeric_limits<uint32_t>::max());
	NS_LOG_DEBUG("MY LEVEL="<<m_level<<", MY SERVER="<<m_server);
	if (m_server == "" && m_level!=6) {
		SendInterest("/broadcast/drsapp/anyserver/"+std::to_string(m_level)+"/"+std::to_string(rand.GetValue()));
		NS_LOG_DEBUG("I'll wait for "<<Strategy()<<" milliseconds for anyserver");
		Simulator::Schedule(MilliSeconds(Strategy()), &DRSApp::AddLevel, this);
	}
	double delay = rand.GetValue()/std::numeric_limits<uint32_t>::max();
	Simulator::Schedule(Seconds(delay), &DRSApp::GenServerPeriod, this);
}
void DRSApp::AddLevel()
{
	if (m_server!="") return;
	if (m_level==6) return; //top
	NS_LOG_DEBUG("Not received data, so my level ++ to "<<m_level+1);
	m_level++;
}
int DRSApp::Strategy()
{
	switch(m_level) {
		case 0: return 10;
		case 1: return 20;
		case 2: return 40;
		case 3: return 100;
		case 4: return 200;
		case 5: return 400;
	}
}
void DRSApp::AnythingNewInterestPeriod()
{
	SendAnythingNewInterest();
	Simulator::Schedule(Seconds(5.0), &DRSApp::AnythingNewInterestPeriod, this);
}
void DRSApp::GenMessagePeriod()
{
	GenMessage();
	Simulator::Schedule(Seconds(2.0), &DRSApp::GenMessagePeriod, this);
}
void DRSApp::GenMessage()
{
	double time = Simulator::Now().GetSeconds();
	/* DEBUG: must convert, or the double is not the same as you see */
	time = hebi::ConvertDouble(time);
	std::string msg = "I'm " + m_name + " at " + std::to_string(time);
	m_messages[time] = msg;
	NS_LOG_DEBUG("Gen Msg: "<<msg);
	std::string dataname = "/"+m_name+"/drsapp/"+std::to_string(time);
	DRSRecord _record(m_name, time, dataname);
	m_recordContainer.Insert(_record);
	/* send something new */
	SendSomethingNewInterest(time, dataname);
	ProcessPendingInterest();
}
void DRSApp::StopApplication()
{
	ndn::App::StopApplication();
}
/*======================================================================================
 * 				On Interest and On Data
 *=====================================================================================*/
/*
 * 0: ANYSERVER INTEREST:	/broadcast/drsapp/anyserver/<level>
 * 1: ANYTHINGNEW INTEREST:	/<name>/   /anythingnew/<label>
 * 				/<name>/   /anythingnew/<label>:<label>:<label>
 * 2: SOMETHINGNEW INTEREST:	/<name>/   /somethingnew/<label>/<dataname>
 * 3: DATA INTEREST:		/<name>/   /<time>
 */
int DRSApp::GetNameType(std::string name)
{
	hebi::SplitStringByIndent(name, '/');
	std::stringstream ss(name);
	std::string stmp;
	ss>>stmp;
	if (stmp == "broadcast") return 0;
	ss>>stmp>>stmp;
	if (stmp == "anythingnew") return 1;
	else if(stmp == "somethingnew") return 2;
	else return 3;
}
void DRSApp::OnInterest(Ptr<const ndn::Interest> interest)
{
	NS_LOG_DEBUG("Received Interest: "<<interest->GetName());
	ndn::App::OnInterest(interest);
	int type = GetNameType(interest->GetName().toUri());
	switch(type) {
		case 0: ProcessAnyserverInterest(interest); break;
		case 1: ProcessAnythingNewInterest(interest); break;
		case 2: ProcessSomethingNewInterest(interest); break;
		case 3: ProcessDataInterest(interest); break;
	}
}
void DRSApp::OnData(Ptr<const ndn::Data> contentObject)
{
	NS_LOG_DEBUG("Received Data for: "<<contentObject->GetName());
	int type = GetNameType(contentObject->GetName().toUri());
	switch(type) {
		case 0: ProcessAnyserverData(contentObject); break;
		case 1: ProcessAnythingNewData(contentObject); break;
		case 2: ProcessSomethingNewData(contentObject); break;
		case 3: ProcessDataData(contentObject); break;
	}
}
/*==========================================================================================
 * 				Process 4x2
 *=========================================================================================*/
/*------------------------------
 * 	Anyserver
 *-----------------------------*/
void DRSApp::ProcessAnyserverInterest(Ptr<const ndn::Interest> interest)
{
	int level = atoi((hebi::GetSubStringByIndent(interest->GetName().toUri(), '/', 4)).c_str());
	NS_LOG_DEBUG("My Level is "<<m_level<<", received level is "<<level);
	if (level<m_level) {
		SendData(interest->GetName(), m_name);
	}
}
void DRSApp::ProcessAnyserverData(Ptr<const ndn::Data> contentObject)
{
	if (atoi(hebi::GetSubStringByIndent(contentObject->GetName().toUri(), '/', 4).c_str()) == m_level) {
		std::string s = GetStringFromData(contentObject);
		m_server = s;
		NS_LOG_DEBUG("My Server is Set to: "<<m_server);
	} else {
		NS_LOG_DEBUG("Too Late!!!");
	}
}
/*--------------------------------
 * 	Anything New
 *-------------------------------*/
void DRSApp::ProcessAnythingNewInterest(Ptr<const ndn::Interest> interest, std::string exclude)
{
	std::string label = hebi::GetSubStringByIndent(interest->GetName().toUri(), '/', 4);
	int index = m_recordContainer.GetLatestIndexByMultiLabels(label); // suitable for one lable
	if (index == m_recordContainer.GetRecordSize()-1) {
		m_pendingInterest = interest;
	} else {
		std::string xml = m_recordContainer.GetAfterIndexAsXML(index);
		if (exclude!="")
			xml = exclude + xml;
		SendData(interest->GetName(), xml);
	}
}
void DRSApp::ProcessAnythingNewData(Ptr<const ndn::Data> contentObject)
{
	std::string xml = GetStringFromData(contentObject);
	if (xml[0]!='<') {
		std::string name = xml.substr(0, xml.find('<'));
		if (name==m_name) {
			NS_LOG_DEBUG("Excluded!!!");
			return;
		}
		else {
			xml = xml.substr(xml.find('<'));
		}
	}
	/* insert the new record, together with my own label */
	double time = Simulator::Now().GetSeconds();
	time = hebi::ConvertDouble(time);
	std::vector<std::string> vs = m_recordContainer.InsertMultiByXML(xml, m_name, time, m_server);
	/* send anything new interest */
	SendAnythingNewInterest();
	/* process pending interest */
	ProcessPendingInterest();
	/* send data interest */
	BOOST_FOREACH(std::string s, vs) {
		/* DEBUG: don't sent for my own message */
		if (s.find(m_name) == -1)
			SendInterest(s);
	}
}
/*---------------------------------
 * 	Something New
 *--------------------------------*/
void DRSApp::ProcessSomethingNewInterest(Ptr<const ndn::Interest> interest)
{
	int index = hebi::MyStringFinder(interest->GetName().toUri(), '/', 5);
	std::string dataname = interest->GetName().toUri().substr(index);
	/* used for exclude */
	std::string label = hebi::GetSubStringByIndent(interest->GetName().toUri(), '/', 4);
	label = label.substr(0, label.find('_'));
	/* create record */
	double time = Simulator::Now().GetSeconds();
	time = hebi::ConvertDouble(time);
	DRSRecord _record(m_name, time, dataname);
	/* send something new interest */
	SendSomethingNewInterest(time, dataname);
	/* insert record */
	m_recordContainer.Insert(_record);
	/* process pending interest */
	//ProcessPendingInterest();
	if (m_pendingInterest!=NULL) {
		ProcessAnythingNewInterest(m_pendingInterest, label);
	}
	/* send data(time) back */
	SendData(interest->GetName(), std::to_string(time));
	/* send data interest */
	SendInterest(dataname);
}
void DRSApp::ProcessSomethingNewData(Ptr<const ndn::Data> contentObject)
{
	std::string oldlabel = hebi::GetSubStringByIndent(contentObject->GetName().toUri(), '/', 4);
	std::string oldname = oldlabel.substr(0, oldlabel.find('_'));
	double oldtime = atof(oldlabel.substr(oldlabel.find('_')+1).c_str());
	double newtime = atof((GetStringFromData(contentObject)).c_str());
	m_recordContainer.AddLabelByLabel(m_server, newtime, oldname, oldtime);
	/* send anything new interest */
	SendAnythingNewInterest();
}
/*---------------------------------
 * 	Data Interest
 *--------------------------------*/
void DRSApp::ProcessDataInterest(Ptr<const ndn::Interest> interest)
{
	double time = atof((hebi::GetSubStringByIndent(interest->GetName().toUri(), '/', 3)).c_str());
	SendData(interest->GetName(), m_messages[time]);
}
void DRSApp::ProcessDataData(Ptr<const ndn::Data> contentObject)
{
	NS_LOG_DEBUG(GetStringFromData(contentObject));
	/* only log should be ok */
}

/*=========================================================================================
 * 				UTILS
 *========================================================================================*/
void DRSApp::SendAnythingNewInterest()
{
	if (m_server!="") {
		std::string stmp;
		if (m_recordContainer.HasName(m_server)) {
			stmp = m_recordContainer.GetNewestLabelByName(m_server);
		} else {
			stmp = m_recordContainer.GetAllNewestLabels();
		}
		SendInterest("/"+m_server+"/drsapp/anythingnew/"+stmp);
	}
}
void DRSApp::SendSomethingNewInterest(double myTime, std::string dataName)
{
	if (m_server!="") {
		SendInterest("/"+m_server+"/drsapp/somethingnew/"+m_name+"_"+std::to_string(myTime)+"/"+dataName);
	}
}
void DRSApp::ProcessPendingInterest()
{
	if (m_pendingInterest!=NULL) {
		ProcessAnythingNewInterest(m_pendingInterest);
	}
}

std::string DRSApp::GetStringFromData(Ptr<const ndn::Data> contentObject)
{
	int size = contentObject->GetPayload()->GetSize();
	const unsigned char *data = contentObject->GetPayload()->PeekData();
	std::string s(reinterpret_cast<const char*>(data), size);
	return s;
}

/*===================================================================================
 * 				Stable Functions
 *==================================================================================*/
void DRSApp::SendData(const std::string &name, const std::string &msg)
{
	const ndn::Name _name(name);
	SendData(_name, msg);
}
void DRSApp::SendData(const ndn::Name &name, const std::string &msg)
{
	NS_LOG_DEBUG("Send Data for: "<<name);
	Ptr<Packet> packet = Create<Packet>(msg);
	Ptr<ndn::Data> data = Create<ndn::Data>(packet);
	data->SetName(name);
	m_transmittedDatas(data, this, m_face);
	m_face->ReceiveData(data);
}
void DRSApp::SendInterest(const std::string &name)
{
	const ndn::Name _name(name);
	SendInterest(_name);
}
void DRSApp::SendInterest(const ndn::Name &name)
{
	NS_LOG_DEBUG("Send Interest for: "<<name);
	Ptr<ndn::Interest> interest = Create<ndn::Interest>();
	UniformVariable rand(0, std::numeric_limits<uint32_t>::max());
	interest->SetNonce(rand.GetValue());
	interest->SetName(name);
	interest->SetInterestLifetime(Seconds(5.0)); //????
	m_transmittedInterests(interest, this, m_face);
	m_face->ReceiveInterest(interest);
}



} //ns3
