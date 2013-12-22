

namespace ns3 {

class Digest
{
public:
	void Insert(std::string name, int seq);
	std::string CalRootDigest();

private:
	std::map<std::string, std::string> m_digest;

};



} //ns3
