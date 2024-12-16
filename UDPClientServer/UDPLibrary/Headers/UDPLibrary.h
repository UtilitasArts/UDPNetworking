#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <type_traits>
#include <cassert>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <filesystem>
#include "BytePacker.h"
#pragma comment( lib, "wsock32.lib" )
#pragma comment( lib, "Ws2_32.lib" )

namespace fs = std::filesystem;


enum class MessageType : uint8_t {
	None,
	ConnectRequest,
	ConnectApproval,
	CreateRequest,
	CreateApproval,
	JoinRequest,
	JoinApproval,
	JoinNotify,
	SessionStart,
	UpdateRequest,
	UpdateApproval,
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
		case MessageType::CreateRequest:
			return "CreateRequest";
			break;
		case MessageType::CreateApproval:
			return "CreateApproval";
			break;
		case MessageType::JoinRequest:
			return "ConnectRequest";
			break;
		case MessageType::JoinApproval:
			return "JoinApproval";
			break;
		case MessageType::JoinNotify:
			return "JoinNotify";
			break;
		case MessageType::SessionStart:
			return "SessionStart";
			break;
		case MessageType::UpdateRequest:
			return "UpdateRequest";
			break;
		case MessageType::UpdateApproval:
			return "UpdateApproval";
			break;
		case MessageType::ProfilePackage:
			return "ProfilePackage";
			break;
		default:
			return "None";
	}
}

namespace CMD {

	template <typename T, typename... Args>
	inline std::string MultiCMD(T first, Args&... rest)
	{
		std::string CombinedCMD = first;
		((CombinedCMD += " && " + rest), ...);

		return CombinedCMD;
	}

	inline std::string Command(std::string command, std::string vars = "")
	{
		return command + vars;
	}

	inline std::string Terminal(std::string command, std::string vars = "/K")
	{
		return "start cmd " + vars +  "\"" + command + "\"";
	}

	inline std::string SetPath(std::filesystem::path path) {
		return "cd \"" + path.string() + "\"";
	}

	inline std::string SetString(std::string string) {
		return "\"" + string + "\"";
	}
}

 namespace UDPSetup {

	inline std::string     MyName;
	inline WSADATA        WSAData;
	inline SOCKET       UDPSocket;
	inline fs::path RestartFolder;
	inline fs::path   ReposFolder;

	inline void InstallFolders() {
		fs::path targetFolder = "UDPClientServer";
		fs::path Location = fs::current_path();	

		for (const fs::path& part : Location) {
			RestartFolder /= part;
			if (part == targetFolder) {
				break;
			}
		} ReposFolder = RestartFolder.parent_path();
		RestartFolder = RestartFolder / "x64" / "Release";
	}

	inline void WelcomeMessage(std::string name = "") {

		if (name == "")
		{
			std::cout << "Please enter your name with a maximum of 10 characters: ";

			std::string TempName;
			getline(std::cin, TempName);
			MyName = TempName.substr(0,10);
			system("cls");
		}
		else
		{
			MyName = name;
		}

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

	inline void UDPInit(uint16_t port = 0, std::string name = "") { InstallFolders(); WelcomeMessage(name); InitWinsock(); OpenUDPSocket(); BindSocket(port); }
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

 	void PrintAdress() {printf("- %-10s adress = %d.%d.%d.%d:%d\n",AdressName.c_str(),Octets[0], Octets[1], Octets[2], Octets[3], Port); }

	inline uint32_t& HostIP()			 { return IP;}
	inline uint16_t& HostPort()			 { return Port; }
	inline uint32_t  NetIP()			 { return htonl(IP); }
	inline uint16_t  NetPort()			 { return htons(Port);}
	inline sockaddr_in* GetSockAddr_In() { return &SockAddress;}
	inline sockaddr* GetSockAddr()		 { return reinterpret_cast<sockaddr*>(&SockAddress); }
	inline int32_t*  GetAddrSize()		 { return &AddrSize;}
	inline std::string& GetAddrName()	 { return AdressName; }

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

public:

	bool operator ==(const AdressCtr& obj) {
		if (IP == obj.IP && Port == obj.Port){
			return true;
		}
		else {
			return false;
		}
	}

	bool operator !=(const AdressCtr& obj) {
		if (IP != obj.IP || Port != obj.Port) {
			return true;
		}
		else {
			return false;
		}
	}
};

namespace UDPPacks {
	inline BytePack   SendBytePack;
	inline BytePack   RecvBytePack;
	inline AdressCtr  ReceiveAdress;
	inline AdressCtr  PublicAdress;
	inline AdressCtr  ServerAdress(80, 61, 175, 45, 8000, "Server", false);

	inline std::vector<AdressCtr>ConnectedAddresses;

	inline unsigned char RecvBuffer[1024];

	inline MessageType RecvMT;

	inline MessageType RecvBytes(bool bPrint){
		ReceiveAdress.SetAdress(0, 0, 0, 0, 0, "None", false);
		RecvMT = MessageType::None;

		int BytesReceived = recvfrom(UDPSetup::UDPSocket, (char*)RecvBuffer, sizeof(RecvBuffer), 0, ReceiveAdress.GetSockAddr(), ReceiveAdress.GetAddrSize());
		if (BytesReceived != SOCKET_ERROR) {

			ReceiveAdress.FillFromSockAddr();
			RecvBytePack.SetByteArray(RecvBuffer, BytesReceived);

			if (RecvBytePack.GetCRCValid())	{
				RecvBytePack.ReturnBytes(RecvMT, 0); 
				std::cout << "- A " << MessageTypeToString(RecvMT) << " Received! \n";

				if (bPrint)
				{
					RecvBytePack.PrintBytes();
				}
			}
		}
		return RecvMT;
	}

	inline void SendBytes(AdressCtr& adress_ctr, bool bPrint = false){
		SendBytePack.AddCRC();
		int bytesSent = sendto(UDPSetup::UDPSocket, SendBytePack.GetByteArrayAsChar(), static_cast<uint32_t>(SendBytePack.GetArraySize()), 0, adress_ctr.GetSockAddr(), *adress_ctr.GetAddrSize());
		MessageType MType = static_cast<MessageType>(SendBytePack.GetByteArrayAsChar()[1]);
		if (bytesSent == SOCKET_ERROR) {
			std::cerr << "Failed to send message." << "\n";
		}
		else {
			std::cout << "\n- A " << MessageTypeToString(MType) << " Sent! " << "\n";
			if (bPrint) { SendBytePack.PrintBytes(); }
		}
	}

	inline bool RecvValidSessionAddress()	{
		bool bValidAddress = false; 

		for (size_t i = 0; i < ConnectedAddresses.size(); i++){
			if (ReceiveAdress == ConnectedAddresses[i])	{
				bValidAddress = true;
			}
		}
		return bValidAddress;
	}
}


