#include "drs-record-container.h"
#include "drs-record.h"
#include "../pugixml.h"
#include<algorithm>
#include<boost/foreach.hpp>
#include<sstream>

#include "../hebi.h"
namespace ns3 {


/*
 * 1. add to m_records
 * 2. add map to m_nameTimeIndex
 */
void DRSRecordContainer::Insert(DRSRecord record)
{
	m_records.push_back(record);
	typedef std::map<std::string, double> m_map_type;
	BOOST_FOREACH(const m_map_type::value_type &m, record.m_label) {
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
void DRSRecordContainer::AddLabelByLabel(std::string newname, double newtime, std::string oldname, double oldtime)
{
	int index = m_nameTimeIndex[oldname][oldtime];
	m_records[index].AddLabel(newname, newtime);
}
/*
 * return nameTimeIndex[name][time]
 */
int DRSRecordContainer::GetLatestIndexBySingleLabel(std::string name, double time)
{
	std::cout<<"\t"<<name<<" "<<time<<std::endl;
	printf("%lf\n", time);
	std::map<double, int>::iterator iter;
	for (iter=m_nameTimeIndex[name].begin();iter!=m_nameTimeIndex[name].end();iter++) {
		printf("%lf\t%lf\n", time, iter->first);
		if (time == iter->first)
			std::cout<<iter->first<<", "<<iter->second<<std::endl;
		if (atof(std::to_string(time).c_str()) == (iter->first))
			std::cout<<"yes"<<std::endl;
	}
	std::cout<<m_nameTimeIndex[name][time]<<std::endl;
	return m_nameTimeIndex[name][time];
}
/*
 * Format:
 * 	alice_13522342
 */
int DRSRecordContainer::GetLatestIndexBySingleLabel(std::string label)
{
	std::string name = label.substr(0, label.find("_"));
	double time = atof((label.substr(label.find("_")+1)).c_str());
	return GetLatestIndexBySingleLabel(name, time);
}
int DRSRecordContainer::GetLatestIndexByMultiLabels(std::vector<boost::tuple<std::string, double> > v)
{
	int result=-1;
	typedef boost::tuple<std::string, double> m_vector_type;
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
		doctmp.load(m_records[i].AsXML().c_str());
		pugi::xml_node li = doc.append_child("li");
		/* the following 2 lines are very KengDie */
		li.append_copy(doctmp.document_element());
		li.append_copy(doctmp.document_element().next_sibling());
	}
	hebi::xml_string_writer writer;
	doc.save(writer);
	return writer.result;
}
/*
 * Return:
 * 	vector<dataname>
 */
std::vector<std::string> DRSRecordContainer::InsertMultiByXML(std::string xml, std::string newname, double newtime)
{
	pugi::xml_document doc;
	doc.load(xml.c_str());
	pugi::xml_node li;
	pugi::xml_node label;
	std::vector<std::string> v;
	for (li=doc.child("li");li;li=li.next_sibling()) {
		DRSRecord _record(li);
		_record.m_label[newname] = newtime; // add my own timelabel
		Insert(_record);
		v.push_back(_record.m_dataName);
	}
	return v;
}
/*
 * no <li>
 * no use
 * Return:
 * 	dataname
 */
std::string DRSRecordContainer::InsertSingleByXML(std::string xml)
{
	pugi::xml_document doc;
	doc.load(xml.c_str());
	//DRSRecord _record = DRSRecord::CreateDRSRecordFromXMLNode(doc);
	DRSRecord _record(doc);
	Insert(_record);
	return _record.m_dataName;
}

bool DRSRecordContainer::HasName(std::string name)
{
	return (m_nameTimeIndex.find(name) != m_nameTimeIndex.end());
}
std::string DRSRecordContainer::GetNewestLabelByName(std::string name)
{
	double time = (--m_nameTimeIndex[name].end())->first;
	return name+"_"+std::to_string(time);
}
std::string DRSRecordContainer::GetAllNewestLabels()
{
	std::string result;
	BOOST_FOREACH(m_map_type::value_type &mi, m_nameTimeIndex) {
		result += GetNewestLabelByName(mi.first);
		result += ":";
	}
	return result;
}


} //ns3
