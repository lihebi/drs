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
/*
 * not really insert.
 * just add a point to the latest index.
 */
void DRSRecordContainer::AddAsLatest(DRSRecord record)
{
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
	m_nameTimeIndex[newname][newtime] = index;
}
/*
 * return nameTimeIndex[name][time]
 */
int DRSRecordContainer::GetLatestIndexBySingleLabel(std::string name, double time)
{
	/* FUTURE: use compare, not equal */
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
std::vector<std::string> DRSRecordContainer::InsertMultiByXML(std::string xml, std::string newname, double newtime, std::string serverName)
{
	std::cout<<xml<<std::endl;
	pugi::xml_document doc;
	doc.load(xml.c_str());
	pugi::xml_node li;
	pugi::xml_node label;
	std::vector<std::string> v;
	for (li=doc.child("li");li;li=li.next_sibling()) {
		DRSRecord _record(li);

		/* 
		 * the xml is received from server. If I send somethingnew to server, I've already have the newest label, so if this is the label, just ignore it. Or there will be multi record for the same dataname, resulting in fetching 2 copy of a message. */
		/*if (this->m_nameTimeIndex[serverName][_record.m_label[serverName]] != 0) {
			std::cout<<"DUPLICATED"<<std::endl;
			std::cout<<_record.m_label[serverName]<<std::endl;
			continue;
		}*/
		typedef std::map<std::string, double> _map_type;
		/*
		 * if the new record has something same as my log, only add labels.
		 * NO! DO NOTHING AT ALL!
		 * if not, add every thing
		 */
		bool b = false;
		BOOST_FOREACH(const _map_type::value_type &m, _record.m_label) {
			if (this->m_nameTimeIndex[m.first][m.second]!=0) {
				int ii = this->m_nameTimeIndex[m.first][m.second];
				if (_record.m_dataName == this->m_records[ii].m_dataName) 
					b = true;
			}
			/*if (this->exclude[m.second]==m.first) {
				m_nameTimeIndex[m.first][m.second] = m_records.size()-1;
				b = true;
			}*/
		}
		if (b) continue;
		if (this->excludeDataName[_record.m_dataName]==1) {
			AddAsLatest(_record);
			continue;
		}

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
