#include "digest-log.h"
#include <boost/foreach.hpp>
#include "pugixml.h"
#include "message.h"


namespace ns3 {

int DigestLog::FindDigest(std::string digest)
{
	for (int i=0;i<m_digests.size();i++) {
		if (digest == m_digests[i]) {
			return i;
		}
	}
	return -1;
}
/*
 * XML Format:
 * 	<packet>
 * 		<li> <name> ... </name> <seq> 123 </seq> </li>
 * 		<li> <name> ... </name> <seq> 123 </seq> </li>
 * 		<li> <name> ... </name> <seq> 123 </seq> </li>
 * 	</packet>
 */
std::string DigestLog::GetFromIndexAsXML(int index)
{
	pugi::xml_document doc;
	doc.load("<packet></packet>");
	pugi::xml_node node = doc.child("packet");
	for (int i=index;i<m_eventName.size();i++) {
		pugi::xml_node n = node.append_child("li");
		n.append_child("name").text().set(m_eventName[i].c_str());
		n.append_child("seq").text().set(m_eventSeq[i]);
	}
	xml_string_writer writer;
	doc.save(writer);
	return writer.result;
}

} //ns3
