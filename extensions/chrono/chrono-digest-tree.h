#include<string>
#include<map>
#include<vector>
#include<boost/tuple/tuple.hpp>
/*
 * FILENAME: chrono-digest-tree.h
 */


namespace ns3 {

class DigestTree
{
public:
	std::string InsertAndUpdate(std::string name, int seq);
	std::string GetCurrentStateAsXML();
	std::vector<boost::tuple<std::string, int> > CompareCurrentStateByXML(std::string xml);
	std::string GetRootDigest();
	void InitRootDigest();
	int GetSeqByName(std::string name);

private:
	void UpdateRootDigest();
	std::string SHA256(std::string s);
	void HashPrinter(char *dst_buf, unsigned char *src_buf, int size);

	std::string m_root_digest;
	/* name ---> seq */
	std::map<std::string, int> m_state;

};



} //ns3
