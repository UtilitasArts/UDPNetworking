#pragma once
//-------------------|
// Standard Includes |
//===================|
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <type_traits>
#pragma comment( lib, "wsock32.lib" )
#pragma comment( lib, "Ws2_32.lib" )
//-------------|
// My Includes |
//=============|
#include "BytePacker.h"
#include "AddressCtr.h"
#include "UDPMessageEnums.h"

//--------------|
// Echo Chamber |
//==============|
struct EchoChamber {
	EchoChamber(AddrCtr addr_ctr, BytePack resend_pack): AdressContainer(addr_ctr), ResendBytePack(resend_pack) {}
	AddrCtr  AdressContainer;
	BytePack ResendBytePack;

	EchoChamber& operator=(const EchoChamber& other) {		
		return *this;
	}
};

//-----------|
// UDP Setup |
//===========|

namespace UDPSetup {	
	//------------------|
	// Public Variables |
	//==================|
	inline std::string	MyName;
	inline WSADATA      WSAData;
	inline SOCKET       UDPSocket;
	inline uint32_t		RecvBufferBytes;	
	inline uint32_t		PrevRecvBufferBytes;
	inline std::filesystem::path RestartFolder;
	inline std::filesystem::path ReposFolder;
	//------------------|
	// Public Functions |
	//==================|
	void InstallFolders();
	void WelcomeMessage(std::string name = "");
	void InitWinsock();
	void OpenUDPSocket();
	void BindSocket(uint16_t port = 0);
	void UDPInit(uint16_t port = 0, std::string name = "");
	void CheckSocketRecvBufferSize();
}

//--------------|
// UDP Packages |
//==============|

namespace UDPPacks {
	//------------------|
	// Public Variables |
	//==================|
	inline BytePack	SendBytePack;
	inline BytePack	ResendBytePack;
	inline BytePack	RecvBytePack;
	inline AddrCtr  ReceiveAdress;
	inline AddrCtr  MyPublicAdress;
	inline AddrCtr  ServerAdress(80, 61, 175, 45, 8000, "Server", false);
	inline std::vector<AddrCtr>ConnectedAddresses;

	inline unsigned char RecvBuffer[1500];
	inline MessageType	 RecvMT;
	inline MessageType	 Echo;

	inline std::vector<EchoChamber> EchoArray;

	//------------------|
	// Public Functions |
	//==================|
	MessageType RecvBytes(bool bPrint);
 	void RecvEchoResponse(bool bPrint);
 	void RecvEchoRequest(bool bPrint);
	void SendBytes(AddrCtr& adress_ctr, bool bEcho = false, bool bPrint = false);
	void SendEchoes(bool bPrint);
	bool RecvValidSessionAddress();
}


