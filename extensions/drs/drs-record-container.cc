#include "message-container.h"
#include "message.h"
#include "pugixml.h"
#include<algorithm>
#include<sstream>

#include "hebi.h"
namespace ns3 {


/*
 * 1. add to m_records
 * 2. add map to m_nameTimeIndex
 */
void DRSRecordContainer::Insert(DRSRecord record)
{
	m_records.push_back(record);
	typedef std::map<std::string, time_t> m_map_type;
	BOOST_FOREACH(const m_map_type::value_type &m, record.m_labels) {
		m_nameTimeIndex[m.first][m.second] = m_records.size()-1;
	}
}
int DRSRecordContainer::GetRecordSize()
{
	return m_records.size();
}
/*
 * 1. add label to m_record[i]
 * 2. add map to m_nameTimeIndex
 */
void DRSRecordContainer::AddLabelByLabel(std::string newname, time_t newtime, std::string oldname, time_t oldtime)
{
	int index = m_nameTimeIndex[oldname][oldtime];
	m_records[index].AddLabel(newname, newtime);
}
/*
 * return nameTimeIndex[name][time]
 */
int DRSRecordContainer::GetLatestIndexBySingleLabel(std::string name, time_t time)
{
	return m_nameTimeIndex[name][time];
}
/*
 * Format:
 * 	alice_13522342
 */
int DRSRecordContainer::GetLatestIndexBySingleLabel(std::string label)
{
	std::string name = label.substr(0, label.find("_"));
	time_t time = atoi(label.substr(label.find("_")+1));
	return GetLatestIndexBySingleLabel(name, time);
}
int DRSRecordContainer::GetLatestIndexByMultiLabels(std::vector<boost::tuple<std::string, time_t> > v)
{
	int result=-1;
	typedef boost::tuple<std::string, time_t> m_vector_type;
	BOOST_FOREACH(const m_vector_type &vi, v) {
		int tmp = GetLatestIndexBySingleLabel(vi.get<0>(), vi.get<1>());
		if (result<tmp) result = tmp;
	}
	return result;
}
int DRSRecordContainer::GetLatestIndexByMultiLabels(std::string labels)
{
	int result = -1;
	hebi::SplitStringByIndent(labels, ':');
	std::stringstream ss(labels);
	std::string stmp;
	while(ss>>stmp) {
		int tmp = GetLatestIndexBySingleLabel(stmp);
		if (result<tmp) result = tmp;
	}
	return result;
}
/*
 * Format:
 * 	<li>
 * 		<label>
 * 			<li> <name> ... </name> <time> ... </time> </li>
 * 			<li> <name> ... </name> <time> ... </time> </li>
 * 		</label>
 * 		<dataname> ... </dataname>
 * 	</li>
 * 	<li>
 * 		...
 * 	</li>
 */
std::string DRSRecordContainer::GetAfterIndexAsXML(int index)
{
	pugi::xml_document doc;
	doc.load("");
	pugi::xml_document doctmp;
	/* from index +1 */
	for (int i=index+1;i<m_records.size();i++) {
		doctmp.load(m_records[i].AsXML());
		doc.append_child("li").append_copy(doctmp);
	}
	hebi::xml_string_writer writer;
	doc.save(writer);
	return writer.result;
}
/*
 * Return:
 * 	vector<dataname>
 */
std::vector<std::string> DRSRecordContainer::InsertMultiByXML(std::string xml)
{
	pugi::xml_document doc;
	doc.load(xml);
	pugi::xml_node li;
	pugi::xml_node label;
	std::vector<std::string> v;
	for (li=doc.child("li");li;li=li.next_sibling) {
		DRSRecord _record = DRSRecord::CreateDRSRecordFromXMLNode(li);
		Insert(_record);
		v.push_back(_record.m_dataName);
	}
	return v;
}
/*
 * Return:
 * 	dataname
 */
std::string DRSRecordContainer::InsertSingleByXML(std::string xml)
{
	pugi::xml_document doc;
	doc.load(xml);
	DRSRecord _record = DRSRecord::CreateDRSRecordFromXMLNode(doc);
	Insert(_record);
	return _record.m_dataName;
}


} //ns3
