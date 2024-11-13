#pragma once

#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment( lib, "wsock32.lib" )
#pragma comment( lib, "Ws2_32.lib" )

class UDPLib {
public:
	
	static int add(int a, int b) {
		return a + b ;
	}

};

#pragma pack(push, 1)
struct UDPPackage {

	uint32_t port;
	uint32_t address;
};
#pragma pack(pop)

