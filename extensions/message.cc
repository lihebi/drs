#include "message.h"
#include<ctime>
#include<iostream>
#include "pugixml.h"

namespace ns3 {

/*
 * set m_content
 */
void Message::SetContent(const std::string& content)
{
	m_content = content;
}
/*
 * add to m_label
 */
void Message::AddLabel(const std::string &name, time_t theTime)
{
	m_label[name] = theTime;
}
/*
 * get m_content
 */
const std::string& Message::GetContent()
{
	return m_content;
}
/*
 * get a person's time
 */
time_t Message::GetTime(const std::string &name)
{
	return m_label[name];
}
/*
 * get m_label
 */
const std::map<std::string, time_t> Message::GetMap()
{
	return m_label;
}
/*
 * return a single XML representing this Message
 * XML format:
 * 	<message>
 * 		<content> ... </content>
 * 		<label>
 * 			<li>
 * 				<name> ... </name>
 * 				<time> ... </time>
 * 			</li>
 * 			<li>
 * 				...
 * 			</li>
 * 		</label>
 * 	</message>
 */
std::string Message::ToXML()
{
	pugi::xml_document doc;
	doc.load("<message><content></content><label></label></message>");
	doc.child("message").child("content").text().set(m_content.c_str());
	pugi::xml_node label = doc.child("message").child("label");
	pugi::xml_node li;
	for (std::map<std::string, time_t>::const_iterator iter=m_label.begin();iter!=m_label.end();iter++) {
		li = label.append_child("li");
		li.append_child("name").text().set(iter->first.c_str());
		li.append_child("time").text().set(static_cast<const int>(iter->second));
	}
	xml_string_writer writer;
	doc.save(writer);
	return writer.result;
}

}
