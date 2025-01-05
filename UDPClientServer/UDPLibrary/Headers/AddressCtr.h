#pragma once

#include <vector>
#include <string>
#include <type_traits>
#include <cassert>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <filesystem>
#pragma comment( lib, "wsock32.lib" )
#pragma comment( lib, "Ws2_32.lib" )

struct AddrCtr
{
	AddrCtr() { InitSockAddr(); }

	AddrCtr(uint8_t octet_a, uint8_t octet_b, uint8_t octet_c, uint8_t octet_d, uint16_t port, std::string name , bool bPrint = false) :
		Port(port),
		Octets{octet_a,octet_b,octet_c,octet_d},
		IP( (octet_a << 24) | (octet_b << 16) | (octet_c << 8) | (octet_d)),
		AdressName(name) {
		InitSockAddr(IP, Port);
		if(bPrint) {PrintAdress();}
	}
 
 	AddrCtr(uint32_t ip, uint16_t port, std::string name, bool bPrint = false) :
 		IP(ip),
 		Port(port),	
		Octets{ (ip >> 24 & 0xFF),(ip >> 16 & 0xFF),(ip >> 8 & 0xFF),(ip & 0xFF) },
		AdressName(name) {
		InitSockAddr(IP, Port);
		if(bPrint) {PrintAdress();}
	}

	inline void SetAdress(uint32_t ip, uint16_t port, std::string name, bool bPrint = false) {
		IP = ip;
		Port = port; 
 		Octets[0] = (ip >> 24 & 0xFF);
		Octets[1] = (ip >> 16 & 0xFF);
		Octets[2] = (ip >>  8 & 0xFF);
		Octets[3] = (ip		  & 0xFF);
 		AdressName = name;
		InitSockAddr(IP, Port);
		if (bPrint) { PrintAdress(); }
	}

	inline void SetAdress(uint8_t octet_a, uint8_t octet_b, uint8_t octet_c, uint8_t octet_d, uint16_t port, std::string name, bool bPrint = false) {
		Port = port;
		Octets[0] = octet_a; Octets[1] = octet_b; Octets[2] = octet_c; Octets[3] = octet_d;
 		IP = (octet_a << 24) | (octet_b << 16) | (octet_c << 8) | (octet_d);
		AdressName = name;
		InitSockAddr(IP, Port);
		if (bPrint) { PrintAdress(); }
	}

	inline void FillFromSockAddr(std::string name = "RecvAddr", bool bPrint = false) {
		uint32_t SockIP = ntohl(SockAddress.sin_addr.s_addr);
		uint16_t SockPort = ntohs(SockAddress.sin_port);
		SetAdress(SockIP, SockPort, name, bPrint);
	}

	inline void SetName(std::string name){
		AdressName = name;
	}

 	void PrintAdress() {printf("%-10s address = %d.%d.%d.%d:%d\n",AdressName.c_str(),Octets[0], Octets[1], Octets[2], Octets[3], Port); }

	inline uint32_t&	HostIP()		 { return IP;}
	inline uint16_t&	HostPort()		 { return Port; }
	inline uint32_t		NetIP()			 { return htonl(IP); }
	inline uint16_t		NetPort()		 { return htons(Port);}
	inline sockaddr_in* GetSockAddr_In() { return &SockAddress;}
	inline sockaddr*	GetSockAddr()	 { return reinterpret_cast<sockaddr*>(&SockAddress); }
	inline int32_t*		GetAddrSize()	 { return &AddrSize;}
	inline std::string& GetAddrName()	 { return AdressName; }

private:	
	inline void InitSockAddr(uint32_t ip = 0, uint16_t port = 0) {
		SockAddress.sin_family		= AF_INET;
		SockAddress.sin_addr.s_addr = NetIP();
		SockAddress.sin_port		= NetPort();
	}

	uint32_t	IP; 
	uint16_t	Port;	
	uint8_t		Octets[4];	
	std::string AdressName;

	sockaddr_in SockAddress;
	int32_t AddrSize = sizeof(SockAddress);	 

public:

	bool operator ==(const AddrCtr& obj) {
		if (IP == obj.IP && Port == obj.Port){
			return true;
		}
		else {
			return false;
		}
	}

	bool operator !=(const AddrCtr& obj) {
		if (IP != obj.IP || Port != obj.Port) {
			return true;
		}
		else {
			return false;
		}
	}

	bool operator =(const AddrCtr& obj) {
		SetAdress(obj.IP,obj.Port,obj.AdressName);
	}


};

