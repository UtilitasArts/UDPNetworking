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
	WaitingForPlayers();

}
void ConnectedToSession_NetClientState::OnRecv()
{
}
void ConnectedToSession_NetClientState::OnExit() {
	BaseNetState_Client::OnExit();
}

void ConnectedToSession_NetClientState::WaitingForPlayers() {
		while (bActive) {
			UDPPacks::RecvBytes(true);

			if (!bSessionStarted)
			{
				if (UDPPacks::ReceiveAdress == UDPPacks::ServerAdress && !bSessionStarted) {
					switch (UDPPacks::RecvMT) {
					case MessageType::JoinNotify:
						ReturnAddresses(UDPPacks::RecvMT);
						break;
					case MessageType::SessionStart:
						ReturnAddresses(UDPPacks::RecvMT);
						SessionStart();
						break;					
					}
				}
			}
			else if (UDPPacks::RecvValidSessionAddress())	{
				switch (UDPPacks::RecvMT) {
				case MessageType::ConnectRequest:				
					RecvPlayerConnectRequest();
					break;
				}
			}
		}
}
void ConnectedToSession_NetClientState::ReturnAddresses(MessageType message_type) {
	uint8_t JoinedCount; uint8_t SessionSize;

	UDPPacks::RecvBytePack.ReturnBytes(JoinedCount,1);
	UDPPacks::RecvBytePack.ReturnBytes(SessionSize,2);

	std::cout << "\n- Players in session count = " << (int)JoinedCount << " out of " << (int)SessionSize << "\n";

	for (size_t i = 0; i < JoinedCount; i++) {
		uint32_t	PublicIP;
		uint16_t	PublicPort;
		std::string PublicName;

		size_t count = i * 3;
		UDPPacks::RecvBytePack.ReturnBytes(PublicIP,   3 + count);
		UDPPacks::RecvBytePack.ReturnBytes(PublicPort, 4 + count);
		UDPPacks::RecvBytePack.ReturnBytes(PublicName, 5 + count);

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

	Sleep(3000);

	UDPPacks::SendBytePack.Clear(20, 3);
	UDPPacks::SendBytePack.AddBytes(MessageType::ConnectRequest);
	UDPPacks::SendBytePack.AddBytes(UDPSetup::MyName);
	
	for (size_t i = 0; i < UDPPacks::ConnectedAddresses.size(); i++) {
		UDPPacks::SendBytes(UDPPacks::ConnectedAddresses[i], true);		
	}
}

void ConnectedToSession_NetClientState::RecvPlayerConnectRequest() {
	std::string PublicName;
	UDPPacks::RecvBytePack.ReturnBytes(PublicName,1);
	std::cout << "- " << PublicName << " says hi! \n";

}

