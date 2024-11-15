#include "UDPLibrary.h"
#include <iostream>

int main(int argc, char* argv[])
{
	char* portchar = argv[1];

	unsigned short port;


	if (sscanf_s(portchar, "%hu", &port) != 1) {
		printf("Port not specified.\n");
	}

	// Initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "Failed to initialize Winsock." << std::endl;
		return 1;
	}

	// Create a socket
	SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket == INVALID_SOCKET) {
		std::cerr << "Failed to create socket." << std::endl;
		WSACleanup();
		return 1;
	}

	// Prepare the local address structure
	sockaddr_in localAddress;
	localAddress.sin_family = AF_INET;
	localAddress.sin_port = htons(port); // The same port as the sender
	localAddress.sin_addr.s_addr = INADDR_ANY; // Accept messages from any IP
	//localAddress.sin_addr.s_addr = htonl(address);

	if (bind(udpSocket, (sockaddr*)&localAddress, sizeof(localAddress)) == SOCKET_ERROR) {
		std::cerr << "Failed to bind socket." << std::endl;
		closesocket(udpSocket);
		WSACleanup();
		return 1;
	}

	int addrlen = sizeof(localAddress);
	if (getsockname(udpSocket, (sockaddr*)&localAddress, &addrlen) == SOCKET_ERROR) {
		std::cerr << "Failed to get socket name." << std::endl;
		closesocket(udpSocket);
		WSACleanup();
		return 1;
	}

	// Print the public port
	const unsigned short publicPort = ntohs(localAddress.sin_port);
	char ipAddr[INET_ADDRSTRLEN];
	InetNtopA(AF_INET, &(localAddress.sin_addr), ipAddr, INET_ADDRSTRLEN);

	std::cout << "Listening on port: " << ipAddr << ":" << publicPort << "\n\n";


	// Receive messages
	while (true) {
		unsigned char buffer[1024];
		sockaddr_in senderAddress;
		senderAddress.sin_family = AF_INET;
		int senderAddressSize = sizeof(senderAddress);

		int bytesReceived = recvfrom(udpSocket, (char*)buffer, sizeof(buffer), 0, (sockaddr*)&senderAddress, &senderAddressSize);
		if (bytesReceived == SOCKET_ERROR) {
			std::cerr << "Failed to receive message." << std::endl;
			break; // Exit the loop on error
		}

		char ipAddr[INET_ADDRSTRLEN];
		InetNtopA(AF_INET, &(senderAddress.sin_addr), ipAddr, INET_ADDRSTRLEN);
		const unsigned short sendport = ntohs(senderAddress.sin_port);

		// Null-terminate the received data and print it
		buffer[bytesReceived] = '\0'; // Ensure null termination for printing
		std::cout << "from" << ipAddr << ":" << sendport << "\n" << "Received message New: " << buffer << std::endl;

		// Prepare the message to send

		char TempMessage[100];
		snprintf(TempMessage, sizeof(TempMessage), "Your Public IP Address =: %s, your Public Port = : %hu", ipAddr, sendport);
		const char* message = TempMessage;

		// Send the message
		int bytesSent = sendto(udpSocket, message, (int)strlen(message), 0, (sockaddr*)&senderAddress, sizeof(senderAddress));
		if (bytesSent == SOCKET_ERROR)
		{
			std::cerr << "Failed to send message." << std::endl;
		}
		else
		{
			std::cout << "Message sent: " << message << std::endl;
		}

		// Optional: Break the loop if you receive a specific message
		if (strcmp((char*)buffer, "exit") == 0) {
			break;
		}
	}

	// Clean up
	closesocket(udpSocket);
	WSACleanup();

	return 0;
}