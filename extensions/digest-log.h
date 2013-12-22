/*
 * Digest, EventName, EventSeq
 *
 */
#include <boost/tuple/tuple.hpp>

namespace ns3 {

class DigestLog {
public:
	/*
	 * method
	 */
	int FindDigest(std::string);
	std::string GetFromIndexAsXML(int index);





	/*
	 * data structure
	 */
	std::vector<std::string> m_digests;
	std::vector<std::string> m_eventName;
	std::vector<int> m_eventSeq;
};

} //ns3
