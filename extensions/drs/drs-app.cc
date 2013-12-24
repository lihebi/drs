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
	//Simulator::Schedule(Seconds(0), &DRSApp::ServerSelectPeriod, this);
	//Simulator::Schedule(Seconds(2), &DRSApp::GenMessagePeriod, this);
	//Simulator::Schedule(Seconds(0), &DRSApp::AnythingNewInterestPeriod, this);
}
void DRSApp::Init()
{
	m_is_anyserver_sent = false;
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
void DRSApp::StopApplication()
{
	ndn::App::StopApplication();
}
/*
 * 0: ANYSERVER INTEREST:	/broadcast/drsapp/anyserver/<level>
 * 1: ANYTHINGNEW INTEREST:	/<name>/   /anythingnew/<label>
 * 				/<name>/   /anythingnew/<label>:<label>:<label>
 * 2: SOMETHINGNEW INTEREST:	/<name>/   /somethingnew/<name>/<dataname>
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
	int type = GetNameType(contentObject->GetName().toUri());
	switch(type) {
		case 0: ProcessAnyserverData(contentObject); break;
		case 1: ProcessAnythingNewData(contentObject); break;
		case 2: ProcessSomethingNewData(contentObject); break;
		case 3: ProcessDataData(contentObject); break;
	}
}
/*
 * /.../.../anyserver/<level>
 */
void DRSApp::ProcessAnyserverInterest(Ptr<const ndn::Interest> interest)
{
	int level = atoi(hebi::GetSubStringByIndent(interest->GetName().toUri(), '/', 4));
	if (level<m_level)
		SendData(interest->GetName(), m_name);
}
/*
 * /.../.../anythingnew/<label>:<label>:<label>
 * <label> = alice_125264
 */
void DRSApp::ProcessAnythingNewInterest(Ptr<const ndn::Interest> interest)
{
	std::string label = hebi::GetSubStringByIndent(interest->GetName().toUri(), '/', 4);
	int index = m_recordContainer.GetLatestIndexByMultiLabels(label); // suitable for one lable
	if (index == m_recordContainer.GetRecordSize()-1) {
		m_pending_interest = interest;
	} else {
		std::string xml = m_recordContainer.GetAfterIndexAsXML(index);
		SendData(interest->GetName(), xml);
	}
}
/*
 * /<server>/.../somethingnew/<m_name>/<dataname>
 * <dataname>: /<name>/drsapp/<time>
 */
void DRSApp::ProcessSomethingNewInterest(Ptr<const ndn::Interest> interest)
{
	//std::string dataname = hebi::GetSubStringByIndent(interest->GetName().toUri(), '/', 5);
	int index = hebi::MyStringFinder(interest->GetName().toUri(), '/', 5);
	std::string dataname = interest->GetName().toUri().substr(index+1);
	/* send something new interest */
	if (m_server!="")
		SendInterest("/"+m_server+"/drsapp/somethingnew/"+m_name+"/"+dataname);
	/* create record */
	long time = hebi::GetPosixTime_TotalMilli();
	DRSRecord _record = DRSRecord::CreateDRSRecord(m_name, time, dataname);
	/* insert record */
	m_recordContainer.Insert(_record);
	/* process pending interest */
	if (m_pendingInterest!=NULL) {
		ProcessAnythingNewInterest(m_pendingInterest);
	}
	/* send data(time) back */
	SendData(interest->GetName(), std::to_string(time));
	/* send data interest */
	SendInterest(dataname);
}
/*
 * /<myname>/.../<time>
 */
void DRSApp::ProcessDataInterest(Ptr<const ndn::Interest> interest)
{
	long time = atoi(hebi::GetSubStringByIndent(interest->GetName().toUri(), '/', 3));
	SendData(interest->GetName(), m_messages[time]);
}
void DRSApp::ProcessAnyserverData(Ptr<const ndn::Data> contentObject)
{
}
void DRSApp::ProcessAnythingNewData(Ptr<const ndn::Data> contentObject)
{
}
void DRSApp::ProcessSomethingNewData(Ptr<const ndn::Data> contentObject)
{
}
void DRSApp::ProcessDataData(Ptr<const ndn::Data> contentObject)
{
}

/*
 * 1. select server
 * Future work: reselect.
 * 2. send anything new interest
 */
void DRSApp::ServerSelectPeriod()
{
	double delay = 0;
	if (m_server=="") {
		if (m_level == 6) ; //6 is top level
		else if (m_is_anyserver_sent == true) {
			m_level++;
			m_is_anyserver_sent = false;
		} else {
			std::stringstream ss;
			ss<<m_level;
			delay = 3*(double)rand()/RAND_MAX;
			Simulator::Schedule(Seconds(delay), &DRSApp::SendInterest, this, "/broadcast/drsapp/anyserver/"+ss.str());
			m_is_anyserver_sent = true;
		}
	}
	/* heatbeat to detect server fail. I don't intent to implement it. */
	else {
		;
	}
	int wait_time = 3*m_level+delay; // add wait_time by level to ensure the area is increasing
	Simulator::Schedule(Seconds(wait_time), &DRSApp::ServerSelectPeriod, this); //need modify for a configuable time
}
void DRSApp::AnythingNewInterestPeriod()
{
	if (m_server!="")
		SendAnythingNewInterest();
	Simulator::Schedule(Seconds(1.0), &DRSApp::AnythingNewInterestPeriod, this);
}
/*
 * call GenMessage() periodly
 */
void DRSApp::GenMessagePeriod()
{
	double delay=0;
	delay = (double)rand()/RAND_MAX;
	GenMessage();
	Simulator::Schedule(Seconds(delay), &DRSApp::GenMessagePeriod, this);
}
/* 
 * Gen message
 * procedure:
 * 	1. Create Message;
 * 	2. update local data
 * 	3. process all pending interest
 * 	4. send something new interest
 */
void DRSApp::GenMessage()
{
	time_t _time = time(NULL);
	std::string _time_str = ctime(&_time);
	std::string _msg_str = "I'm "+m_name+" at " + _time_str.erase(_time_str.size()-1);
	Message _msg;
	_msg.SetContent(_msg_str);
	int index = m_msg_container.Size();
	if (UpdateLocal(_msg)) {
		NS_LOG_DEBUG("Gened message: "<<_msg_str);
		ProcessAllPendingInterests();
		if (m_server!="") //future work: resend this??
			SendSomethingNewInterest(index);
	}
}
/* 
 * 1. add my own label. Only add label in this function.
 * 2. update local
 */
bool DRSApp::UpdateLocal(Message _msg)
{
	time_t _time = time(NULL);
	std::string _time_str = ctime(&_time);
	_msg.AddLabel(m_name, _time);
	m_msg_container.Add(_msg);
	return true; //future work: return status implies whether the message already exist
}
/*
 * FORMAT: /<client-name>/drsapp/giveme/<index>
 */
void DRSApp::ProcessGiveMeInterest(Ptr<const ndn::Interest> interest, int index)
{
	std::string message_str = m_msg_container.ToXMLAfterIndex(index);
	SendData(interest->GetName().toUri(), message_str);
}
/*
 * check if the interest can be satisfied. If yes, send data.
 * Future Work: set interest timeout
 * Name format: /<server-name>/drsapp/anythingnew/xxx_12341:xxx_43253:
 * Procedure:
 * 	1. for every label, check a index in m_msg_container
 * 	2. get a most recent label
 * 	3. Convert messages after the most recent label into a single XML.
 * 	4. Send the XML.
 */
bool DRSApp::ProcessPendingInterest(Ptr<const ndn::Interest> interest)
{
	std::string labels = interest->GetName().toUri();
	std::cout<<labels<<std::endl;
	SplitString(labels, '/');
	std::stringstream ss(labels);
	int index=-1;
	if(!(ss>>labels>>labels>>labels>>labels))
		index = 0; // the client has nothing, so send all to him
	SplitString(labels, ':');
	ss.clear();
	ss.str(labels);
	std::string stmp, name, label;
	bool no_record = true;
	while(ss>>stmp) {
		if (index==0) break;
		SplitString(stmp, '_');
		std::stringstream ss2(stmp);
		ss2>>name>>label;
		time_t labelTime = static_cast<time_t>(atoi(label.c_str()));
		int i = m_msg_container.GetIndexNewerThan(name, labelTime);
		if (i!=-2)
			no_record = false;
		if (i>index) 
			index = i;
	}
	if (no_record) index = 0; //no even one time fit, so send all to him
	if (index != -1) {
		std::string message_str = m_msg_container.ToXMLAfterIndex(index);
		SendData(interest->GetName().toUri(), message_str);
		return true;
	}
	return false;
}
/*
 * 1. go over the m_pendingInterests
 * 2. if one is processed and sent, remove it
 */
void DRSApp::ProcessAllPendingInterests()
{
	for (int i=0;i<m_pendingInterests.size();i++) {
		if (ProcessPendingInterest(m_pendingInterests[i]))
			m_pendingInterests.erase(m_pendingInterests.begin()+i);
	}
}
/*
 * send something new based on latest labels
 * Entry: index is the new message's start index in m_msg_container. The server will send back the index, so the client can use it to return all messages after this index.
 * FORMAT:
 * 	/<server-name>/drsapp/somethingnew/<my-name>/<random_number>
 * Future work:
 * 	1. as below
 * 	2. latest label? or the triggered label by a parameter?
 * 	3. what if don't receive giveme new interest?
 */
void DRSApp::SendSomethingNewInterest(int index)
{
	std::string name = "/"+m_server+"/drsapp/somethingnew/"+m_name;
	name += "/"+std::to_string(index);
	SendInterest(name);
}
/*
 * send anything new to server
 * FORMAT:
 * 	/<server-name>/drsapp/anythingnew/<time-label>
 */
void DRSApp::SendAnythingNewInterest()
{
	std::string name = "/"+m_server+"/drsapp/anythingnew";
	std::string latestLabels = m_msg_container.GetLatestLabels(m_server);
	name += "/"+latestLabels;
	SendInterest(name);
}
/*
 * send give me interests to client
 * FORMAT:
 * 	/<client-name>/drsapp/giveme/<index>
 */
void DRSApp::SendGiveMeInterest(std::string clientName, std::string index)
{
	std::string name = "/"+clientName+"/drsapp/giveme";
	name += "/"+index;
	SendInterest(name);
}

void DRSApp::ProcessReceivedMessages(const Ptr<const ndn::Data> contentObject, bool need_send_somethingnew)
{
	int index = m_msg_container.Size();
	int size = contentObject->GetPayload()->GetSize();
	const unsigned char *data = contentObject->GetPayload()->PeekData();
	pugi::xml_document doc;
	doc.load_buffer(data, size);
	//doc.save(std::cout);
	pugi::xml_node node = doc.child("packet").child("message"); //first child??
	bool is_updated = false;
	for(;node;node=node.next_sibling()) {
		Message _msg;
		std::string _content(node.child("content").text().get());
		_msg.SetContent(_content);
		pugi::xml_node label = node.child("label").child("li");
		for (;label;label=label.next_sibling()) {
			std::string _name(label.child("name").text().get());
			time_t _time = static_cast<time_t>(label.child("time").text().as_int());
			_msg.AddLabel(_name, _time);
		}
		is_updated |= UpdateLocal(_msg);
	}
	if (is_updated) {
		ProcessAllPendingInterests();
		if (need_send_somethingnew) //if this update is triggered by the server, obviously don't need send Something New Interest
			SendSomethingNewInterest(index);
	}
}
void DRSApp::ProcessAnyServerData(const Ptr<const ndn::Data> &contentObject)
{
	int size = contentObject->GetPayload()->GetSize();
	const unsigned char *data = contentObject->GetPayload()->PeekData();
	std::string msg(reinterpret_cast<const char *>(data), size);
	m_server = msg;
	NS_LOG_DEBUG("My server changed to "<<m_server);
}

void DRSApp::SendInterest(std::string name)
{
	Ptr<ndn::Name> prefix = Create<ndn::Name>(name);
	Ptr<ndn::Interest> interest = Create<ndn::Interest>();
	UniformVariable rand(0, std::numeric_limits<uint32_t>::max());
	interest->SetNonce(rand.GetValue());
	interest->SetName(prefix);
	interest->SetInterestLifetime(Seconds(1.0));
	NS_LOG_DEBUG("Sending Interest packet for " << *prefix);
	m_transmittedInterests(interest, this, m_face);
	m_face->ReceiveInterest(interest);
}
/* 
 * substitute indent with space
 */
void DRSApp::SplitString(std::string &name, char indent)
{
	int index;
	for(;;) {
		index = name.find(indent);
		if (index == -1)
			break;
		else {
			name[index] = ' ';
		}
	}
}

void ChronoApp::SendData(const std::string &name, const std::string &msg)
{
	SendData(Create<ndn::Name>(name), msg);
}
void ChronoApp::SendData(const Ptr<ndn::Name> &name, const std::string &msg)
{
	Ptr<Packet> packet = Create<Packet>(msg);
	Ptr<ndn::Data> data = Create<ndn::Data>(packet);
	data->SetName(name);
	m_transmittedDatas(data, this, m_face);
	m_face->ReceiveData(data);
}
void ChronoApp::SendInterest(cosnt std::string &name)
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



} //ns3
