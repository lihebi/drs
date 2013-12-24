#include "hebi.h"

namespace hebi {

/* 
 * will modify the string 
 */
void SplitStringByIndent(std::string &s, char indent)
{
	int index;
	for (;;) {
		index = s.find(indent);
		if (index == -1) break;
		else s[index] = ' ';
	}
}
std::string GetSubStringByIndent(const std::string &s, char indent, int seq)
{
	int begin=-1;
	int end;
	for (int i=0;i<seq;i++)
		begin = s.find(indent, begin+1);
	end = s.find(indent, begin+1);
	if (end==-1)
		return s.substr(begin);
	else
		return s.substr(begin, end-begin);
}
boost::tuple<long, long, long, long> GetPosixTime()
{
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration td = now.time_of_day();
	const long hours = td.hours();
	const long minutes = td.minutes();
	const long seconds = td.seconds();
	const long milliseconds = td.total_milliseconds() - ((hours*3600+minutes*60+seconds)*1000);
	return boost::make_tuple(hours, minutes, seconds, milliseconds);
}
long GetPosixTime_Hours()
{
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration td = now.time_of_day();
	return td.hours();
}
long GetPosixTime_Minutes()
{
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration td = now.time_of_day();
	return td.minutes();
}
long GetPosixTime_Seconds()
{
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration td = now.time_of_day();
	return td.seconds();
}
long GetPosixTime_Milli()
{
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration td = now.time_of_day();
	const long hours = td.hours();
	const long minutes = td.minutes();
	const long seconds = td.seconds();
	const long milliseconds = td.total_milliseconds() - ((hours*3600+minutes*60+seconds)*1000);
	return milliseconds;
}


} //hebi

