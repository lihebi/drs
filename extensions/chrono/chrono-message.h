#include<string>
#include<map>
/*
 * FILENAME: chrono-message.h
 */
namespace ns3 {

class ChronoMessage {
public:
	bool Insert(int seq, std::string content);
	std::string Get(int seq);

private:
	/*
	 * seq ---> content
	 * Only store my own generated messages. Cache everyone's message will be so large. Can output it into a log file.
	 */
	std::map<int, std::string> m_messages;

};




}
