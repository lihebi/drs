#ifndef MESSAGE_CONTAINER_H_
#define MESSAGE_CONTAINER_H_
#include "drs-record.h"
#include<vector>
#include<boost/tuple/tuple.hpp>
#include<map>

namespace ns3 {

class DRSRecordContainer
{
public:
	void Insert(DRSRecord record);
	int GetRecordSize();
	void AddLabelByLabel(std::string newname, time_t newtime, std::string oldname, time_t oldtime);
	int GetLatestIndexBySingleLabel(std::string name, time_t time);
	int GetLatestIndexBySingleLabel(std::string label);
	int GetLatestIndexByMultiLabels(std::vector<boost::tuple<std::string, time_t> > v);
	int GetLatestIndexByMultiLabels(std::string labels);
	std::string GetAfterIndexAsXML(int index);
	std::vector<std::string> InsertMultiByXML(std::string xml, std::string newname, long newtime);
	std::string InsertSingleByXML(std::string xml);
	std::string GetAllNewestLabels();
	std::string GetNewestLabelByName(std::string name);
	bool HasName(std::string name);

private:
	typedef std::map<std::string, std::map<time_t, int> > m_map_type;
	/*
	 * name ---> (time ---> index)
	 */
	std::map<std::string, std::map<time_t, int> > m_nameTimeIndex;
	std::vector<DRSRecord> m_records;

}; //MessageContainer



} //ns3
#endif
