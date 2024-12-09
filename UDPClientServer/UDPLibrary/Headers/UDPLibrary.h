#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <type_traits>
#include <cassert>
#include <winsock2.h>
#include <WS2tcpip.h>
#include "BytePacker.h"
#pragma comment( lib, "wsock32.lib" )
#pragma comment( lib, "Ws2_32.lib" )


enum class MessageType : uint8_t {
	None,
	ConnectRequest,
	ConnectApproval,
	CreateSession,
	JoinSession,
	ProfilePackage,
};

inline std::string MessageTypeToString(MessageType& MesType) {
	switch (MesType) {
		case MessageType::ConnectRequest:
		return "ConnectRequest";
		break;
		case MessageType::ConnectApproval:
		return "ConnectApproval";
		break;
		case MessageType::ProfilePackage:
		return "ProfilePackage";
		break;
		default:
		case MessageType::None:
		return "None";
	}
}

 namespace UDPSetup {

	inline std::string  MyName;
	inline WSADATA     WSAData;
	inline SOCKET    UDPSocket;


	inline void WelcomeMessage() {
		std::cout << "Please enter your name: ";
		getline(std::cin, MyName);
		system("cls");
		std::cout << " - Thank you " << MyName << "\n";
	}
	inline void InitWinsock() {
		if (WSAStartup(MAKEWORD(2, 2), &WSAData) != NO_ERROR) {
			std::cerr << "Failed to initialize Winsock. \n";
			exit(0);
		}
	}
	inline void OpenUDPSocket() {
		UDPSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (UDPSocket == INVALID_SOCKET) {
			std::cerr << "Failed to create socket." << std::endl;
			WSACleanup();
			exit(0);
		}
		else {
			std::cout << " - A UDP Socket has been sucessfully opened!\n";
		}

		DWORD nonBlocking = 1;
		if (ioctlsocket(UDPSocket, FIONBIO, &nonBlocking) != NO_ERROR) {
			printf("ioctlsocket failed");
		}
		else {
			std::cout << " - The UDP Socket has been sucessfully set to NON-blocking!\n";
		}

	}

	inline void BindSocket(uint16_t port = 0)
	{
		sockaddr_in localAddress;
		localAddress.sin_family = AF_INET;
		if (port > 0) {
			localAddress.sin_port = htons(port);
		}
		localAddress.sin_addr.s_addr = INADDR_ANY; // Accept messages from any IP
		//localAddress.sin_addr.s_addr = htonl(address);

		if (bind(UDPSocket, (sockaddr*)&localAddress, sizeof(localAddress)) == SOCKET_ERROR) {
			std::cerr << "Failed to bind socket." << std::endl;
			closesocket(UDPSocket);
			WSACleanup();
			exit(0);
		}
		else{
			std::cout << " - The UDP Socket was succesfully bound to the local-adress\n";
		}

	}

	inline void InitConnect(uint16_t port = 0) { WelcomeMessage(); InitWinsock(); OpenUDPSocket(); BindSocket(port); }
}

struct AdressCtr
{
	AdressCtr() { InitSockAddr(); }

	AdressCtr(uint8_t octet_a, uint8_t octet_b, uint8_t octet_c, uint8_t octet_d, uint16_t port, std::string name , bool bPrint = false) :
		Port(port),
		Octets{octet_a,octet_b,octet_c,octet_d},
		IP( (octet_a << 24) | (octet_b << 16) | (octet_c << 8) | (octet_d)),
		AdressName(name) {
		InitSockAddr(IP, Port);
		if(bPrint) {PrintAdress();}
	}
 
 	AdressCtr(uint32_t ip, uint16_t port, std::string name, bool bPrint = false) :
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

	inline void FillFromSockAddr(std::string name = "Server", bool bPrint = false) {
		uint32_t SockIP = ntohl(SockAddress.sin_addr.s_addr);
		uint16_t SockPort = ntohs(SockAddress.sin_port);
		SetAdress(SockIP, SockPort, name, bPrint);
	}

 	void PrintAdress() {printf(" - %s adress = %d.%d.%d.%d:%d\n",AdressName.c_str(),Octets[0], Octets[1], Octets[2], Octets[3], Port); }

	inline uint32_t HostIP()			 { return IP;}
	inline uint16_t HostPort()			 { return Port; }
	inline uint32_t NetIP()				 { return htonl(IP); }
	inline uint16_t NetPort()			 { return htons(Port);}
	inline sockaddr_in* GetSockAddr_In() { return &SockAddress;}
	inline sockaddr* GetSockAddr()		 { return reinterpret_cast<sockaddr*>(&SockAddress); }
	inline int32_t*  GetAddrSize()		 { return &AddrSize;}

	inline void SendPack(BytePack& SendPackage, bool bPrint = false)
	{		
		SendPackage.AddCRC();
		int bytesSent = sendto(UDPSetup::UDPSocket, SendPackage.GetByteArrayAsChar(), static_cast<uint32_t>(SendPackage.GetArraySize()), 0, GetSockAddr(), AddrSize);
		MessageType MType = static_cast<MessageType>(SendPackage.GetByteArrayAsChar()[1]);

		if (bytesSent == SOCKET_ERROR) {
			std::cerr << "Failed to send message." << "\n";
		}
		else {
			std::cout << " - A " << MessageTypeToString(MType) << " Sent! " << "\n\n";
			if(bPrint){ SendPackage.PrintBytes(); }
		}
	}

private:	
	inline void InitSockAddr(uint32_t ip = 0, uint16_t port = 0) {
		SockAddress.sin_family		= AF_INET;
		SockAddress.sin_addr.s_addr = NetIP();
		SockAddress.sin_port		= NetPort();
	}

	std::string AdressName;

	uint16_t Port;	
	uint32_t IP; 
	uint8_t Octets[4];	

	sockaddr_in SockAddress;
	int32_t AddrSize = sizeof(SockAddress);	 
};



