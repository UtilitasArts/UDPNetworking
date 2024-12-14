#pragma once
#include <filesystem>
#include <iostream>
#include "SessionStateMachine.h"
namespace fs = std::filesystem;

std::vector<SessionStateMachine*> Sessions;

void ConnectRequest(){

	std::cout << "Received: \n"; UDPPacks::ReceiveAdress.PrintAdress();

	std::string Name;
	UDPPacks::RecvBytePack.ReturnBytes(Name, 1, true);
	UDPPacks::SendBytePack.Clear(20,3);

	uint32_t RecvNetIp   = UDPPacks::ReceiveAdress.HostIP();
	uint16_t RecvNetPort = UDPPacks::ReceiveAdress.HostPort();
	bool     bConnectionApproved = true;
	uint8_t  AmountOfSessions = static_cast<uint8_t>(Sessions.size());

	UDPPacks::SendBytePack.AddBytes(MessageType::ConnectApproval);
	UDPPacks::SendBytePack.AddBytes(RecvNetIp);
	UDPPacks::SendBytePack.AddBytes(RecvNetPort);
	UDPPacks::SendBytePack.AddBytes(bConnectionApproved);
	UDPPacks::SendBytePack.AddBytes(AmountOfSessions);

	if (AmountOfSessions > 0)
	{
		for (size_t i = 0; i < Sessions.size(); i++)
		{
			std::string SessionName = Sessions[i]->SessionName;
			UDPPacks::SendBytePack.AddBytes(SessionName);
		}
	}
	std::cout << "Sending: \n";
	UDPPacks::SendBytes(UDPPacks::ReceiveAdress, true);
}

void CreateSession() {

	std::cout << "Received: \n"; UDPPacks::ReceiveAdress.PrintAdress();
 	std::string RoomID;

 	UDPPacks::RecvBytePack.ReturnBytes(RoomID, 1);
	std::cout << "- Created a room with ID:" << RoomID;

 	UDPPacks::SendBytePack.Clear(20, 3);  
 	UDPPacks::SendBytePack.AddBytes(MessageType::CreateApproval);
	UDPPacks::SendBytes(UDPPacks::ReceiveAdress);
}

void JoinSession() {
	// 	std::string Name;
	// 	RecvBytePack.ReturnBytes(Name, 1, true);
}

void UpdateServer() {
	std::cout << "- Request to update server: \n";	
	UDPPacks::SendBytePack.Clear(20, 3);
	UDPPacks::SendBytePack.AddBytes(MessageType::UpdateApproval);
	UDPPacks::SendBytes(UDPPacks::ReceiveAdress);

	closesocket(UDPSetup::UDPSocket);
	WSACleanup();

	std::string Command2 = "&& git pull && cd \"" + UDPSetup::RestartFolder.string() + "\" && start cmd /K \"UDPServer.exe\"";
	std::string Command  = "start cmd /C \" cd \"" + UDPSetup::ReposFolder.string()   + "\"" + Command2 + "\"";

	system(Command.c_str());

	std::cout << "- Update of server was approved, Restarting now";
	exit(0);
}


namespace CMD {

	template <typename T, typename... Args>
	std::string MultiCMD(T first, Args&... rest)
	{
		std::string CombinedCMD = first;
		((CombinedCMD += " && " + rest), ...);
		return CombinedCMD;
	}

	std::string Command(std::string command, std::string vars = "")
	{
		return command + vars;
	}

	std::string Terminal(std::string command, std::string vars = "/K")
	{
		return "start cmd " + vars + "\"" + command + "\"";
	}

	std::string SetPath(std::filesystem::path path) {
		return "cd \"" + path.string() + "\"";
	}

	std::string SetString(std::string string) {
		return "\"" + string + "\"";
	}
}

int main(){


  	UDPSetup::UDPInit(8000,"Server");

	fs::path Dir = "C:/";
	Dir = Dir / "users" / "Utili";

	std::string Exit		 = CMD::Command("exit");

	std::string RestartPath     = CMD::SetPath(UDPSetup::RestartFolder);
	std::string RestartSoftware = CMD::Command("cls && dir");
	std::string RestartTerminal = CMD::Terminal(RestartSoftware);

	std::string ReposPath		= CMD::SetPath(UDPSetup::ReposFolder);
	std::string GitStatus		= CMD::Command("git status");
	std::string GitAdd			= CMD::Command("git add .");
	std::string GitCommit		= CMD::Command("git commit -m", CMD::SetString("Test"));
	std::string GitPush			= CMD::Command("git push -u origin main");

	std::string GitCommands		= CMD::MultiCMD(GitStatus,GitAdd,GitCommit,GitPush);
	std::string GitTerminal		= CMD::Terminal(CMD::MultiCMD(ReposPath,GitCommands,RestartTerminal));

	std::string FinalCommand  = GitTerminal;

	system(FinalCommand.c_str());

	std::cout << GitTerminal << "\n";


// 
// 	std::cout << "\n - Waiting for Clients";
// 
// 	while (true) {
// 
// 		UDPPacks::RecvBytes(true);
// 
// 		switch (UDPPacks::RecvMT) {
// 		case MessageType::ConnectRequest:
// 			ConnectRequest();
// 			break;
// 		case MessageType::CreateRequest:
// 			CreateSession();
// 			break;
// 		case MessageType::JoinRequest:
// 			JoinSession();
// 			break;
// 		case MessageType::UpdateRequest:
// 			UpdateServer();
// 			break;
// 		}
// 	}


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