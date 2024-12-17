#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <type_traits>
#include <cassert>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment( lib, "wsock32.lib" )
#pragma comment( lib, "Ws2_32.lib" )

#include <filesystem>
namespace fs = std::filesystem;

#include "BytePacker.h"
#include "AddressCtr.h"
#include "UDPMessageEnums.h"

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

		if (name == "")	{
			std::cout << "Please enter your name with a maximum of 10 characters: ";
			std::string TempName;
			getline(std::cin, TempName);
			MyName = TempName.substr(0,10);
			system("cls");
		}
		else{
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

namespace UDPPacks {
	inline BytePack	SendBytePack;
	inline BytePack	RecvBytePack;
	inline AddrCtr  ReceiveAdress;
	inline AddrCtr  PublicAdress;
	inline AddrCtr  ServerAdress(80, 61, 175, 45, 8000, "Server", false);

	inline std::vector<AddrCtr>ConnectedAddresses;

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

	inline void SendBytes(AddrCtr& adress_ctr, bool bPrint = false){
		SendBytePack.AddCRC();
		int bytesSent	  = sendto(UDPSetup::UDPSocket, SendBytePack.GetByteArrayAsChar(), static_cast<uint32_t>(SendBytePack.GetArraySize()), 0, adress_ctr.GetSockAddr(), *adress_ctr.GetAddrSize());
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


