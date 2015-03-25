#ifndef _MY_UTILS_H_
#define _MY_UTILS_H_

#include <string>
#include <vector>
#include <list>
#include <ostream>
#include <my_network_type.h>


const char* inet_ntoa_e(B_U32 ip);
std::ostream& print_mac(std::ostream& out, MY_MAC_ADDR& mac, char seperator = ':') ;
MY_MAC_ADDR ConvertMacStr2Value(const std::string& str);
MY_MAC_ADDR ConvertMacStr2Value(const char* str);

namespace MyNetworksUtil {		
	class NetworksUtil {
	public:
		NetworksUtil();
		~NetworksUtil();
		bool test();
		B_U32 GetRighSendIp(B_U32 dst);
		MY_MAC_ADDR GetLocalMAC();
		bool IsValidLocalIp(B_U32 ip);
	private:
		void Init();

		std::string hostname;
		std::vector<B_U32> ipSet; //network order
		std::list<MY_MAC_ADDR> macSet;

		bool init;
	};

	class NetworksErr{
	public:
		NetworksErr(const char* des):err(des) {};
		virtual const std::string& what() const {return err;}
	private:
		std::string err;
	};
}

#endif