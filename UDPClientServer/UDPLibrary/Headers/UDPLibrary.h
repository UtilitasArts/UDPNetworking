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
#include <unordered_map>
#include <unordered_set>
#pragma comment( lib, "wsock32.lib" )
#pragma comment( lib, "Ws2_32.lib" )
//-------------|
// My Includes |
//=============|
#include "BytePacker.h"
#include "AddressCtr.h"
#include "UDPMessageEnums.h"
#include "Timer.h"

struct AddrChamber {
	AddrChamber(AddrCtr& public_address) : PublicAddress(public_address) {}
	AddrChamber(AddrCtr& public_address, AddrCtr& local_address) : PublicAddress(public_address), LocalAddress(local_address) {}

	AddrCtr PublicAddress;
	AddrCtr LocalAddress;
};

//--------------|
// Echo Chamber |
//==============|
struct EchoChamber {
	EchoChamber(AddrCtr& addr_ctr, BytePack& resend_pack, std::chrono::steady_clock::time_point send_time) : AdressContainer(addr_ctr), ResendBytePack(resend_pack), SendTime(send_time) {}
	AddrCtr  AdressContainer;
	BytePack ResendBytePack;
	std::chrono::steady_clock::time_point SendTime;	

	EchoChamber& operator=(const EchoChamber& other) {
		return *this;
	}
};

//--------------------------|
// Message ID to check maps |
//==========================|
struct MessageID {
	MessageID(AddrCtr addr_ctr, uint32_t recv_id) 
	: IP(addr_ctr.HostIP()), Port(addr_ctr.HostPort()), RecvID(recv_id) {}
	uint32_t IP;
	uint16_t Port;
	uint32_t RecvID;

	MessageID& operator=(const MessageID& other) {
		if (this != &other) { // Prevent self-assignment
			IP = other.IP;
			Port = other.Port;
			RecvID = other.RecvID;
		}
		return *this;
	}

	bool operator==(const MessageID& other) const {
		return IP == other.IP && Port == other.Port && RecvID == other.RecvID;
	}
};

inline size_t hash_combine(size_t lhs, size_t rhs) {
	if constexpr (sizeof(size_t) >= 8) {
		lhs ^= rhs + 0x517cc1b727220a95 + (lhs << 6) + (lhs >> 2);
	}
	else {
		lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
	}
	return lhs;
}

struct MessageIDHash {
 	size_t operator()(const MessageID& entry) const {
		size_t seed = 0;
		size_t IPHash = std::hash<uint32_t>()(entry.IP);
		size_t PortHash = std::hash<uint16_t>()(entry.Port);
		size_t RecvIDHash = std::hash<uint32_t>()(entry.RecvID);

		seed = hash_combine(seed,IPHash);
		seed = hash_combine(seed,PortHash);
		seed = hash_combine(seed,RecvIDHash);
 		return seed;
 	}
};

struct AddrCtrHash {
	size_t operator()(const AddrCtr& entry) const {
		size_t seed = 0;

 		size_t IPHash     = std::hash<uint32_t>()(entry.HostIP());
 		size_t PortHash   = std::hash<uint16_t>()(entry.HostPort());
  
  		seed = hash_combine(seed, IPHash);
  		seed = hash_combine(seed, PortHash);
 		return seed;
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
	inline AddrCtr		LocalAddress;

	inline uint32_t		RecvBufferBytes;
	inline uint32_t		PrevRecvBufferBytes;

	inline std::filesystem::path RestartFolder;
	inline std::filesystem::path ReposFolder;
	//------------------|
	// Public Functions |
	//==================|
	void InstallFolders();
	void InitWinsock();
	void OpenUDPSocket();
	void BindSocket(uint16_t port = 0);
	void GetLocalAddr(uint16_t port);
	void UDPInit(uint16_t port = 0);

	bool SocketHasNewBytes();
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
	inline MessageType	 RecvEcho;
	inline uint32_t		 RecvID;
	inline uint32_t		 SendID;	

 	inline std::unordered_set<MessageID, MessageIDHash>				 BlockMap;
	inline std::unordered_map<MessageID, EchoChamber, MessageIDHash> EchoMap;
	inline std::unordered_map<MessageID, EchoChamber, MessageIDHash> TempEchoMap;
	inline std::unordered_map<AddrCtr,   AddrChamber, AddrCtrHash>   AddrMap;


	//------------------|
	// Public Functions |
	//==================|
	MessageType RecvBytes(bool bPrint, bool bIsServer = false);
	bool RecvEchoResponse(bool bPrint);
	bool RecvEchoRequest(bool bPrint);
	void SendBytes(AddrCtr& adress_ctr, bool bPrint = false, BytePack send_bytes = UDPPacks::SendBytePack);

	inline Timer MessageTimer;
	inline Timer EchoTimer;
	void SendEchoes(bool bPrint = false);

	//----------------|
	// Create Message |
	//================|
	template<typename T, typename... Types>
	void AddMessageData(T CurrentType, Types... args) {
		SendBytePack.AddBytes(CurrentType);
		if constexpr (sizeof...(args) > 0) {
			AddMessageData(args...);
		}
	}
	template<typename... Types>
	void CreateEchoMessage(MessageType message_type, MessageType echo_type, uint32_t& send_id, Types... args) {
		SendBytePack.Clear();
		SendBytePack.AddBytes(message_type);
		SendBytePack.AddBytes(echo_type);
		SendBytePack.AddBytes(send_id);

		if constexpr (sizeof...(args) > 0) {
			AddMessageData(args...);
		}
	}

	template<typename... Types>
	void CreateMessage(MessageType message_type, Types... args) {
		SendBytePack.Clear();
		SendBytePack.AddBytes(message_type);
		SendBytePack.AddBytes(MessageType::None);
		SendBytePack.AddBytes(SendID);

		if constexpr (sizeof...(args) > 0) {
			AddMessageData(args...);
		}
	}
}



