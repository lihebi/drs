#ifndef _CHATAPP_H_
#define _CHATAPP_H_
#include "ns3/ndn-app.h"
#include "message-container.h"
namespace ns3 {

class ChatApp : public ndn::App
{
public:
	static TypeId GetTypeId();
	virtual void StartApplication();
	virtual void StopApplication();
	virtual void OnInterest(Ptr<const ndn::Interest> interest);
	virtual void OnData(Ptr<const ndn::Data> contentObject);
	virtual void Init();
	virtual void Config();
	virtual void Period();
	virtual void SplitString(std::string &name, char indent);
	virtual void GenMessagePeriod();
private:
	void ProcessAllPendingInterests();
	bool ProcessPendingInterest(Ptr<const ndn::Interest> interest);

	void ProcessAnyServerData(const Ptr<const ndn::Data> &contentObject);
	void ProcessReceivedMessages(const Ptr<const ndn::Data> contentObject, bool need_send_somethingnew);

	void SendInterest(std::string name);
	void SendAnythingNewInterest();
	void SendSomethingNewInterest();
	void SendGiveMeInterest(std::string clientName);

	void SendData(std::string name, const std::string msg);
	void GenMessage();
	bool UpdateLocal(Message _msg);

	std::string m_server;
	bool m_is_anyserver_sent;
	std::string m_name;
	int m_level;
	MessageContainer m_msg_container;
	std::vector<Ptr<const ndn::Interest> > m_pendingInterests;
};

} //end namespace ns3

#endif
