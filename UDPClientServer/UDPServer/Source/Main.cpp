#pragma once
#include <iostream>
#include <filesystem>
#include "SessionStateMachine.h"

#include "CMDCommands.h"
namespace fs = std::filesystem;

std::vector<SessionStateMachine*> Sessions;

void ConnectRequest(){

	uint8_t  SessionCount = static_cast<uint8_t>(Sessions.size());	
	UDPPacks::CreateEchoMessage(MessageType::ConnectApproval, MessageType::EchoRequest, UDPPacks::SendID,
								true,
								UDPPacks::ReceiveAdress.HostIP(),
								UDPPacks::ReceiveAdress.HostPort(),
								SessionCount);

 	if (SessionCount > 0) {
 		for (size_t i = 0; i < Sessions.size(); i++) {				
 			UDPPacks::AddMessageData(Sessions[i]->SessionName,
 									 Sessions[i]->SessionSize,
 									 Sessions[i]->JoinedCount,
 									 Sessions[i]->CurrentStateEnum);
 
 			std::cout << "SessionState = " << ESessionStateString(Sessions[i]->CurrentStateEnum);
 		}
 	}

	UDPPacks::SendBytes(UDPPacks::ReceiveAdress,true);
}
//
void CreateSession() {
	std::cout << "Received: \n"; UDPPacks::ReceiveAdress.PrintAdress();
	std::string Name;
 	std::string RoomID;
	uint8_t AmountOfPlayers;

	UDPPacks::RecvBytePack.ReturnBytes(Name, 3);
 	UDPPacks::RecvBytePack.ReturnBytes(RoomID, 4);
	UDPPacks::RecvBytePack.ReturnBytes(AmountOfPlayers, 5);

	SessionStateMachine* NewSession = new SessionStateMachine(RoomID, AmountOfPlayers);
	std::cout << "- " << Name << " Created a room with ID:" << RoomID << " for " << (int)AmountOfPlayers << " players.\n";

	Sessions.push_back(NewSession);
	UDPPacks::ReceiveAdress.SetName(Name);

	UDPPacks::CreateEchoMessage(MessageType::CreateApproval, MessageType::EchoRequest, UDPPacks::SendID,
								true);
	UDPPacks::SendBytes(UDPPacks::ReceiveAdress, true);

	NewSession->JoinSession();
}

void JoinSession() {
	std::cout << "Received: \n"; UDPPacks::ReceiveAdress.PrintAdress();

	std::string Name;
	uint8_t RoomNr;
	UDPPacks::RecvBytePack.ReturnBytes(Name, 3);
	UDPPacks::RecvBytePack.ReturnBytes(RoomNr, 4);

	std::cout << Name << " = Joining RoomNr " << (int)RoomNr << "\n";
	UDPPacks::ReceiveAdress.SetName(Name);
	Sessions[RoomNr]->JoinSession();
}

void UpdateServer() {
	std::cout << "- Request to update server: \n";	
	UDPPacks::CreateEchoMessage( MessageType::UpdateApproval, MessageType::EchoRequest, UDPPacks::SendID);
	UDPPacks::SendBytes(UDPPacks::ReceiveAdress, true);

	//todo vertel iedereen te updaten.

	closesocket(UDPSetup::UDPSocket);
	WSACleanup();

	std::string RestartPath		= CMD::SetPath(UDPSetup::RestartFolder);
	std::string BatchFile		= CMD::Command("Update.bat " ,CMD::SetString("UDPServer.exe"));
	std::string FinalCommand	= CMD::MultiCMD(RestartPath, BatchFile);

	std::cout << "- Update of server was approved, Restarting now";

	system(FinalCommand.c_str());
	exit(0);

}

int main(){

    	UDPSetup::UDPInit(8000,"Server"); 
		std::cout << "\n - Waiting for clients \n";

		Timer	 GlobalTimer;
		uint64_t Time = 0;
		uint64_t Accumulator = 0;
		uint64_t FrameCount = 0;
		const int64_t FixedTimeStep = 16666667;
		const uint64_t NanoSecond   = 1000000000;

		//---------------------|
		// Fixed 60 FPS update |
		//=====================|
		while (true) {
			Accumulator += GlobalTimer.GetDeltaTimeNS();
			while (Accumulator >= FixedTimeStep) {
				FrameCount++; Time += FixedTimeStep;	Accumulator -= FixedTimeStep;

				//--------------|
				// Frame Update |
				//==============|

				while (UDPSetup::SocketHasNewBytes()) {
					UDPPacks::RecvBytes(true);

					switch (UDPPacks::RecvMT) {
					case MessageType::ConnectRequest:
						ConnectRequest();
						break;
					case MessageType::CreateRequest:
						CreateSession();
						break;
					case MessageType::JoinRequest:
						JoinSession();
						break;
					case MessageType::UpdateRequest:
						UpdateServer();
						break;
					}
				}

				UDPPacks::SendEchoes(true);
			}
		}

	return 0;

}



 
 
 //int main()
 //{
 //    const char* portchar = "8000";
 //
 //	unsigned short port;
 //
 //
 //	if (sscanf_s(portchar, "%hu", &port) != 1) {
 //		printf("Port not specified.\n");
 //	}
 //
 //	// Initialize Winsock
 //	WSADATA wsaData;
 //	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
 //		std::cerr << "Failed to initialize Winsock." << std::endl;
 //		return 1;
 //	}
 //
 //	// Create a socket
 //	SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
 //	if (udpSocket == INVALID_SOCKET) {
 //		std::cerr << "Failed to create socket." << std::endl;
 //		WSACleanup();
 //		return 1;
 //	}
 //
 //	// Prepare the local address structure
 //	sockaddr_in localAddress;
 //	localAddress.sin_family = AF_INET;
 //	localAddress.sin_port = htons(port); // The same port as the sender
 //	localAddress.sin_addr.s_addr = INADDR_ANY; // Accept messages from any IP
 //	//localAddress.sin_addr.s_addr = htonl(address);
 //
 //	if (bind(udpSocket, (sockaddr*)&localAddress, sizeof(localAddress)) == SOCKET_ERROR) {
 //		std::cerr << "Failed to bind socket." << std::endl;
 //		closesocket(udpSocket);
 //		WSACleanup();
 //		return 1;
 //	}
 //
 //	int addrlen = sizeof(localAddress);
 //	if (getsockname(udpSocket, (sockaddr*)&localAddress, &addrlen) == SOCKET_ERROR) {
 //		std::cerr << "Failed to get socket name." << std::endl;
 //		closesocket(udpSocket);
 //		WSACleanup();
 //		return 1;
 //	}
 //
 //	// Print the public port
 //	const unsigned short publicPort = ntohs(localAddress.sin_port);
 //	char ipAddr[INET_ADDRSTRLEN];
 //	InetNtopA(AF_INET, &(localAddress.sin_addr), ipAddr, INET_ADDRSTRLEN);
 //
 //	std::cout << "Listening on port: " << ipAddr << ":" << publicPort << "\n\n";
 //
 //
 //	// Receive messages
 //	while (true) {
 //		unsigned char buffer[1024];
 //		sockaddr_in senderAddress;
 //		senderAddress.sin_family = AF_INET;
 //		int senderAddressSize = sizeof(senderAddress);
 //
 //		int bytesReceived = recvfrom(udpSocket, (char*)buffer, sizeof(buffer), 0, (sockaddr*)&senderAddress, &senderAddressSize);
 //		if (bytesReceived == SOCKET_ERROR) {
 //			std::cerr << "Failed to receive message." << std::endl;
 //			break; // Exit the loop on error
 //		}
 //
 //		char ipAddr[INET_ADDRSTRLEN];
 //		InetNtopA(AF_INET, &(senderAddress.sin_addr), ipAddr, INET_ADDRSTRLEN);
 //		const unsigned short sendport = ntohs(senderAddress.sin_port);
 //
 //		// Null-terminate the received data and print it
 //		buffer[bytesReceived] = '\0'; // Ensure null termination for printing
 //		std::cout << "from" << ipAddr << ":" << sendport << "\n" << "Received message New: " << buffer << std::endl;
 //
 //		// Prepare the message to send
 //
 //		char TempMessage[100];
 //		snprintf(TempMessage, sizeof(TempMessage), "Your Public IP Address =: %s, your Public Port = : %hu", ipAddr, sendport);
 //		const char* message = TempMessage;
 //
 //		// Send the message
 //		int bytesSent = sendto(udpSocket, message, (int)strlen(message), 0, (sockaddr*)&senderAddress, sizeof(senderAddress));
 //		if (bytesSent == SOCKET_ERROR)
 //		{
 //			std::cerr << "Failed to send message." << std::endl;
 //		}
 //		else
 //		{
 //			std::cout << "Message sent: " << message << std::endl;
 //		}
 //
 //		// Optional: Break the loop if you receive a specific message
 //		if (strcmp((char*)buffer, "exit") == 0) {
 //			break;
 //		}
 //	}
 //
 //	// Clean up
 //	closesocket(udpSocket);
 //	WSACleanup();
 //
 //	return 0;
 //}