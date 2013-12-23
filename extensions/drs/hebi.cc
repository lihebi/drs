#include "hebi.h"

namespace hebi {

/* 
 * will modify the string 
 */
void StringSplit(std::string &s, char indent)
{
	int index;
	for (;;) {
		index = name.find(indent);
		if (index == -1) break;
		else name[index] = ' ';
	}
}
std::string StringGet(const std::string &s, char indent, int seq)
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




}
