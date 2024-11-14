#pragma once

#include <iostream>
#include <vector>

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
struct Package {
	bool	 a;			    //  1 byte
	int8_t   b;             //  1 byte
	int16_t  c;             //  2 bytes
	int32_t  d;             //  4 bytes
	int64_t  e;             //  8 bytes
	uint8_t  f;             //  1 byte
	uint16_t g;             //  2 bytes
	uint32_t h;             //  4 bytes
	uint64_t i;             //  8 bytes

	float	 j;             //  4 bytes
	double   k;			    //  8 bytes

	char l[255];			//255 bytes
	std::string m;          // variable length (size-prefixed)
	std::vector<int32_t> n; // variable length (size-prefixed)	
};
#pragma pack(pop)



