/*
 * FILENAME: chrono-digest-log.cc
 */
#include "chrono-digest-log.h"
#include <boost/foreach.hpp>

namespace ns3 {

/*
 * Get Name and Seq according to digest
 * Call HasDigest First!!!
 */
boost::tuple<std::string, int> DigestLog::GetNameAndSeq(std::string digest)
{
	return m_digest_log[digest];
}
/* 
 * insert: digest ---> (name, seq)
 */
void DigestLog::Insert(std::string digest, std::string name, int seq)
{
	m_digest_log[digest] = boost::make_tuple(name, seq);
}
/*
 * Has Digest?
 */
bool DigestLog::HasDigest(std::string digest)
{
	return (m_digest_log.find(digest)!=m_digest_log.end());
}

} //ns3
