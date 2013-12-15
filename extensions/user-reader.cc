#include "user-reader.h"
#include<iostream>
#include<fstream>
#include<sstream>

void UserReader::SetFilename(std::string filename)
{
	m_filename = filename;
}
void UserReader::Read(void)
{
	std::ifstream is;
	is.open(m_filename.c_str());
	while(!is.eof()) {
		std::string line;
		getline(is, line);
		if (line[0] == '#')
			continue;
		if (line.empty())
			continue;
		std::istringstream lineBuffer(line);
		User user;
		lineBuffer >> user.name >> user.prefix >> user.node >> user.join_time;
		m_users.push_back(user);
	}
}
