#include "drs-record.h"
#include<ctime>
#include<iostream>
#include "pugixml.h"
#include "hebi.h"

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
	doc.load("<label></label><dataname>"+m_dataName+"</dataname>");
	pugi::xml_node label = doc.child("label");
	pugi::xml_node node;
	BOOST_FOREACH(m_map_type::value_type &mi, m_label) {
		node = label.append_child("li");
		node.append_child("name").text().set(mi.first);
		node.append_child("time").text().set(std::to_string(mi.second));
	}
	hebi::xml_string_writer writer;
	doc.save(writer);
	return writer.result;
}

DRSRecord DRSRecord::CreateDRSRecordFromXMLNode(pugi::xml_node root_node)
{
	pugi::xml_node node;
	DRSRecord _record = new DRSRecord;
	std::string name;
	time_t time;
	for (node=root_node.child("label").child("li");node;node=node.next_sibling()) {
		name = node.child("name").text().get();
		time = node.child("time").text().as_int();
		_record[name] = time;
	}
	_record.m_dataName = root_node.child("dataname").text().get();
	return _record;
}
DRSRecord DRSRecord::CreateDRSRecord(std::string name, time_t time, std::string dataname)
{
	DRSRecord _record = new DRSRecord;
	_record.m_dataName = dataname;
	_record[name] = time;
	return _record;
}


} //ns3
