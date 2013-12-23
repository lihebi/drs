#include "chrono-digest-tree.h"
#include <algorithm>
#include "pugixml.h"
#include "hebi.h"

namespace ns3 {

/*
 * Insert into m_state
 * UpdateRootDigest
 * return old digest
 * On Error: return ""
 */
std::string DigestTree::InsertAndUpdate(std::string name, int seq)
{
	if (m_state[name]>=seq) return "";
	std::string old = m_root_digest;
	m_state[name] = seq;
	UpdateRoot();
	return old;
}
/*
 * Format:
 * 	<li> <name> ... </name> <seq> ... </seq> </li>
 * 	<li> <name> ... </name> <seq> ... </seq> </li>
 */
std::string DigestTree::GetCurrentStateAsXML()
{
	pugi::xml_document doc;
	doc.load("");
	std::map<std::string, int>::const_iterator iter;
	for (iter=m_state.begin();iter!=m_state.end();iter++) {
		pugi::xml_node node = doc.append_child("li");
		node.append_child("name").text().set(iter.first);
		node.append_child("seq").text().set(std::to_string(iter.second));
	}
	hebi::xml_string_writer writer;
	doc.save(writer);
	return writer.result;
}
/*
 * Entry: xml (format as above)
 * Return: (name, seq) pairs to be fetch
 */
std::vector<boost::tuple<std::string, int> > CompareCurrentStateByXML(std::string xml)
{
	pugi::xml_document doc;
	doc.load(xml);
	pugi::xml_node node = doc.child("li");
	std::string name;
	int seq;
	vector<boost::tuple<std::string, int> > v;
	for (;node;node=node.next_sibling()) {
		name = node.child("name").text().get();
		seq = node.child("seq").text().as_int();
		for (int i=m_state[name];i<seq;i++)
			v.push_back(boost::make_tuple(name, i));
	}
}
std::string DigestTree::GetRootDigest()
{
	return m_root_digest;
}
void DigestTree::UpdateRoot()
{
	std::string s;
	std::map<std::string, int>::const_iterator iter;
	for (iter=m_state.begin();iter!=m_state.end();iter++) {
		s += iter.first + std::to_string(iter.second);
	}
	/* need a large function */
	m_root_digest = hash(s);
}


} //ns3
