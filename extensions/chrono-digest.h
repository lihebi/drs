

namespace ns3 {

class Digest
{
public:
	void Insert(std::string name, int seq);
	std::string GetCurrentStateAsXML();
	std::string GetRootDigest();

private:
	/* called by Insert() */
	void UpdateRoot();

	std::string m_root_digest;
	/* name ---> seq */
	std::map<std::string, int> m_state;

};



} //ns3
