#include <stdafx.h>
#include <winsock.h>
#include <assert.h>
#include "my_network_utils.h"
#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <Winsock2.h>
#include <Iphlpapi.h>

#include "my_misc_utils.h"


using namespace std;
using namespace MyNetworksUtil;

static const B_U32 IP_MASK = 0xFFFFFF00;
/***************************************************************************************************************/
namespace {
bool GetLocalHostname(string& hostname);
bool GetLocalIPs(const string& hostname, vector<B_U32>& ipSet);
bool GetLocalMacs(list<MY_MAC_ADDR>& macSet);
}
/***************************************************************************************************************/
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

MY_MAC_ADDR ConvertMacStr2Value(const string& str)
{
	MY_MAC_ADDR macAddr;
	bzero(&macAddr, sizeof(macAddr));

	if (str.empty()) {
		return macAddr;
	}

	int cnt = 0;
	size_t pos = static_cast<size_t>(-1);
	while ((pos = str.find(':', pos+1)) != -1) {
		++cnt;
	}
	assert(cnt == 5);
	if (cnt != 5) {
		return macAddr;
	}

	sscanf_s(str.c_str(), "%X:%X:%X:%X:%X:%X",
		  &macAddr.addr[0], &macAddr.addr[1], &macAddr.addr[2],
		  &macAddr.addr[3], &macAddr.addr[4], &macAddr.addr[5]);
	macAddr.len = 6;

	return macAddr;
}

MY_MAC_ADDR ConvertMacStr2Value(const char* str)
{	
	MY_MAC_ADDR macAddr;
	bzero(&macAddr, sizeof(macAddr));

	if (NULL == str || '\0' == str[0]) {
		return macAddr;
	}

	int cnt = 0;
	const char* p = str;
	while (*p) {
		if (':' == *p) {
			++cnt;
		}
		++p;
	}	
	assert(cnt == 5);
	if (cnt != 5) {
		return macAddr;
	}

	sscanf_s(str, "%X:%X:%X:%X:%X:%X",
		  &macAddr.addr[0], &macAddr.addr[1], &macAddr.addr[2],
		  &macAddr.addr[3], &macAddr.addr[4], &macAddr.addr[5]);
	macAddr.len = 6;

	return macAddr;
}


namespace MyNetworksUtil{
	using std::cout;
	using std::endl;
	using std::vector;

	NetworksUtil::NetworksUtil():init(false)
	{
		//Init Windows Sock API 
		WORD	wVersion = MAKEWORD(2,2);
		WSADATA wsaData;
		
		if (WSAStartup(wVersion, &wsaData)) {
			throw NetworksErr("WSAStartup Failed");
		}
	}

	NetworksUtil::~NetworksUtil()
	{
		WSACleanup();
	}

	void NetworksUtil::Init()
	{
		if (!init) {
			if (!GetLocalHostname(hostname)) {
				throw NetworksErr("GetLocalHostname failed");
			}

			if (!GetLocalIPs(hostname, ipSet)) {
				throw NetworksErr("GetLocalIPs failed");
			}

			if (!GetLocalMacs(macSet)) {
				throw NetworksErr("GetLocalMacs failed");
			}

		}
	}

	B_U32 NetworksUtil::GetRighSendIp(B_U32 dst)
	{
		vector<B_U32>::const_iterator pos;
		B_U32 ulLocalIp = 0;

		Init();
		
		if (0 == dst) {
			return ipSet.empty() ? 0 : ipSet[0];
		}
		
		for (pos = ipSet.begin(); pos != ipSet.end(); ++pos) {
			if ((*pos&IP_MASK) == (dst&IP_MASK)) {
				ulLocalIp = *pos;
				break;
			}
		}

		return ulLocalIp;
		
	}

	MY_MAC_ADDR NetworksUtil::GetLocalMAC()
	{
		MY_MAC_ADDR mac;

		bzero(&mac, sizeof(mac));

		return macSet.empty()?mac:macSet.front();
	}

	bool NetworksUtil::IsValidLocalIp(B_U32 ip) 
	{
		Init();
		return (find(ipSet.begin(), ipSet.end(), ip) != ipSet.end());
	}

	bool NetworksUtil::test()
	{
		cout<<"Hostname: "<<hostname<<endl;

		vector<B_U32>::const_iterator pos;
		for (pos = ipSet.begin(); pos != ipSet.end(); ++pos) {			
			cout<<"ip: "<<inet_ntoa_e(*pos)<<endl;
		}

		return true;
	}
}

namespace {
	bool GetLocalHostname(string& hostname)
	{
		//get host name
		char hostnamebuf[256];
		if (gethostname(hostnamebuf, sizeof(hostnamebuf))) {
			return false;
		}
		hostname = hostnamebuf;

		return true;
	}

	bool GetLocalIPs(const string& hostname, vector<B_U32>& ipSet)
	{
		//get host info
		hostent *pHostent = gethostbyname(hostname.c_str());
		if (NULL == pHostent) {
			return false;
		}

		//parse hostent
		for (int i = 0; pHostent->h_addr_list[i] != NULL; ++i) {
			B_U32 ip;
			assert(sizeof(ip) == pHostent->h_length);
			memcpy(&ip, pHostent->h_addr_list[i], pHostent->h_length);
			ipSet.push_back(ip);
		} 
		return true;
	}

	bool GetLocalMacs(list<MY_MAC_ADDR>& macSet)
	{
		ULONG32 cnt = 16;
		DWORD	status;
		ULONG buflen = cnt*sizeof(IP_ADAPTER_ADDRESSES);   
		PIP_ADAPTER_ADDRESSES padapterAddr = new IP_ADAPTER_ADDRESSES [cnt];
		
		while ((status = GetAdaptersAddresses(AF_INET,
											 GAA_FLAG_SKIP_DNS_SERVER|GAA_FLAG_SKIP_MULTICAST|GAA_FLAG_SKIP_UNICAST,
											 NULL, padapterAddr, &buflen)) == ERROR_BUFFER_OVERFLOW) {
			delete [] padapterAddr;
			cnt *= 2;
			padapterAddr = new IP_ADAPTER_ADDRESSES [cnt];
		}
		
		if (ERROR_SUCCESS != status) {
			delete [] padapterAddr;
			return false;
		}
		
		for (PIP_ADAPTER_ADDRESSES pOne = padapterAddr; pOne; pOne = pOne->Next) {
			if (pOne->Flags&IP_ADAPTER_RECEIVE_ONLY) {
				continue;
			}

			if (pOne->IfType&IF_TYPE_ETHERNET_CSMACD) {
				MY_MAC_ADDR macAddr;

				//only get ethernet adapter
				assert(pOne->PhysicalAddressLength < sizeof(macAddr.addr)-1);

				macAddr.len = static_cast<char>(pOne->PhysicalAddressLength);
				memcpy(macAddr.addr, pOne->PhysicalAddress, macAddr.len);
				macAddr.addr[macAddr.len] = 0;

				if (NULL != wcsstr(pOne->FriendlyName, L"Local Area Connection")) {
					macSet.push_front(macAddr);
				} else {
					macSet.push_back(macAddr);
				}
			}
		}
		delete [] padapterAddr;

		return true;
	}

}
