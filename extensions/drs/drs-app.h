#ifndef _CHATAPP_H_
#define _CHATAPP_H_
#include "ns3/ndn-app.h"
#include "drs-record-container.h"
#include "drs-record.h"
namespace ns3 {

class ChatApp : public ndn::App
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

	void ProcessAnyserverInterest(Ptr<const ndn::Interest> interest);
	void ProcessAnythingNewInterest(Ptr<const ndn::Interest> interest);
	void ProcessSomethingNewInterest(Ptr<const ndn::Interest> interest);
	void ProcessDataInterest(Ptr<const ndn::Interest> interest);
	void ProcessAnyserverData(Ptr<const ndn::Data> contentObject);
	void ProcessAnythingNewData(Ptr<const ndn::Data> contentObject);
	void ProcessSomethingNewData(Ptr<const ndn::Data> contentObject);
	void ProcessDataData(Ptr<const ndn::Data> contentObject);

	void ServerSelectPeriod();
	void GenMessagePeriod();
	void AnythingNewInterestPeriod();


	void ProcessAllPendingInterests();
	bool ProcessPendingInterest(Ptr<const ndn::Interest> interest);
	void ProcessGiveMeInterest(Ptr<const ndn::Interest> interest, int index);

	void ProcessAnyServerData(const Ptr<const ndn::Data> &contentObject);
	void ProcessReceivedMessages(const Ptr<const ndn::Data> contentObject, bool need_send_somethingnew);

	void SendInterest(std::string name);
	void SendAnythingNewInterest();
	void SendSomethingNewInterest(int index);
	void SendGiveMeInterest(std::string clientName, std::string index);

	void SendData(std::string name, const std::string msg);
	void GenMessage();
	bool UpdateLocal(Message _msg);

	virtual void SplitString(std::string &name, char indent);

	std::string m_server;
	bool m_is_anyserver_sent;
	std::string m_name;
	int m_level;
	MessageContainer m_msg_container;
	std::vector<Ptr<const ndn::Interest> > m_pendingInterest;

	DRSRecordContainer m_recordContainer;
	std::map<long, std::string> m_messages;
};

} //end namespace ns3

#endif
