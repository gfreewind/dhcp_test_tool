#pragma once
#include <windows.h>
#include <stdexcept>

#include "my_basetype.h"
#include "my_network_type.h"
#include "my_misc_utils.h"

typedef enum {
    DHCP_OPTION_REQUEST_IP_E = 0x32,
    DHCP_OPTION_MSG_TYPE_E = 0x35,
    DHCP_OPTION_CLIENT_ID_E = 0x3D,
    DHCP_OPTION_SUBNET_SELECTION_E = 0x76,
    DHCP_OPTION_END_E = 0xff,
} DHCP_OPTION_E;


typedef enum {
    DHCP_PACKET_TYPE_NULL_E,
    DHCP_PACKET_TYPE_DISCOVER_E,
    DHCP_PACKET_TYPE_OFFER_E,
    DHCP_PACKET_TYPE_REQUEST_E,
    DHCP_PACKET_TYPE_DECLINE_E,
    DHCP_PACKET_TYPE_ACK_E,
    DHCP_PACKET_TYPE_NAK_E,
    DHCP_PACKET_TYPE_RELEASE_E,
    DHCP_PACKET_TYPE_INFORM_E
} DHCP_PACKET_TYPE_E;


typedef enum {
    DHCP_OPCODE_NULL_E = 0,
    DHCP_OPCODE_REQUEST_E = 1, 
    DHCP_OPCODE_REPLY_E = 2
} DHCP_OPCODE_E;

typedef struct {
    DHCP_OPTION_E eType;
    B_U8          len;
    B_U8          data[256];
} DHCP_OPTION_SET_S;

typedef enum {
    DHCP_HARDWARE_TYPE_NULL_E = 0,
    DHCP_HARDWARE_TYPE_ETHER_E = 1,
} DHCP_HARDWARE_TYPE_E;
#define DHCP_ETH_HARDWARE_LEN          6


typedef enum {
    DHCP_MSG_TYPE_NULL_E     = 0,   
    DHCP_MSG_TYPE_DISCOVER_E = 1,   /* Client broadcast to locate available servers. */
    DHCP_MSG_TYPE_OFFER_E    = 2,   /* Server to client in response to DHCPDISCOVER with offer of configuration parameters.
                                                              My Note: Server check the network ip before offer, not ack
                                                         */
    
    DHCP_MSG_TYPE_REQUEST_E  = 3,   /* Client message to servers either 
                                                              (a) requesting offered parameters from one server 
                                                                   and implicitly declining offers from all others, 
                                                              (b) confirming correctness of previously allocated address after, e.g., system reboot, 
                                                              (c) extending the lease on a particular network address.
                                                          */
    DHCP_MSG_TYPE_DECLINE_E  = 4,   /* Client to server indicating network address is already in use. */
    DHCP_MSG_TYPE_ACK_E      = 5,   /* Server to client with configuration parameters, including committed network address. 
                                                              My Note: Client should check the ip and parameter when receive the ack.
                                                         */
    DHCP_MSG_TYPE_NAK_E      = 6,   /* Server to client indicating client's notion of network address is incorrect 
                                                              (e.g., client has moved to new subnet) or client's lease as expired 
                                                              My Note: if client receive the nak, it should restart configuration again.
                                                          */
    DHCP_MSG_TYPE_RELEASE_E  = 7,   /* Client to server relinquishing network address and cancelling remaining lease. */
    DHCP_MSG_TYPE_INFORM_E   = 8,   /* Client to server, asking only for local configuration parameters; client already has externally configured network address. */
} DHCP_MSG_TYPE_E;

static const B_U32 DHCP_MAGIC_COOKIE = 0x63825363;

class DhcpPacket {
public:    
    enum {
        DHCP_FIXED_HEAD_SIZE = 236
    };

    DhcpPacket(size_t size=576):packet(size), optionSize(size), posEndOpt(7)
    {
        if (optionSize < sizeof(DHCP_MAGIC_COOKIE)) {
            throw std::invalid_argument("Option size couldn't less than 4 bytes");
        }      

        pMsgType = NULL;
    }

    bool IsOfferMsg() {
        return ((NULL != pMsgType) && (DHCP_MSG_TYPE_OFFER_E ==*pMsgType));
    }

    bool IsAckMsg() {
        return ((NULL != pMsgType) && (DHCP_MSG_TYPE_ACK_E ==*pMsgType));
    }

    void SetMsgType(DHCP_MSG_TYPE_E eType);
    void SetPacketType(DHCP_PACKET_TYPE_E packetType); 

    /* Set dhcp packet content */
    void SetTransacId(B_U32 id) {packet.transacId = id;};
    void SetRouteAddr(B_U32 ip) {packet.giaddr = ip;};
    void Sethardware(const MY_MAC_ADDR& mac) {memcpy(packet.chaddr, mac.addr, mac.len);}
    void SetXid(B_U64 time);
    void SetClientMac(const MY_MAC_ADDR& mac);
    void SetYourIp(B_U32 ip) {packet.yiaddr = ip;}
    void SetClientIp(B_U32 ip) {packet.ciaddr = ip;}
    void AddOption(DHCP_OPTION_SET_S& opt);
    void SetEndOption();

    /* Get dhcp packet content */
    bool CopyDhcpPacket(const char* buf, B_U32 bufLen);
    size_t GetPacketSize() {return DHCP_FIXED_HEAD_SIZE+realSize;}
    B_U32 GetYourIp() {return packet.yiaddr;};

    /* Send packet */
    void DhcpPacket::SendPacket(SOCKET sock, struct sockaddr_in& sockaddrServer);

private:
    bool IsValidOptions();
    B_U8* pMsgType;
    

    class Packet{
    public:
		Packet(size_t size=576)
        {
            init();
            memset(options, 0, min(size, 1024));
			options[0] = (B_U8)((DHCP_MAGIC_COOKIE>>24)&0xff);
			options[1] = (B_U8)((DHCP_MAGIC_COOKIE>>16)&0xff);
			options[2] = (B_U8)((DHCP_MAGIC_COOKIE>>8)&0xff);
			options[3] = static_cast<B_U8>(DHCP_MAGIC_COOKIE&0xff);
            options[4] = DHCP_OPTION_MSG_TYPE_E;
            options[5] = 1;
            FillFixedType();
        }
        void    FillFixedType()
        {
            hardwareType = DHCP_HARDWARE_TYPE_ETHER_E;
            hardwareLen = DHCP_ETH_HARDWARE_LEN;
            hops = 1;

            //fill magic 
        }
        
        /* Message op code/message type. 1 = BOOTREQUEST, 2 = BOOTREPLY */
        B_U8    opcode;
        
        /* Hardware address type.see ARP section in "Assigned Numbers" RFC; e.g., '1' = 10mb ethernet */
        B_U8    hardwareType;
        
        /* Hardware address length (e.g.  '6' for 10mb ethernet) */
        B_U8    hardwareLen;
    
        /* Client sets to zero, optionally used by relay agents when booting via a relay agent. */
        B_U8    hops;          
    
        /* Transaction ID, a random number chosen by the client, 
               used by the client and server to associate messages and responses between a client and a server. */
        B_U32   transacId;      
    
        /* Filled in by client, seconds elapsed since client began address acquisition or renewal process. */
        B_U16   secs;       
    
        /* Flags (see figure 2). */
        B_U16   flags;
    
        /* Client IP address; only filled in if client is in BOUND, RENEW or REBINDING state and can respond to ARP requests */
        B_U32   ciaddr;
    
        /* 'your' (client) IP address; 
               My Note: server filled, it should be the ip allocated by server
          */
        B_U32   yiaddr;
    
        /* IP address of next server to use in bootstrap; returned in DHCPOFFER, DHCPACK by server */
        B_U32   siaddr;
    
        /* Relay agent IP address, used in booting via a relay agent */
        B_U32   giaddr;
    
        /* Client hardware address */
        B_U8    chaddr[16];
    
        /* Optional server host name, null terminated string */
        B_U8    sname[64];
    
        /* Boot file name, null terminated string; 
              "generic" name or null in DHCPDISCOVER, fully qualified directory-path name in DHCPOFFER */
        B_U8    file[128];
    
        /* Optional parameters field.  See the options documents for a list of defined options 
               The 'options' field is now variable length. 
               A DHCP client must be prepared to receive DHCP messages with an 'options' field of at least length 312 octets.
               This requirement implies that a DHCP client must be prepared to receive a message of up to 576 octets,
               the minimum IP datagram size an IP host must be prepared to accept [3].  
               DHCP clients may negotiate the use of larger DHCP messages through the 'maximum DHCP message size' option.
               The options field may be further extended into the 'file' and 'sname' fields.
          */
        B_U8    options[1024];
    private:
        void init();
    };
    Packet packet;

    size_t optionSize;
    size_t realSize;
    size_t posEndOpt;
};


