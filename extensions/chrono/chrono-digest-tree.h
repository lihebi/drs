

namespace ns3 {

class DigestTree
{
public:
	std::string InsertAndUpdate(std::string name, int seq);
	std::string GetCurrentStateAsXML();
	std::vector<boost::tuple<std::string, int> > CompareCurrentStateByXML(std::string xml);
	std::string GetRootDigest();

private:
	void UpdateRootDigest();

	std::string m_root_digest;
	/* name ---> seq */
	std::map<std::string, int> m_state;

};



} //ns3
