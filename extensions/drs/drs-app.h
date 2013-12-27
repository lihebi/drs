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
	void ProcessAnythingNewInterest(Ptr<const ndn::Interest> interest, std::string exclude="");
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
	void SendSomethingNewInterest(double myTime, std::string dataName);
	void ProcessPendingInterest();
	double GetRand();
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
	std::map<double, std::string> m_messages;
	Ptr<const ndn::Interest> m_pendingInterest; // may there be many different pending interests?

	void ReadEnv();
	int m_env_GSPSO;
	int m_env_GMPSO;
	int m_env_ANPSO;

	int m_env_GSPSF;
	int m_env_GMPSF;
	int m_env_ANPSF;

	int m_env_ANPO;
	int m_env_GMPO;
	int m_env_GSPO;

	int m_env_ANPF;
	int m_env_GMPF;
	int m_env_GSPF;

	int m_env_ILT;
	int m_env_TL;
	int m_env_S0;
	int m_env_S1;
	int m_env_S2;
	int m_env_S3;
	int m_env_S4;
	int m_env_S5;
};

} //end namespace ns3

#endif
