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
 				ReturnAddresses(UDPPacks::RecvMT);
 				break;
 			case MessageType::SessionStart:
 				std::cout << "STARTED!!!";
 				ReturnAddresses(UDPPacks::RecvMT);
 				SessionStart();
 				break;
 			}
 		}
 	}
 	else if (UDPPacks::RecvValidSessionAddress()) {
 		switch (UDPPacks::RecvMT) {
 		case MessageType::ConnectRequest:
 			RecvPlayerConnectRequest();
 			break;
 		}
 	}
}

void ConnectedToSession_NetClientState::ReturnAddresses(MessageType message_type) {
	uint8_t JoinedCount; uint8_t SessionSize;

	UDPPacks::RecvBytePack.ReturnBytes(JoinedCount,3);
	UDPPacks::RecvBytePack.ReturnBytes(SessionSize,4);

	std::cout << "\n- Players in session count = " << (int)JoinedCount << " out of " << (int)SessionSize << "\n";

	for (size_t i = 0; i < JoinedCount; i++) {
		uint32_t	PublicIP;
		uint16_t	PublicPort;
		std::string PublicName;

		size_t count = i * 3;
		UDPPacks::RecvBytePack.ReturnBytes(PublicIP,   5 + count);
		UDPPacks::RecvBytePack.ReturnBytes(PublicPort, 6 + count);
		UDPPacks::RecvBytePack.ReturnBytes(PublicName, 7 + count);

		AddrCtr CurSessionAddress(PublicIP, PublicPort, PublicName);
		CurSessionAddress.PrintAdress();

		if (message_type == MessageType::SessionStart) {
			if (CurSessionAddress != UDPPacks::MyPublicAdress){
				UDPPacks::ConnectedAddresses.push_back(CurSessionAddress);
			}
		}
	}
}

void ConnectedToSession_NetClientState::SessionStart() {
	std::cout <<"- Session is being started -\n";

	bSessionStarted = true;

	UDPPacks::CreateEchoMessage(MessageType::ConnectRequest, MessageType::EchoRequest, UDPPacks::SendID,
		UDPSetup::MyName);

	
	for (size_t i = 0; i < UDPPacks::ConnectedAddresses.size(); i++) {

		UDPPacks::SendBytes(UDPPacks::ConnectedAddresses[i], true);		
	}
}

void ConnectedToSession_NetClientState::RecvPlayerConnectRequest() {
	std::string PublicName;
	UDPPacks::RecvBytePack.ReturnBytes(PublicName,3);
	std::cout << "- " << PublicName << " says hi! \n";
}

