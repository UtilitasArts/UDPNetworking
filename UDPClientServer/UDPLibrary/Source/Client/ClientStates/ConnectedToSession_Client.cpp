#include <filesystem>
#include "CMDCommands.h"
#include "SessionStateEnums.h"
#include "NetClientStates.h"
#include "NetClientStateMachine.h"

namespace fs = std::filesystem;

void ConnectedToSession_NetClientState::InitState(){
	BaseNetState_Client::InitState();
	StateEnum = ENetClientStates::ConnectedToSession;
}
void ConnectedToSession_NetClientState::OnEnter() {
	BaseNetState_Client::OnEnter();

	std::cout << "- Waiting For Players.. \n";
	bSessionStarted = false;
}
void ConnectedToSession_NetClientState::OnRecv()
{
 	if (!bSessionStarted) {
 		if (UDPPacks::ReceiveAdress == UDPPacks::ServerAdress && !bSessionStarted) {
 			switch (UDPPacks::RecvMT) {
 			case MessageType::JoinNotify:
 				ReturnAddresses();
 				break;
 			case MessageType::SessionStart:
 				ReturnAddresses();
 				SessionStart();
 				break;
 			}
 		}
 	}
 	else {
 		switch (UDPPacks::RecvMT) {
 		case MessageType::ConnectRequest:
 			RecvPlayerConnectRequest();
 			break;
 		}
 	}
}

void ConnectedToSession_NetClientState::ReturnAddresses() {
	uint8_t JoinedCount; uint8_t SessionSize;

	UDPPacks::RecvBytePack.ReturnBytes(JoinedCount,3);
	UDPPacks::RecvBytePack.ReturnBytes(SessionSize,4);

	std::cout << "\n- Players in session count = " << (int)JoinedCount << " out of " << (int)SessionSize << "\n";

	for (size_t i = 0; i < JoinedCount; i++) {
		uint32_t	LocalIP;
		uint32_t	PublicIP;
		uint16_t	PublicPort;
		std::string PublicName;

		size_t count = i * 4;
		UDPPacks::RecvBytePack.ReturnBytes(LocalIP,    5 + count);
		UDPPacks::RecvBytePack.ReturnBytes(PublicIP,   6 + count);
		UDPPacks::RecvBytePack.ReturnBytes(PublicPort, 7 + count);
		UDPPacks::RecvBytePack.ReturnBytes(PublicName, 8 + count);

		AddrCtr CurPublicSessionAddress(PublicIP, PublicPort, PublicName);
		AddrCtr CurLocalSessionAddress(LocalIP  , PublicPort, PublicName);

		if (CurPublicSessionAddress != UDPPacks::MyPublicAdress) {
			if (PublicIP == UDPPacks::MyPublicAdress.HostIP()) {
				UDPPacks::AddrMap.emplace(CurLocalSessionAddress, AddrChamber(CurLocalSessionAddress, CurLocalSessionAddress));
			}
			else{				
				UDPPacks::AddrMap.emplace(CurPublicSessionAddress, AddrChamber(CurPublicSessionAddress, CurLocalSessionAddress));
			}
		}
	}
}


void ConnectedToSession_NetClientState::SessionStart() {
	std::cout <<"- Session is being started -\n";

	bSessionStarted = true;
	UDPPacks::CreateEchoMessage(MessageType::ConnectRequest, MessageType::EchoRequest, UDPPacks::SendID);

	for (auto Chamber = UDPPacks::AddrMap.begin(); Chamber != UDPPacks::AddrMap.end(); Chamber++) {

		std::cout << "Attempt to send to ";
		Chamber->second.PublicAddress.PrintAdress();

		if (Chamber->second.PublicAddress.HostIP() == UDPPacks::MyPublicAdress.HostIP()) {
			std::cout << "- Local Connection Detected \n";
			UDPPacks::SendBytes(Chamber->second.LocalAddress, true);
		}
		else{
			UDPPacks::SendBytes(Chamber->second.PublicAddress, true);
		}
	}
}

void ConnectedToSession_NetClientState::RecvPlayerConnectRequest() {
	std::cout << "- " << UDPPacks::ReceiveAdress.GetAddrName() << " says hi! \n";
}

