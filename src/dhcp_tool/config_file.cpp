#include <stdafx.h>
#include <string>
#include <cassert>
#include "config_file.h"
#include "my_network_utils.h"
#include "my_string_utils.h"
#include "my_misc_utils.h"

#pragma warning( disable : 4510 4610 )

using namespace std;

/********************************** Internal Functions ****************************************/
namespace {
	const char CONFIG_FILE_HEAD[] = "//This is the config file for dhcp tool\n\n\n";

	typedef enum {
		DHCP_CONFIG_KEYWORD_LOCALIP_E,
		DHCP_CONFIG_KEYWORD_LOCALMAC_E,
		DHCP_CONFIG_KEYWORD_SERVERIP_E,
		DHCP_CONFIG_KEYWORD_INTERVAL_E,
		DHCP_CONFIG_KEYWORD_TOTALTIME_E,
		DHCP_CONFIG_KEYWORD_RELEASE_E,
		DHCP_CONFIG_KEYWORD_MACFLAG_E,
		DHCP_CONFIG_KEYWORD_NULL_E
	} DHCP_CONFIG_KEYWORD_E;
	const char* GetKeywordsByType(DHCP_CONFIG_KEYWORD_E eType);
	string GetKeywordString(const string& line);
	string GetValueString(const string& line) ;
	DHCP_CONFIG_KEYWORD_E GetKeywordType(const string& keyword);

}
/**********************************************************************************************/
ConfigFile::~ConfigFile()
{
	using std::endl;

	if (configFile.length()) {
		Init();
		//enter config file name
		fstream fs(configFile.c_str(), ios_base::out|ios_base::trunc);
		
		if (fs.is_open()) {
			//save current config
			fs<<CONFIG_FILE_HEAD<<endl;
			
			fs<< GetKeywordsByType(DHCP_CONFIG_KEYWORD_LOCALIP_E)<<inet_ntoa_e(GetLocalIp())<<endl;
			fs<< GetKeywordsByType(DHCP_CONFIG_KEYWORD_LOCALMAC_E);
			print_mac(fs, localMac)<<endl;
			fs<< GetKeywordsByType(DHCP_CONFIG_KEYWORD_SERVERIP_E)<<inet_ntoa_e(serverIp)<<endl;
			fs<< GetKeywordsByType(DHCP_CONFIG_KEYWORD_INTERVAL_E)<<interval<<endl;
			fs<< GetKeywordsByType(DHCP_CONFIG_KEYWORD_TOTALTIME_E)<<totalTime<<endl;
			fs<< GetKeywordsByType(DHCP_CONFIG_KEYWORD_RELEASE_E)<<bReleaseAfterUsed<<endl;
			fs<< GetKeywordsByType(DHCP_CONFIG_KEYWORD_MACFLAG_E)<<macSetting<<endl;

			fs.close();
		}

	}
}

B_U32 ConfigFile::GetLocalIp()
{
	Init();
	return localIp;
}

B_U32 ConfigFile::GetServerIp()
{
	Init();
	return serverIp;
}

B_U32 ConfigFile::GetInterval()
{
	Init();
	return interval;
}

B_U32 ConfigFile::GetTotaltime()
{
	Init();
	return totalTime;
}

MY_MAC_ADDR ConfigFile::GetLocalMac()
{
	Init();
	return localMac;
}

void ConfigFile::Init()
{
	if (!bInit) {
		localIp = 0;
		serverIp = 0;
		bzero(&localMac, sizeof(localMac));
		interval = 100;
		totalTime = 3;

		fstream fs(configFile.c_str(), ios_base::in);
		if (fs.is_open()) {
			//there is config file, we could get setting from config file
			string line;

			while (getline(fs, line)) {
				string keyword = TrimSpace(GetKeywordString(line));
				string value = TrimSpace(GetValueString(line));
				
				switch (GetKeywordType(keyword)) {
					case DHCP_CONFIG_KEYWORD_LOCALIP_E:
						localIp = inet_addr(value.c_str());
						break;
					case DHCP_CONFIG_KEYWORD_LOCALMAC_E:
						localMac = ConvertMacStr2Value(value);
						break;
					case DHCP_CONFIG_KEYWORD_SERVERIP_E:
						serverIp = inet_addr(value.c_str()); 
						break;
					case DHCP_CONFIG_KEYWORD_INTERVAL_E:
						interval = atoi(value.c_str());
						break;
					case DHCP_CONFIG_KEYWORD_TOTALTIME_E:
						totalTime = atoi(value.c_str());
						break;
					case DHCP_CONFIG_KEYWORD_RELEASE_E:
						bReleaseAfterUsed = atoi(value.c_str());
						break;
					case DHCP_CONFIG_KEYWORD_MACFLAG_E:
						macSetting = atoi(value.c_str());
						break;
					case DHCP_CONFIG_KEYWORD_NULL_E:
						break;
					default:
						assert(false);
				}
			}

			fs.close();

		} 
		else {
			//no config file, we could get the setting from system
			using MyNetworksUtil::NetworksUtil;

			localIp = networkUtil.GetRighSendIp(0);
			localMac = networkUtil.GetLocalMAC();
		}		
		bInit = true;
	}

}


namespace {		
	using namespace std;

	typedef struct {
		const char* const 	  keyWord;
		DHCP_CONFIG_KEYWORD_E eType;
	} DHCP_CONFIG_KEYWORD_PAIR_E;

	const DHCP_CONFIG_KEYWORD_PAIR_E DHCP_KEYWORDS_SET[] = {
		{"Local IP:", DHCP_CONFIG_KEYWORD_LOCALIP_E},
		{"Local MAC:", DHCP_CONFIG_KEYWORD_LOCALMAC_E},
		{"Server IP:", DHCP_CONFIG_KEYWORD_SERVERIP_E},
		{"Interval:", DHCP_CONFIG_KEYWORD_INTERVAL_E},
		{"Total time:", DHCP_CONFIG_KEYWORD_TOTALTIME_E},
		{"Release after used:", DHCP_CONFIG_KEYWORD_RELEASE_E},
		{"MAC Setting:", DHCP_CONFIG_KEYWORD_MACFLAG_E}
	};

	const char* GetKeywordsByType(DHCP_CONFIG_KEYWORD_E eType)
	{
		assert(DHCP_KEYWORDS_SET[eType].eType == eType);
		return DHCP_KEYWORDS_SET[eType].keyWord;
	}
 
	string GetKeywordString(const string& line)
	{
		size_t pos;
		pos = line.find_first_of(':');
		if (-1 == pos) {
			return "";
		}

		return line.substr(0, pos+1);
	}

	string GetValueString(const string& line) 
	{
		size_t pos;
		pos = line.find_first_of(':');
		if (-1 == pos) {
			return "";
		}

		return line.substr(pos+1);;
	}

	DHCP_CONFIG_KEYWORD_E GetKeywordType(const string& keyword)
	{
		int i;
		for (i = DHCP_CONFIG_KEYWORD_LOCALIP_E; i < DHCP_CONFIG_KEYWORD_NULL_E; ++i) {
			if (keyword == DHCP_KEYWORDS_SET[i].keyWord) {
				break;
			}
 		}

		return static_cast<DHCP_CONFIG_KEYWORD_E>(i);
	}
}
