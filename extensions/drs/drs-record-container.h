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
	void AddLabelByLabel(std::string newname, double newtime, std::string oldname, double oldtime);
	int GetLatestIndexBySingleLabel(std::string name, double time);
	int GetLatestIndexBySingleLabel(std::string label);
	int GetLatestIndexByMultiLabels(std::vector<boost::tuple<std::string, double> > v);
	int GetLatestIndexByMultiLabels(std::string labels);
	std::string GetAfterIndexAsXML(int index);
	std::vector<std::string> InsertMultiByXML(std::string xml, std::string newname, double newtime, std::string serverName);
	std::string InsertSingleByXML(std::string xml);
	std::string GetAllNewestLabels();
	std::string GetNewestLabelByName(std::string name);
	bool HasName(std::string name);

private:
	typedef std::map<std::string, std::map<double, int> > m_map_type;
	/*
	 * name ---> (time ---> index)
	 * a (name, time) pair may refer to multi indexes, but we will only need to record the newest one.
	 * The only funcion that matters is AddLabelByLabel, however it is only used in ProcessSomethingNewData, where the reference is single.
	 */
	std::map<std::string, std::map<double, int> > m_nameTimeIndex;
	std::vector<DRSRecord> m_records;

}; //MessageContainer



} //ns3
#endif
