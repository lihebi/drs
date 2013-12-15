/*
 * Need Improve:
 * 	1. use boost::posix_time instead of time(NULL)
 * Next Step:
 * 	1. add tracer
 * 	2. do experiment
 * 	3. implement ChronoSync
 */
#include "chat-app.h"
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

NS_LOG_COMPONENT_DEFINE("ChatApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(ChatApp);

TypeId ChatApp::GetTypeId()
{
	static TypeId tid = TypeId("ChatApp")
		.SetParent<ndn::App>()
		.AddConstructor<ChatApp>()
		;
	return tid;
}
/*
 * 1. Init(): set private variables
 * 2. Config(): set fib: 1. /broadcast/chatapp 2. /<m_name>/chatapp
 * 3. start Period: check and gen server
 * 4. start Gen Message Period: gen message
 */
void ChatApp::StartApplication()
{
	NS_LOG_DEBUG(Simulator::Now().GetSeconds());
	ndn::App::StartApplication();
	Init();
	Config();
	Simulator::Schedule(Seconds(0), &ChatApp::ServerSelectPeriod, this);
	Simulator::Schedule(Seconds(2), &ChatApp::GenMessagePeriod, this);
	Simulator::Schedule(Seconds(0), &ChatApp::AnythingNewInterestPeriod, this);
}
void ChatApp::Init()
{
	m_is_anyserver_sent = false;
	m_server = "";
	m_level = 0;
	m_name = Names::FindName(GetNode());
}
void ChatApp::Config()
{
	Ptr<ndn::Fib> fib = GetNode()->GetObject<ndn::Fib>();
	Ptr<ndn::Name> prefix = Create<ndn::Name>("/broadcast/chatapp");
	fib->Add(*prefix, m_face, 0);
	prefix = Create<ndn::Name>("/"+m_name);
	fib->Add(*prefix, m_face, 0);
}

/*
 * 1. select server
 * Future work: reselect.
 * 2. send anything new interest
 */
void ChatApp::ServerSelectPeriod()
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
			Simulator::Schedule(Seconds(delay), &ChatApp::SendInterest, this, "/broadcast/chatapp/anyserver/"+ss.str());
			m_is_anyserver_sent = true;
		}
	}
	/* heatbeat to detect server fail. I don't intent to implement it. */
	else {
		;
	}
	int wait_time = 3*m_level+delay; // add wait_time by level to ensure the area is increasing
	Simulator::Schedule(Seconds(wait_time), &ChatApp::ServerSelectPeriod, this); //need modify for a configuable time
}
void ChatApp::AnythingNewInterestPeriod()
{
	if (m_server!="")
		SendAnythingNewInterest();
	Simulator::Schedule(Seconds(1.0), &ChatApp::AnythingNewInterestPeriod, this);
}
/*
 * call GenMessage() periodly
 */
void ChatApp::GenMessagePeriod()
{
	double delay=0;
	delay = (double)rand()/RAND_MAX;
	GenMessage();
	Simulator::Schedule(Seconds(delay), &ChatApp::GenMessagePeriod, this);
}
/* 
 * Gen message
 * procedure:
 * 	1. Create Message;
 * 	2. update local data
 * 	3. process all pending interest
 * 	4. send something new interest
 */
void ChatApp::GenMessage()
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
bool ChatApp::UpdateLocal(Message _msg)
{
	time_t _time = time(NULL);
	std::string _time_str = ctime(&_time);
	_msg.AddLabel(m_name, _time);
	m_msg_container.Add(_msg);
	return true; //future work: return status implies whether the message already exist
}
/*
 * process interest
 */
void ChatApp::OnInterest(Ptr<const ndn::Interest> interest)
{
	ndn::App::OnInterest(interest);
	std::string name = interest->GetName().toUri();
	NS_LOG_DEBUG("Received Interest packet for " << name);
	SplitString(name, '/');
	std::stringstream ss(name);
	std::string stmp;
	ss>>stmp;
	if (stmp == "broadcast") {
		ss >> stmp;
		if (stmp == "chatapp") {
			ss >> stmp;
			/* anyserver */
			if (stmp == "anyserver") {
				int level;
				ss >> level;
				NS_LOG_DEBUG("Level: " << level << " My Level: " << m_level);
				if (level<m_level) {
					SendData(interest->GetName().toUri(), m_name);
				}
			}
		}
	} else if (stmp == m_name) {
		ss >> stmp;
		if (stmp == "chatapp") {
			ss >> stmp;
			/* something new */
			if (stmp == "somethingnew") {
				ss >> stmp;
				std::string index;
				ss >> index;
				SendGiveMeInterest(stmp, index);
			}
			/* anything new */
			else if (stmp == "anythingnew") {
				if (!ProcessPendingInterest(interest))
					m_pendingInterests.push_back(interest);
			}
			/* give me */
			else if (stmp == "giveme") {
				ss>>stmp;
				ProcessGiveMeInterest(interest, atoi(stmp.c_str()));
			}
		}
	}
}
/*
 * FORMAT: /<client-name>/chatapp/giveme/<index>
 */
void ChatApp::ProcessGiveMeInterest(Ptr<const ndn::Interest> interest, int index)
{
	std::string message_str = m_msg_container.ToXMLAfterIndex(index);
	SendData(interest->GetName().toUri(), message_str);
}
/*
 * check if the interest can be satisfied. If yes, send data.
 * Future Work: set interest timeout
 * Name format: /<server-name>/chatapp/anythingnew/xxx_12341:xxx_43253:
 * Procedure:
 * 	1. for every label, check a index in m_msg_container
 * 	2. get a most recent label
 * 	3. Convert messages after the most recent label into a single XML.
 * 	4. Send the XML.
 */
bool ChatApp::ProcessPendingInterest(Ptr<const ndn::Interest> interest)
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
void ChatApp::ProcessAllPendingInterests()
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
 * 	/<server-name>/chatapp/somethingnew/<my-name>/<random_number>
 * Future work:
 * 	1. as below
 * 	2. latest label? or the triggered label by a parameter?
 * 	3. what if don't receive giveme new interest?
 */
void ChatApp::SendSomethingNewInterest(int index)
{
	std::string name = "/"+m_server+"/chatapp/somethingnew/"+m_name;
	name += "/"+std::to_string(index);
	SendInterest(name);
}
/*
 * send anything new to server
 * FORMAT:
 * 	/<server-name>/chatapp/anythingnew/<time-label>
 */
void ChatApp::SendAnythingNewInterest()
{
	std::string name = "/"+m_server+"/chatapp/anythingnew";
	std::string latestLabels = m_msg_container.GetLatestLabels(m_server);
	name += "/"+latestLabels;
	SendInterest(name);
}
/*
 * send give me interests to client
 * FORMAT:
 * 	/<client-name>/chatapp/giveme/<index>
 */
void ChatApp::SendGiveMeInterest(std::string clientName, std::string index)
{
	std::string name = "/"+clientName+"/chatapp/giveme";
	name += "/"+index;
	SendInterest(name);
}
/*
 * process on data
 */
void ChatApp::OnData(Ptr<const ndn::Data> contentObject)
{
	std::string name = contentObject->GetName().toUri();
	NS_LOG_DEBUG("Receiving Data packet for " << name);
	NS_LOG_DEBUG("DATA received for name " << name);
	SplitString(name, '/');
	std::stringstream ss(name);
	std::string stmp;
	ss >> stmp;
	if (stmp == "broadcast") {
		ss >> stmp;
		if (stmp == "chatapp") {
			ss >> stmp;
			/* anyserver */
			if (stmp == "anyserver") {
				ProcessAnyServerData(contentObject);
			}
		}
	}
	else if (stmp == m_name) {
		ss >> stmp;
		if (stmp == "chatapp") {
			ss >> stmp;
			/* something new, never receive
			 * future work: resend somethingnew */
			if (stmp == "somethingnew") {
				;
			}
			/* anything new */
			else if (stmp == "anythingnew") {
				ProcessReceivedMessages(contentObject, false);
			}
			/* give me */
			else if (stmp == "giveme") {
				ProcessReceivedMessages(contentObject, true);
			}
		}
	}
}
void ChatApp::ProcessReceivedMessages(const Ptr<const ndn::Data> contentObject, bool need_send_somethingnew)
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
void ChatApp::ProcessAnyServerData(const Ptr<const ndn::Data> &contentObject)
{
	int size = contentObject->GetPayload()->GetSize();
	const unsigned char *data = contentObject->GetPayload()->PeekData();
	std::string msg(reinterpret_cast<const char *>(data), size);
	m_server = msg;
	NS_LOG_DEBUG("My server changed to "<<m_server);
}

void ChatApp::StopApplication()
{
	NS_LOG_DEBUG("Stoped");
	ndn::App::StopApplication();
	NS_LOG_DEBUG("Stoped");
}
void ChatApp::SendInterest(std::string name)
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
void ChatApp::SplitString(std::string &name, char indent)
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
/* 
 * Entry:
 * 	1. interest name
 * 	2. message as string
 */
void ChatApp::SendData(std::string name, const std::string msg)
{
	NS_LOG_DEBUG("Sending data for " << name);
	Ptr<Packet> packet = Create<Packet>(msg);
	Ptr<ndn::Data> data = Create<ndn::Data>(packet);
	data->SetName(Create<ndn::Name>(name));
	m_transmittedDatas(data, this, m_face);
	m_face->ReceiveData(data);
}

} //ns3
