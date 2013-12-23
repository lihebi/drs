
#include "chrono-message.h"

namespace ns3 {


std::string ChronoMessage::Get(int seq);
{
	return m_messages[seq];
}
/*
 * not use return value yet
 */
bool ChronoMessage::Insert(int seq, std::string content)
{
	m_messages[seq] = content;
	return true;
}


} //ns3
