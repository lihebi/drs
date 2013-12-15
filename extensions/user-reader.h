#ifndef USER_READER_H_
#define USER_READER_H_
#include<iostream>
#include<vector>

class User
{
public:
	std::string name;
	std::string prefix;
	std::string node;
	std::string join_time;
};

class UserReader
{
public:
	void SetFilename(std::string filename);
	void Read(void);
	std::vector<User> m_users;
private:
	int m_number;
	std::string m_filename;
};

#endif
