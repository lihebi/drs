
#include "chrono-message.h"

namespace ns3 {


void ChronoMessages::Push(std::string name, int seq, std::string content)
{
	if (m_messages.find(name)) {
		m_messages[name][seq] = content;
	} else {
		m_messages[name] = new std::map;
		m_messages[name][seq] = content;
	}
}
std::string ChronoMessages::Get(std::string name, int seq);
{
	return m_messages[name][seq];
}


}
