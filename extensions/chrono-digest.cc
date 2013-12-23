#include "chrono-digest.h"
#include <algorithm>
#include "pugixml.h"

namespace ns3 {

void Digest::Insert(std::string name, int seq)
{
	m_state[name] = seq;
	UpdateRoot();
}
struct xml_string_writer : pugi::xml_writer
{
	std::string result;
	virtual void write(const void *data, size_t size) {
		result += std::string(static_cast<const char*>(data), size);
	}
};
/*
 * Format:
 * 	<packet>
 * 		<li> <name> ... </name> <seq> ... </seq> </li>
 * 		<li> <name> ... </name> <seq> ... </seq> </li>
 * 		<li> <name> ... </name> <seq> ... </seq> </li>
 * 	</packet>
 */
std::string Digest::GetCurrentStateAsXML()
{
	pugi::xml_document doc;
	pugi.load("<packet></packet>");
	std::map<std::string, int>::const_iterator iter;
	for (iter=m_state.begin();iter!=m_state.end();iter++) {
		pugi::xml_node node = doc.append_child("li");
		node.append_child("name").text().set(iter.first);
		node.append_child("seq").text().set(std::to_string(iter.second));
	}
	xml_string_writer writer;
	doc.save(writer);
	return writer.result;
}
std::string Digest::GetRootDigest()
{
	return m_root_digest;
}
void Digest::UpdateRoot()
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
