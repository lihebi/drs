#ifndef MESSAGE_H_
#define MESSAGE_H_
#include<iostream>
#include<map>
#include "pugixml.h"

namespace ns3 {

class Message
{
public:
	void SetContent(const std::string&);
	void AddLabel(const std::string &name, time_t theTime);
	const std::string& GetContent();
	time_t GetTime(const std::string &name);
	const std::map<std::string, time_t> GetMap();
	std::string ToXML();

private:
	std::string m_content;
	std::map<std::string, time_t> m_label;

}; //Message

struct xml_string_writer: pugi::xml_writer
{
	std::string result;
	virtual void write(const void *data, size_t size)
	{
		result += std::string(static_cast<const char*>(data), size);
	}
};

} //ns3
#endif
