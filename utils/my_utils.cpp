#include <stdafx.h>
#include <ostream>
#include <cstdio>
#include <cstdlib>
#include <my_basetype.h>
#include "my_network_type.h"
#include "my_network_utils.h"

using std::ostream;
using namespace std;
#if 0
const char* inet_ntoa_e(B_U32 ip)
{
	in_addr in;
	in.s_addr = ip;

	return inet_ntoa(in);
}

ostream& print_mac(ostream& out, MY_MAC_ADDR& mac, char seperator) 
{
	char szBuf[20];

	for (int i = 0; i < mac.len; ++i) {
		bzero(szBuf, sizeof(szBuf));
		_snprintf_s(szBuf, sizeof(szBuf), 2, "%02x", B_U32(mac.addr[i]));
		out<<szBuf;
		if (i != mac.len-1) {
			out<<seperator;
		}
	}

	return out;
}
#endif