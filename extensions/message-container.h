#ifndef MESSAGE_CONTAINER_H_
#define MESSAGE_CONTAINER_H_
#include "message.h"
#include<vector>

namespace ns3 {

class MessageContainer
{
public:
	bool Add(Message msg);
	Message GetMessage(int index);
	std::string GetLatestLabels(std::string name);
	std::string GetLatestLabels();
	size_t Size();
	void AddIndex(std::string name, int index);
	std::vector<int> GetIndexVector(std::string);
	int GetIndexNewerThan(std::string name, time_t theTime);
	std::string ToXMLAfterIndex(int index);
private:
	std::vector<Message> m_messages;
	/* 
	 * name ----> index vector
	 */
	std::map<std::string, std::vector<int> > m_record;

}; //MessageContainer



} //ns3
#endif
