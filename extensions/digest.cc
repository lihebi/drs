#include "digest.h"
#include <algorithm>

namespace ns3 {

void Digest::Insert(std::string name, int seq)
{
	m_digest[name] = myhash(name, seq);
}
std::string Digest::CalRootDigest()
{
	for (std::map<std::string, std::string> iter=m_digest.begin();iter!=m_digest.end();iter++) {
		myhash(iter);
	}
	return ;
}


} //ns3
