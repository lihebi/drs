/*
 * Digest, EventName, EventSeq
 *
 */
#include <boost/tuple/tuple.hpp>

namespace ns3 {

class DigestLog {
public:
	boost::tuple<std::string, int> GetNameAndSeq(std::string digest);
	void Insert(std::string digest, std::string name, int seq);

private:
	/*
	 * digest ----> (name, seq)
	 * Only Consider one digest connect with one event pair
	 */
	std::map<std::string, boost::tuple<std::string, int> > m_digest_log;


};

} //ns3
