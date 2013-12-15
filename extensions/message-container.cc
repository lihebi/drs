#include "message-container.h"
#include "message.h"
#include "pugixml.h"
#include<algorithm>
#include<sstream>

namespace ns3 {

/*
 * 1. add message to m_message
 * 2. add index to m_record
 */
bool MessageContainer::Add(Message msg)
{
	m_messages.push_back(msg);
	const std::map<std::string, time_t> _label = msg.GetMap();
	for (std::map<std::string, time_t>::const_iterator iter = _label.begin(); iter!=_label.end();iter++) {
		AddIndex(iter->first, m_messages.size()-1); //concurrent problem
	}
}
/*
 * get message by index
 */
Message MessageContainer::GetMessage(int index)
{
	return m_messages[index];
}
/*
 * 1. get server's latest time
 * 2. if no record, get global latest labels
 */
std::string MessageContainer::GetLatestLabels(std::string name)
{
	if (m_record.find(name) != m_record.end()) {
		std::vector<int> v=m_record[name];
	std::cout<<v.size()<<std::endl;
		Message _msg = m_messages[v[v.size()-1]];
		time_t _time = _msg.GetTime(name);
		return name+"_"+std::to_string(_time);
	} else {
		return GetLatestLabels();
	}
}
/*
 * get all latest labels for everyone as the following format
 * 	xxx_1421:xxx_32:
 */
std::string MessageContainer::GetLatestLabels()
{
	std::map<std::string, std::vector<int> >::const_iterator iter;
	std::vector<std::string> vs;
	for (iter=m_record.begin();iter!=m_record.end();iter++) {
		std::string name = iter->first;
		std::vector<int> v = iter->second;
		int index = v[v.size()-1];
		time_t _time = m_messages[index].GetTime(name);
		vs.push_back(name+"_"+std::to_string(_time));
		//ss<<name<<"_"<<_time<<":"; //can not make sure the latest index is the latest time label
	}
	sort(vs.begin(), vs.end());
	std::stringstream ss;
	for (int i=0;i<vs.size();i++) {
		ss<<vs[i]<<":";
	}
	return ss.str();
}
/*
 * return size of m_message
 */
size_t MessageContainer::Size()
{
	return m_messages.size();
}
/*
 * add index to m_record
 */
void MessageContainer::AddIndex(std::string name, int index)
{
	m_record[name].push_back(index);
}
/*
 * get index vector for a person
 */
std::vector<int> MessageContainer::GetIndexVector(std::string name)
{
	return m_record[name];
}
/*
 * get m_record's index that later than a given person's time label
 */
int MessageContainer::GetIndexNewerThan(std::string name, time_t theTime)
{
	std::vector<int> v = GetIndexVector(name);
	if (v.empty())
		return -2; // no record for this person
	for (int i=0;i<v.size();i++) { // use another efficient way
		if (v[i] > theTime) //need use miliseconds
			return i;
	}
	return -1;
}
/*
 * convert the messages after index into a single XML
 * XML format:
 * 	<packet>
 * 		<message> ... </message>
 * 		<message> ... </message>
 * 		<message> ... </message>
 * 		<message> ... </message>
 * 	</packet>
 */
std::string MessageContainer::ToXMLAfterIndex(int index)
{
	pugi::xml_document doc;
	doc.load("<packet></packet>");
	for (;index<Size();index++) {
		pugi::xml_document single_doc;
		single_doc.load(m_messages[index].ToXML().c_str());
		doc.append_copy(single_doc);
	}
	xml_string_writer writer;
	doc.save(writer);
	return writer.result;
}

} //ns3
