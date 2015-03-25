#ifndef _DHCP_CONFIG_H_
#define _DHCP_CONFIG_H_

class Config {
public:
	B_U32 GetLocalIp();
	bool test();

	Config():bInit(false) {};
private:
	void GetConfigSetting();

	B_U32 localIp;

	bool bInit;
};


#endif