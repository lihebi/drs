#include "drs-record.h"
#include<ctime>
#include<iostream>
#include "../pugixml.h"
#include "../hebi.h"
#include<boost/foreach.hpp>

namespace ns3 {

void DRSRecord::AddLabel(std::string name, time_t time)
{
	m_label[name] = time;
}
/*
 * Format:
 * 	<label>
 * 		<li> <name> ... </name> <time> ... </time> </li>
 * 		<li> <name> ... </name> <time> ... </time> </li>
 * 	</label>
 * 	<dataname> ... </dataname>
 */
std::string DRSRecord::AsXML()
{
	pugi::xml_document doc;
	std::string stmp = "<label></label><dataname>"+m_dataName+"</dataname>";
	doc.load(stmp.c_str());
	pugi::xml_node label = doc.child("label");
	pugi::xml_node node;
	typedef std::map<std::string, time_t> m_map_type;
	BOOST_FOREACH(m_map_type::value_type &mi, m_label) {
		node = label.append_child("li");
		node.append_child("name").text().set(mi.first.c_str());
		node.append_child("time").text().set(std::to_string(mi.second).c_str());
	}
	hebi::xml_string_writer writer;
	doc.save(writer);
	return writer.result;
}

DRSRecord::DRSRecord(pugi::xml_node root_node)
{
	pugi::xml_node node;
	std::string name;
	time_t time;
	for (node=root_node.child("label").child("li");node;node=node.next_sibling()) {
		name = node.child("name").text().get();
		time = node.child("time").text().as_int();
		m_label[name] = time;
	}
	m_dataName = root_node.child("dataname").text().get();
}
DRSRecord::DRSRecord(std::string name, time_t time, std::string dataname)
{
	m_dataName = dataname;
	m_label[name] = time;
}


} //ns3
