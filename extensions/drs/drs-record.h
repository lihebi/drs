#ifndef MESSAGE_H_
#define MESSAGE_H_
#include<iostream>
#include<map>
#include "pugixml.h"

namespace ns3 {

class DRSRecord
{
friend class DRSRecordContainer;
public:
	void AddLabel(std::string name, time_t time);
	std::string AsXML();
	DRSRecord CreateDRSRecordFromXMLNode(pugi::xml_node);
	DRSRecord CreateDRSRecord(std::string name, time_t time, std::string dataname);

	std::map<std::string, time_t> m_label;
	std::string m_dataName;

};

} //ns3
#endif
