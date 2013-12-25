#ifndef MESSAGE_H_
#define MESSAGE_H_
#include<iostream>
#include<map>
#include "../pugixml.h"

namespace ns3 {

class DRSRecord
{
public:
	DRSRecord(pugi::xml_node root_node);
	DRSRecord(std::string name, double time, std::string dataname);
	void AddLabel(std::string name, double time);
	std::string AsXML();
	DRSRecord CreateDRSRecordFromXMLNode(pugi::xml_node);
	DRSRecord CreateDRSRecord(std::string name, double time, std::string dataname);

	std::map<std::string, double> m_label;
	std::string m_dataName;

};

} //ns3
#endif
