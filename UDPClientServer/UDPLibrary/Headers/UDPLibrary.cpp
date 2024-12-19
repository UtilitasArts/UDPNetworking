//-------------|
// My Includes |
//=============|
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
void UDPSetup::WelcomeMessage(std::string name) {

	if (name == "") {
		std::cout << "Please enter your name with a maximum of 10 characters: ";
		std::string TempName;
		getline(std::cin, TempName);
		MyName = TempName.substr(0, 10);
		system("cls");
	}
	else {
		MyName = name;
	}
	std::cout << "- Thank you " << MyName << "\n";
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
	else {
		std::cout << "- The UDP Socket was succesfully bound to the local-adress\n";
	}
}
void UDPSetup::CheckSocketRecvBufferSize()
{
	int optVal;
	int optLen = sizeof(int);
	int err;

	err = getsockopt(UDPSocket, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, &optLen);
	err = ioctlsocket(UDPSocket, FIONREAD, (u_long*)&RecvBufferBytes);	

	if(err != SOCKET_ERROR)	{
		if (PrevRecvBufferBytes != RecvBufferBytes)
		{
			printf("- SockOpt BufferSize = %ld\n", optVal);
			printf("- Bytes available in the receive buffer: %d\n", RecvBufferBytes);
		}
	}
	PrevRecvBufferBytes = RecvBufferBytes;
}
void UDPSetup::UDPInit(uint16_t port, std::string name) {
	InstallFolders();
	WelcomeMessage(name);
	InitWinsock();
	OpenUDPSocket();
	BindSocket(port);
	CheckSocketRecvBufferSize();
}

//-----------|
// UDP Packs |
//===========|
//--------------------|
// Receiving messages |
//====================|
MessageType UDPPacks::RecvBytes(bool bPrint) {

	ReceiveAdress.SetAdress(0, 0, 0, 0, 0, "None", false);
	RecvMT	   = MessageType::None;
	RecvEcho   = MessageType::None;

	int BytesReceived = recvfrom(UDPSetup::UDPSocket, (char*)RecvBuffer, sizeof(RecvBuffer), 0, ReceiveAdress.GetSockAddr(), ReceiveAdress.GetAddrSize());
	if (BytesReceived != SOCKET_ERROR) {
		ReceiveAdress.FillFromSockAddr();
		RecvBytePack.SetByteArray(RecvBuffer, BytesReceived);

		if (RecvBytePack.GetCRCValid()) {

			RecvBytePack.ReturnBytes(RecvMT,   0);
			RecvBytePack.ReturnBytes(RecvEcho, 1);
			RecvBytePack.ReturnBytes(RecvID,   2);
			//------------------------|
			// Block certain messages |
			//========================| 
			if (BlockMap.count(MessageID(ReceiveAdress, RecvID))) {
				if (bPrint) { std::cout << "- Echo Message Blocked\n"; }
				ReceiveAdress.SetAdress(0, 0, 0, 0, 0, "None", false);
				RecvMT   = MessageType::None;
				RecvEcho = MessageType::None;
				return RecvMT;
			}	
	
			if (bPrint)	{
				std::cout << "- A " << MessageTypeToString(RecvMT) << " Received! \n";
				std::cout << "- " << MessageTypeToString(RecvEcho) << "\n";
				std::cout << "- ID" << (int)RecvID << "\n";
				RecvBytePack.PrintBytes();
			}

 			RecvEchoRequest(bPrint);
 			RecvEchoResponse(bPrint);
		}
	}
	return RecvMT;
}
//----------------------|
// Receive Echo Request |
//======================| 
void UDPPacks::RecvEchoRequest(bool bPrint) {
	if (RecvEcho == MessageType::EchoRequest) {

		BlockMap.emplace(MessageID(ReceiveAdress, RecvID));
		CreateEchoMessage(ReceiveAdress,MessageType::EchoResponse,RecvMT, RecvID);

		UDPPacks::SendBytes(UDPPacks::ServerAdress, true);
		UDPPacks::SendID++;
	}
}
//-----------------------|
// Receive Echo Response |
//=======================|
void UDPPacks::RecvEchoResponse(bool bPrint) {
	if (EchoMap.size() > 0)
	{	
		if (RecvMT == MessageType::EchoResponse) {
			if (EchoMap.count(MessageID(ReceiveAdress, RecvID))) {
				std::cout << "is in map!";
			}
			else{
				std::cout << "is Not in map!";
			}
			EchoMap.erase(MessageID(ReceiveAdress, RecvID));
			std::cout << "Removing message from echomap! \n ECHOSIZE =" << EchoMap.size() << "\n";

		}
	}
}
//---------------|
// Resend Echoes |
//===============| // we assume no one hears us.
void UDPPacks::SendEchoes(bool bPrint) {
	if (EchoMap.size() > 0)	{
		for (auto Chamber : EchoMap) {
			int bytesSent = sendto(UDPSetup::UDPSocket, Chamber.second.ResendBytePack.GetByteArrayAsChar(), static_cast<uint32_t>(Chamber.second.ResendBytePack.GetArraySize()), 0, Chamber.second.AdressContainer.GetSockAddr(), *Chamber.second.AdressContainer.GetAddrSize());
			if (bytesSent == SOCKET_ERROR) {
				std::cerr << "Failed to send echo." << "\n";
			}
			else {
				MessageType MType = static_cast<MessageType>(Chamber.second.ResendBytePack.GetByteArrayAsChar()[1]);
				std::cout << "\n- A " << MessageTypeToString(MType) << " Echo Sent! " << "\n";
				if (bPrint) { Chamber.second.ResendBytePack.PrintBytes(); }
			}			
		}
	}
}

//----------------|
// Send Functions |
//================|
void UDPPacks::SendBytes(AddrCtr& adress_ctr, bool bPrint) {
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
bool UDPPacks::RecvValidSessionAddress() {
	bool bValidAddress = false;

	for (size_t i = 0; i < ConnectedAddresses.size(); i++) {
		if (ReceiveAdress == ConnectedAddresses[i]) {
			bValidAddress = true;
		}
	}
	return bValidAddress;
}




