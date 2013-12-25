#ifndef _CHATAPP_H_
#define _CHATAPP_H_
#include "ns3/ndn-app.h"
#include "drs-record-container.h"
#include "drs-record.h"
#include "ns3/ndn-app-face.h"
#include "ns3/random-variable.h"
namespace ns3 {

class DRSApp : public ndn::App
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
	/* periods */
	void GenServerPeriod();
	void AddLevel();
	int Strategy();
	void AnythingNewInterestPeriod();
	void GenMessagePeriod();
	void GenMessage();

	/* process 4x2 */
	void ProcessAnyserverInterest(Ptr<const ndn::Interest> interest);
	void ProcessAnythingNewInterest(Ptr<const ndn::Interest> interest);
	void ProcessSomethingNewInterest(Ptr<const ndn::Interest> interest);
	void ProcessDataInterest(Ptr<const ndn::Interest> interest);
	void ProcessAnyserverData(Ptr<const ndn::Data> contentObject);
	void ProcessAnythingNewData(Ptr<const ndn::Data> contentObject);
	void ProcessSomethingNewData(Ptr<const ndn::Data> contentObject);
	void ProcessDataData(Ptr<const ndn::Data> contentObject);
	/* utils */
	int GetNameType(std::string name);
	std::string GetStringFromData(Ptr<const ndn::Data> contentObject);
	void SendAnythingNewInterest();
	void SendSomethingNewInterest(long myTime, std::string dataName);
	void ProcessPendingInterest();
	/* stable functions */
	void SendData(const std::string &name, const std::string &msg);
	void SendData(const ndn::Name &name, const std::string &msg);
	void SendInterest(const std::string &name);
	void SendInterest(const ndn::Name &name);


	/* new */
	int m_level;
	std::string m_server;
	std::string m_name;
	DRSRecordContainer m_recordContainer;
	std::map<long, std::string> m_messages;
	Ptr<const ndn::Interest> m_pendingInterest; // may there be many different pending interests?
};

} //end namespace ns3

#endif
