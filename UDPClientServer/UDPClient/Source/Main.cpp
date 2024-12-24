#pragma once
#include <filesystem>
#include <iostream>
#include "NetClientStateMachine.h"

namespace fs = std::filesystem;

// 	BytePack SendPack(200,16);
// 	uint32_t A = 20;
// 	SendPack.AddBytes(A);
// 	SendPack.AddCRC();
// 	SendPack.PrintBytes();
// 	uint32_t AA;
// 	BytePack ReceivePack(200,16);
// 	ReceivePack.SetByteArray(SendPack.GetByteArray(),SendPack.GetArraySize(),true);
// 	ReceivePack.ReturnBytes(AA,0,true);

#define Message(x,y,z)  x.AddBytes(y); z x.AddCRC();

int main() {

	NetClientStateMachine NetStateMachine;

	Timer	 GlobalTimer;
	uint64_t Time			= 0;
	uint64_t Accumulator	= 0;
	uint64_t FrameCount		= 0;
	const int64_t FixedTimeStep = 16666667;
	const uint64_t NanoSecond	= 1000000000;

	//---------------------|
	// Fixed 60 FPS update |
	//=====================|
	while (true) {	
		Accumulator += GlobalTimer.GetDeltaTimeNS();		
		while (Accumulator >= FixedTimeStep) {	
			FrameCount++;Time += FixedTimeStep;	Accumulator -= FixedTimeStep;	

			//--------------|
			// Frame Update |
			//==============|
			NetStateMachine.OnTick();
		}	
	}
}

 
//      int main(int argc, char* argv[])
//      {
//      	int GG = 1;
//      
//      	GG = UDPLib::add(GG, 3);
//      
//      	std::cout << GG;
//      
//      
//      	char* ipchar = argv[1];
//      	char* portchar = argv[2];
//      
//      	uint16_t port;
//      	uint8_t a, b, c, d;
//      
//      	// Use sscanf_s to split the IP address into four integers
//      	if (sscanf_s(ipchar, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d) != 4)
//      	{
//      		printf("Invalid IP address format.\n");
//      	}
//      
//      	if (sscanf_s(portchar, "%hu", &port) != 1) {
//      		printf("Port not specified.\n");
//      	}
//      
//      	// Initialize Winsock
//      	WSADATA wsaData;
//      	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
//      		std::cerr << "Failed to initialize Winsock." << std::endl;
//      		return 1;
//      	}
//      
//      	// Create a socket
//      	SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//      	if (udpSocket == INVALID_SOCKET) {
//      		std::cerr << "Failed to create socket." << std::endl;
//      		WSACleanup();
//      		return 1;
//      	}
//      
//      	uint32_t address = (a << 24) | (b << 16) | (c << 8) | d;
//      
//      	sockaddr_in addr;
//      	addr.sin_family = AF_INET;
//      	addr.sin_addr.s_addr = htonl(address);
//      	addr.sin_port = htons(port);
//      
//      
//      	char TestAddr[INET_ADDRSTRLEN];
//      	InetNtopA(AF_INET, &(addr.sin_addr), TestAddr, INET_ADDRSTRLEN);
//      	const unsigned short Testport = ntohs(addr.sin_port);
//      
//      	std::cout << "\n" << TestAddr << ":" << Testport << "\n";
//      
//      	// Prepare the message to send
//      	const char* message = argv[3];
//      
//      	// Send the message
//      	int bytesSent = sendto(udpSocket, message, (int)strlen(message), 0, (sockaddr*)&addr, sizeof(addr));
//      	if (bytesSent == SOCKET_ERROR) {
//      		std::cerr << "Failed to send message." << std::endl;
//      	}
//      	else {
//      		std::cout << "Message sent: " << message << std::endl;
//      	}
//      
// 
// 
//      	// Receive messages
//      	while (true) {
//      
//      		unsigned char buffer[1024];
//      		sockaddr_in senderAddress;
//      		senderAddress.sin_family = AF_INET;
//      		int senderAddressSize = sizeof(senderAddress);
//      
//      		std::cout << "\nListening: \n\n";
//      
//      		int bytesReceived = recvfrom(udpSocket, (char*)buffer, sizeof(buffer), 0, (sockaddr*)&senderAddress, &senderAddressSize); // halts the loop because the socket is prolly set to blocking
//      		if (bytesReceived == SOCKET_ERROR) {
//      			std::cerr << "Failed to receive message." << std::endl;
//      			break; // Exit the loop on error
//      		}
//      
//      		char ipAddr[INET_ADDRSTRLEN];
//      		InetNtopA(AF_INET, &(senderAddress.sin_addr), ipAddr, INET_ADDRSTRLEN);
//      		const unsigned short sendport = ntohs(senderAddress.sin_port);
//      
//      		// Null-terminate the received data and print it
//      		buffer[bytesReceived] = '\0'; // Ensure null termination for printing
//      		std::cout << "from" << ipAddr << ":" << sendport << "\n" << "Received message: " << buffer << std::endl;
//      
//      		// Optional: Break the loop if you receive a specific message
//      		if (strcmp((char*)buffer, "exit") == 0) {
//      			break;
//      		}
//      	}
//      
//      	// Clean up
//      	closesocket(udpSocket);
//      	WSACleanup();
//      
//      	return 0;
//      }
//      
