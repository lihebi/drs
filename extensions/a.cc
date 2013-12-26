#include "hebi.h"
#include<iostream>

int main()
{
	std::string s = hebi::GetSubStringByIndent("/aaa/bbb/ccc", '/', 2);
	std::cout<<s<<std::endl;
	return 0;
}
