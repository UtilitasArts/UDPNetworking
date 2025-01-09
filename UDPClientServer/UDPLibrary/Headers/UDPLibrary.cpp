#include <thread>
#include <chrono>
#include "UDPLibrary.h"


namespace fs = std::filesystem;

//-----------|
// UDP Setup |
//===========|
void UDPSetup::InstallFolders() {
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
void UDPSetup::InitWinsock() {
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != NO_ERROR) {
		std::cerr << "Failed to initialize Winsock. \n";
		exit(0);
	}
}
void UDPSetup::OpenUDPSocket() {
	UDPSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (UDPSocket == INVALID_SOCKET) {
		std::cerr << "Failed to create socket." << std::endl;
		WSACleanup();
		exit(0);
	}
	else {
		std::cout << "- A UDP Socket has been sucessfully opened!\n";
	}

	DWORD nonBlocking = 1;
	if (ioctlsocket(UDPSocket, FIONBIO, &nonBlocking) != NO_ERROR) {
		printf("ioctlsocket failed");
	}
	else {
		std::cout << "- The UDP Socket has been sucessfully set to NON-blocking!\n";
	}
}
void UDPSetup::BindSocket(uint16_t port) {
	sockaddr_in SetupSockAddr;
	SetupSockAddr.sin_family = AF_INET;
	if (port > 0) {
		//-------------------|
		// Ports for Servers |
		//===================|
		SetupSockAddr.sin_port = htons(port);
	}
	else{
		//-------------------|
		// Ports for clients |
		//===================|
		SetupSockAddr.sin_port = 0;
	}
	SetupSockAddr.sin_addr.s_addr = INADDR_ANY; // Accept messages from any IP
	//localAddress.sin_addr.s_addr = htonl(address);

	if (bind(UDPSocket, (sockaddr*)&SetupSockAddr, sizeof(SetupSockAddr)) == SOCKET_ERROR) {
		std::cerr << "Failed to bind socket." << std::endl;
		closesocket(UDPSocket);
		WSACleanup();
		exit(0);
	}
	else {
		std::cout << "- The UDP Socket was succesfully bound to the local-adress\n";

		//-------------|
		// Port Number |
		//=============|
		int addrlen = sizeof(SetupSockAddr);
		if (getsockname(UDPSocket, (struct sockaddr*)&SetupSockAddr, &addrlen) == -1) {
			exit(0);
		}

		GetLocalAddr(SetupSockAddr.sin_port);
	}
}
void UDPSetup::GetLocalAddr(uint16_t port) {

	//---------------|
	// Get Host Name |
	//===============|
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)) != 0) {
		exit(0);
	}

	UDPSetup::MyName = hostname;

	//---------------------------------------|
	// Prepare emptry data for getaddrinfo() |
	//=======================================|
	DWORD dwRetval;
	struct addrinfo hints;
	struct addrinfo *result = NULL;
	struct addrinfo *ptr	= NULL;

	ZeroMemory(&hints, sizeof(hints));
	//-------------------------------------|
	// Set search settings for getaddrinfo |
	//=====================================|
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = AF_INET;
	hints.ai_protocol = IPPROTO_UDP;

	//---------------------|
	// retrieve local IP's |
	//=====================|
	dwRetval = getaddrinfo(hostname, NULL, &hints, &result);
	if (dwRetval != 0) {
		printf("getaddrinfo failed with error: %d\n", dwRetval);
		WSACleanup();
		exit(0);
	}

	//----------------------------------|
	// Get ipv4 address from array|
	//==================================|
	struct sockaddr_in* sockaddr_ipv4;

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		switch (ptr->ai_family) {
		case AF_INET:

			//------------------------------------------------------------|
			// Get IP from sockaddr struct and store in our own container |
			//============================================================|
			sockaddr_ipv4 = (struct sockaddr_in*)ptr->ai_addr;

			uint32_t IP = sockaddr_ipv4->sin_addr.s_addr;
 			uint_convert<uint32_t>::NetToHost(IP);
			uint_convert<uint16_t>::NetToHost(port);

			uint8_t octet = (IP >> 24 & 0xFF);		

			//---------------------|
			// Store Local Address |
			//=====================|

			if (octet == 192) {
				std::cout << "- Local";
				LocalAddress.SetAdress(IP, port, hostname, true);
				return;
			}		

			break;
		}
	}












	
}

bool UDPSetup::SocketHasNewBytes()
{
	int optVal;
	int optLen = sizeof(int);
	int err;

	err = getsockopt(UDPSocket, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, &optLen);
	err = ioctlsocket(UDPSocket, FIONREAD, (u_long*)&RecvBufferBytes);	

	if (err != SOCKET_ERROR) {
		if (PrevRecvBufferBytes != RecvBufferBytes && RecvBufferBytes > 0) {
			//printf("- Bytes in the buffer: %d / %d \n", RecvBufferBytes, optVal);
		}

		PrevRecvBufferBytes = RecvBufferBytes;

		if (RecvBufferBytes >= 1) {
			return true;
		}
	}

	return false;
}
void UDPSetup::UDPInit(uint16_t port) {
	InstallFolders();	
	InitWinsock();
	OpenUDPSocket();
	BindSocket(port);
	std::cout << "\n";
}
//-----------|
// UDP Packs |
//===========|
void printWSAError() {
	int errorCode = WSAGetLastError(); // Retrieve the last error code

	std::cout << "WSAGetLastError returned: " << errorCode << std::endl;
	wchar_t* s = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);
	fprintf(stderr, "%S\n", s);
	LocalFree(s);	
}
// -------------------|
// Receiving messages |
//====================|
MessageType UDPPacks::RecvBytes(bool bPrint, bool bIsServer) {
	ReceiveAdress.SetAdress(0, 0, 0, 0, 0, "None", false);
	RecvMT	   = MessageType::None;
	RecvEcho   = MessageType::None;

	int BytesReceived = recvfrom(UDPSetup::UDPSocket, (char*)RecvBuffer, sizeof(RecvBuffer), 0, ReceiveAdress.GetSockAddr(), ReceiveAdress.GetAddrSize());
	if (BytesReceived != SOCKET_ERROR) {

		// ------------------------|
		// Get Address and Package |
		//=========================|
		ReceiveAdress.FillFromSockAddr();
		RecvBytePack.SetByteArray(RecvBuffer, BytesReceived);	

		// ---------------------------------------------|
		// If Package is correct we identify the sender |
		//==============================================|
		if (RecvBytePack.GetCRCValid()) {
			if (ReceiveAdress != UDPPacks::ServerAdress && !bIsServer)	{			
				if (AddrMap.count(ReceiveAdress)  <= 0)	{
					if (bPrint) {
						printf("* << Receive Unauthorised Message fr ");
						ReceiveAdress.PrintAdress();
					}

					return RecvMT;
				}
				else
				{
					ReceiveAdress = AddrMap.at(ReceiveAdress).PublicAddress;
				}
			}		
			else if (ReceiveAdress == UDPPacks::ServerAdress){	
				ReceiveAdress = UDPPacks::ServerAdress;
			}
			else
			{
				std::cout << "someone sends";
			}

			// --------------------------|
			// Identify the message type |
			//===========================|
			RecvBytePack.ReturnBytes(RecvMT,   0);
			RecvBytePack.ReturnBytes(RecvEcho, 1);
			RecvBytePack.ReturnBytes(RecvID,   2);

			//------------------------|
			// Block certain messages |
			//========================| 
			if (RecvEchoRequest(bPrint) || RecvEchoResponse(bPrint)) { return RecvMT; };

			if (bPrint) {
				printf("* << Receive [ ");
				printf("%-*s", 15, MessageTypeToString(RecvMT).c_str());
				printf(" ] [ ");
				printf("%-*s", 15, MessageTypeToString(RecvEcho).c_str());
				printf(" ] [MsgID:");
				printf("#%04d", (int)RecvID);
				printf(" ] fr ");
				ReceiveAdress.PrintAdress();	
			}
		}	
	}
	else {
// 		std::cout << "Some Recv Message Error occured \n";
// 		printWSAError();
	}
	return RecvMT;
}
//----------------------|
// Receive Echo Request |
//======================| 
bool UDPPacks::RecvEchoRequest(bool bPrint = false) {
	 if (RecvEcho == MessageType::EchoRequest) {
		 CreateEchoMessage(MessageType::EchoResponse, RecvMT, RecvID);
		 UDPPacks::SendBytes(UDPPacks::ReceiveAdress, true);

		 if (!BlockMap.emplace(MessageID(ReceiveAdress, RecvID)).second) {
			 ReceiveAdress.SetAdress(0, 0, 0, 0, 0, "None", false);
			 RecvMT   = MessageType::None;
			 RecvEcho = MessageType::None;
			 return true;
		 }
	 }
	 return false;
 }
//-----------------------|
// Receive Echo Response |
//=======================|
bool UDPPacks::RecvEchoResponse(bool bPrint) {
	if (RecvMT == MessageType::EchoResponse) {		
		if (EchoMap.size() > 0){			
			if (EchoMap.count(MessageID(ReceiveAdress, RecvID))) {
				if (bPrint) {
					printf("* << Receive [ ");
					printf("%-*s", 15, MessageTypeToString(RecvMT).c_str());
					printf(" ] [ ");
					printf("%-*s", 15, MessageTypeToString(RecvEcho).c_str());
					printf(" ] [MsgID:");
					printf("#%04d", (int)RecvID);
					printf(" ] fr ");
					ReceiveAdress.PrintAdress();
				}
				std::cout << "* -- Echo with RTT of " << MessageTimer.CalcDuration(EchoMap.at(MessageID(ReceiveAdress, RecvID)).SendTime) << "MS ";
				EchoMap.erase(MessageID(ReceiveAdress, RecvID));
				std::cout << " Removing echo_message from EchoMap! ECHOSIZE =" << EchoMap.size() << "\n";
				return true;
			}
			else
			{
				std::cout << "* -- Received an echo response that was considered lost or never requested from ";
				ReceiveAdress.PrintAdress();

				return true;
			}
		}	
	}
	return false;
}
//---------------|
// Resend Echoes |
//===============| // we assume no one hears us.
void UDPPacks::SendEchoes(bool bPrint) {

	if (EchoMap.size() > 0)	{

		for (auto Chamber = EchoMap.begin(); Chamber != EchoMap.end(); ) {
			
			//--------------------------------| 
			// Add new SendID to the packages |
			//================================|

			if (MessageTimer.CalcDuration(Chamber->second.SendTime) > 3000){
				uint32_t NetSendID = SendID;
				if (isLittleEndian()) {
					uint_convert<uint32_t>::HostToNet(NetSendID);
					memcpy(Chamber->second.ResendBytePack.GetByteArray() + 5, &NetSendID, sizeof(SendID));
				}
				else{
					memcpy(Chamber->second.ResendBytePack.GetByteArray() + 5, &NetSendID, sizeof(SendID));
				}

				//----------------|
				// Resend Message |
				//================|
				printf("-- [New Echo] --\n");
				SendBytes(Chamber->second.AdressContainer,true,Chamber->second.ResendBytePack);

				Chamber = EchoMap.erase(Chamber);			
			}
			else
			{
				Chamber++;
			}
	
		}
	}

	if (TempEchoMap.size() > 0)
	{
		EchoMap.merge(TempEchoMap);
		TempEchoMap.clear();
	}
}

//----------------|
// Send Functions |
//================|
void UDPPacks::SendBytes(AddrCtr& address_ctr, bool bPrint, BytePack send_pack) {

	MessageType SendMT;
	MessageType SendEcho;
	uint32_t	MsgSendID;	
	send_pack.ReturnBytes(SendMT,   0);
	send_pack.ReturnBytes(SendEcho, 1);
	send_pack.ReturnBytes(MsgSendID,2);	
	
	if (SendEcho == MessageType::EchoRequest) {
		TempEchoMap.emplace(MessageID(address_ctr, MsgSendID), EchoChamber(address_ctr, send_pack, MessageTimer.CurrentTime()));
	}

	send_pack.AddCRC();

	int bytesSent = sendto(UDPSetup::UDPSocket, send_pack.GetByteArrayAsChar(), static_cast<uint32_t>(send_pack.GetArraySize()), 0, address_ctr.GetSockAddr(), *address_ctr.GetAddrSize());

	if (bytesSent == SOCKET_ERROR) {
		std::cerr << "Failed to send message." << "\n";
	}
	else {
		printf("* >> Sending [ ");
		printf("%-*s", 15, MessageTypeToString(SendMT).c_str());
		printf(" ] [ ");
		printf("%-*s", 15, MessageTypeToString(SendEcho).c_str());
		printf(" ] [MsgID:");
		printf("#%04d", (int)MsgSendID);
		printf(" ] to ");
		address_ctr.PrintAdress();
	}
	SendID++;

	//std::cout << SendID;
}

