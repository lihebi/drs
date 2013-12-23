#include "chrono-digest-log.h"
#include <boost/foreach.hpp>

namespace ns3 {

boost::tuple<std::string, int> DigestLog::GetNameAndSeq(std::string digest)
{
	if (m_digest_log.find(digest) != m_digests.end()) {
		return m_digest_log[digest];
	} else {
		return boost::make_tuple("", -1);
	}
}
void DigestLog::Insert(std::string digest, std::string name, int seq)
{
	m_digest_log[digest] = boost::make_tuple(name, seq);
}

} //ns3
