//#include "pugixml.h"

#include<boost/date_time/posix_time/posix_time.hpp>
#include<boost/tuple/tuple.hpp>


namespace hebi {

/*
struct xml_string_writer : pugi::xml_writer
{
	std::string result;
	virtual void write(const void *data, size_t size) {
		result += std::string(static_cast<const char*>(data), size);
	}
};
*/
void SplitStringByIndent(std::string &s, char indent);
std::string GetSubStringByIndent(const std::string &s, char indent, int seq);
boost::tuple<long, long, long, long> GetPosixTime();
long GetPosixTime_Hours();
long GetPosixTime_Minutes();
long GetPosixTime_Seconds();
long GetPosixTime_Milli();




} //hebi
