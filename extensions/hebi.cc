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
		return s.substr(begin+1);
	else
		return s.substr(begin+1, end-begin-1); //DEBUG
}
int MyStringFinder(const std::string &s, char indent, int seq)
{
	int result=-1;
	for (int i=0;i<seq;i++)
		result = s.find(indent, result+1);
	return result;
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
long GetPosixTime_TotalMilli()
{
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration td = now.time_of_day();
	return td.total_milliseconds();
}

bool CheckDouble(double d)
{
	double tmp = atof(std::to_string(d).c_str());
	return tmp==d;
}
double ConvertDouble(double d)
{
	return atof(std::to_string(d).c_str());
}
std::string GetEnv(const std::string &var)
{
	const char *val = ::getenv(var.c_str());
	if (val == 0) return "";
	else return val;
}
int GetEnvAsInt(const std::string &var)
{
	return atoi(GetEnv(var).c_str());
}


} //hebi

