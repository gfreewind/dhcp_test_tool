#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

#include <fstream>
#include <string>

#include "my_network_utils.h"

class ConfigFile {
public:
	ConfigFile(const char* fn):configFile(fn),
	                           bInit(false),
	                           localIp(0),
	                           serverIp(0),
	                           interval(0),
	                           totalTime(0),
	                           macSetting(0),
	                           bReleaseAfterUsed(0){}
	~ConfigFile();
	
	B_U32 GetLocalIp();
	void SetLocalIp(B_U32 ip) {localIp = ip;}
	
	B_U32 GetServerIp();
    void SetServerIP(B_U32 ip) {serverIp = ip;}
	
	B_U32 GetInterval();
    void SetInterval(B_U32 time) {interval = time;}
	
	B_U32 GetTotaltime();
    void SetTotaltime(B_U32 time) {totalTime = time;}

    B_BOOL GetReleaseFlag() {return bReleaseAfterUsed;};
    void   SetReleaseFlag(B_BOOL bFlag) {bReleaseAfterUsed = bFlag;};

    B_U32 GetMacSettingFlag() const {return macSetting;};
    void  SetMacSettingFlag(B_U32 flag) { macSetting = flag;};

    bool IsValidLocalIp(B_U32 ip) {return networkUtil.IsValidLocalIp(ip);}
        
	
	MY_MAC_ADDR GetLocalMac();
	bool test();
private:
	void Init();
	MyNetworksUtil::NetworksUtil networkUtil;

	B_U32 localIp;
	B_U32 serverIp;
	MY_MAC_ADDR localMac;
	B_U32 interval;
	B_U32 totalTime;
	B_U32 macSetting;
	B_BOOL bReleaseAfterUsed;

	std::string  configFile;
	std::fstream ifs;
	mutable bool bInit;
};


#endif
