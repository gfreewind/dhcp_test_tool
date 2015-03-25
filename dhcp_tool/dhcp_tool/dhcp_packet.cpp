#include <stdafx.h>
#include <cassert>
#include <Winsock.h>
#include "dhcp_packet.h"

void DhcpPacket::SetPacketType(DHCP_PACKET_TYPE_E packetType)
{
	switch (packetType) {
		case DHCP_PACKET_TYPE_NULL_E:
			break;
		case DHCP_PACKET_TYPE_DISCOVER_E:
		case DHCP_PACKET_TYPE_REQUEST_E:
		case DHCP_PACKET_TYPE_INFORM_E:
			packet.opcode = DHCP_OPCODE_REQUEST_E;
			break;
		case DHCP_PACKET_TYPE_OFFER_E:
		case DHCP_PACKET_TYPE_ACK_E:
			packet.opcode = DHCP_OPCODE_REPLY_E;
			break;
		case DHCP_PACKET_TYPE_DECLINE_E:
		case DHCP_PACKET_TYPE_NAK_E:
		case DHCP_PACKET_TYPE_RELEASE_E:
			assert(false);
			break;
	}
	packet.options[6] = static_cast<B_U8>(packetType);
}

void DhcpPacket::SetXid(B_U64 time)
{
	packet.transacId = static_cast<B_U32>(time);
	
}

void DhcpPacket::SetClientMac(const MY_MAC_ADDR& mac)
{
	assert(mac.len <= sizeof(packet.chaddr));
	memcpy(packet.chaddr, mac.addr, mac.len);
}

void DhcpPacket::AddOption(DHCP_OPTION_SET_S& opt)
{
	packet.options[posEndOpt] = static_cast<B_U8>(opt.eType);
	++posEndOpt;
	packet.options[posEndOpt] = opt.len;
	++posEndOpt;
	memcpy(packet.options+posEndOpt, opt.data, opt.len);
	posEndOpt += opt.len;
}

bool DhcpPacket::CopyDhcpPacket(const char * buf, B_U32 bufLen)
{
	if (bufLen <= DHCP_FIXED_HEAD_SIZE) {
		return false;
	}
	memcpy(&packet, buf, DHCP_FIXED_HEAD_SIZE);

	if (sizeof(packet.options)+DHCP_FIXED_HEAD_SIZE < bufLen) {
		return false;
	}
	memcpy(&packet.options, buf+DHCP_FIXED_HEAD_SIZE, bufLen-DHCP_FIXED_HEAD_SIZE);
	optionSize = bufLen-DHCP_FIXED_HEAD_SIZE;
	posEndOpt = sizeof(DHCP_MAGIC_COOKIE);

	B_U8 len;
	while (DHCP_OPTION_END_E != packet.options[posEndOpt] && posEndOpt < sizeof(packet.options)-1) {
		//option;
		++posEndOpt;
		//len;
		len = packet.options[posEndOpt];
		posEndOpt += (len+1);
	}

	if (!IsValidOptions()) {
		return false;
	}

	return true;
	
}

bool DhcpPacket::IsValidOptions()
{	
	B_U32 cookie = (((((B_U32)packet.options[0])<<24)&0xff000000)
	              |((((B_U32)packet.options[1])<<16)&0x00ff0000)
	              |((((B_U32)packet.options[2])<<8)&0x0000ff00)
	              |(((B_U32)packet.options[3])&0xff));
	if (cookie != DHCP_MAGIC_COOKIE) {
		return false;
	}

	if (DHCP_OPTION_END_E != packet.options[optionSize-1]
	   && 0 != packet.options[optionSize-1]) {
		return false;
	}

	B_U8 *ptStart = packet.options;
	B_U8 *ptEnd = ptStart+sizeof(DHCP_MAGIC_COOKIE);
	bool bHasEndOption = false;
	B_U32 realSize = sizeof(DHCP_MAGIC_COOKIE);
	B_U8 opt;
	B_U8  len;

	while (ptStart+optionSize > ptEnd) {
		//type
		opt = *ptEnd;
		++ptEnd;
		++realSize;
		if (DHCP_OPTION_END_E == opt) {
			bHasEndOption = true;
			break;
		}
		else if (DHCP_OPTION_MSG_TYPE_E == opt) {
			pMsgType = ptEnd+1;
		}

		//len
		len = *ptEnd;
		++ptEnd;
		++realSize;

		//skip data;
		realSize += len;
		ptEnd += len;
	}

	optionSize = realSize;
	return bHasEndOption;
}

void DhcpPacket::Packet::init()
{
	opcode = 0;
	hardwareType = 0;
	hardwareLen = 0;
	hops = 0;		   
	transacId = 0;		
	secs = 0;		
	flags = 0;
	ciaddr = 0;
	yiaddr = 0;
	siaddr = 0;
	giaddr = 0;
	bzero(chaddr, sizeof(chaddr));
	bzero(sname, sizeof(sname));
	bzero(file, sizeof(file));
}

void DhcpPacket::SendPacket(SOCKET sock, struct sockaddr_in& sockaddrServer)
{
	int sendSize = SOCKET_ERROR;
	const char* data = (const char*)(&this->packet);

	int size = sizeof(sockaddrServer);

	//SetEndOption();

	sendSize = sendto(sock, data, (int)GetPacketSize(), 0, 
					  (struct sockaddr*)&sockaddrServer, 
					   size);
	if (SOCKET_ERROR == sendSize) {
		TRACE("SendPacket failed, error:%d\n",  WSAGetLastError());
	}
}

void DhcpPacket::SetEndOption()
{
	B_U8 *ptOpt = packet.options+posEndOpt;	
	//B_U8 *ptOpt = packet.options+sizeof(DHCP_MAGIC_COOKIE);	

#if 0	
	B_U8  len;
	while (*ptOpt && DHCP_OPTION_END_E != *ptOpt) {
		//option;
		++ptOpt;
		//len;
		len = *ptOpt;
		//skip data
		ptOpt += len;
	}
#endif
	*ptOpt = DHCP_OPTION_END_E;

	realSize = ptOpt-packet.options+1;
}

void DhcpPacket::SetMsgType(DHCP_MSG_TYPE_E eType)
{	
	if (NULL != pMsgType) {
		*pMsgType = static_cast<B_U8>(eType);
	}

	if (DHCP_MSG_TYPE_DISCOVER_E == eType 
	   || DHCP_MSG_TYPE_REQUEST_E == eType
	   || DHCP_MSG_TYPE_DECLINE_E == eType
	   || DHCP_MSG_TYPE_RELEASE_E == eType
	   || DHCP_MSG_TYPE_INFORM_E == eType) {
	   	packet.opcode = 1;
	}
	else {
		packet.opcode = 2;
	}
}
