#include <stdafx.h>
#include <string>
#include <cstdio>

using std::string;

#pragma warning( disable : 4996 )

string TrimSpace(string str)
{
	size_t pos = str.find_first_not_of(' ');
	if (-1 != pos) {
		str.erase(0,pos);
	}
	pos = str.find_last_not_of(' ');
	if (-1 != pos) {
		str.erase(pos+1);
	}

	return str;
}

std::string ConvertNumToString(int num) 
{
	static char szBuf[256];

	_snprintf(szBuf, sizeof(szBuf), "%d", num);

	return string(szBuf);
}

