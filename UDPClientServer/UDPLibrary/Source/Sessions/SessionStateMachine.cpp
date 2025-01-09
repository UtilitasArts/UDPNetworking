#pragma once
#include "SessionStates.h"
#include "SessionStateMachine.h"

SessionStateMachine::SessionStateMachine(std::string session_name, uint8_t session_size) :
SessionName(session_name),
SessionSize(session_size),
JoinedCount(0),
StateArray
(
	{
		new Initialize_SessionState(this),
		new SessionInProgress_SessionState(this),
		new DestroyingSession_SessionState(this)
	}
)
{ 
	SetState(ESessionStates::InitializeSession);
}

SessionStateMachine::~SessionStateMachine() {
	for (size_t i = 0; i < StateArray.size(); i++) {
		delete StateArray[i];		
	}
}

void SessionStateMachine::SetState(ESessionStates NextState) {
	if (CurrentState) {	CurrentState->OnExit();	}
	CurrentState      = StateArray.at((size_t)NextState);
	CurrentStateEnum  = NextState;
	if (CurrentState) {	CurrentState->OnEnter();}
}

 bool SessionStateMachine::JoinSession() {

	 UDPPacks::CreateEchoMessage(MessageType::JoinApproval, MessageType::EchoRequest, UDPPacks::SendID);

 	if (SessionSize >= JoinedCount + 1) {
 		JoinedCount++;

        AddrChamber TempAddrChamber(UDPPacks::ReceiveAdress, UDPPacks::AddrMap.at(UDPPacks::ReceiveAdress).LocalAddress);

        SessionAddrMap.emplace(UDPPacks::ReceiveAdress, TempAddrChamber);
 
        UDPPacks::AddMessageData(true); 	
 		UDPPacks::SendBytes(UDPPacks::ReceiveAdress, true);
 
 		if (JoinedCount == SessionSize)	{
 			SendSessionAdresses(MessageType::SessionStart);
 		}
 		else {
 			SendSessionAdresses(MessageType::JoinNotify);
 		}
 
 		return true;
 	}
 	else {
        UDPPacks::AddMessageData(false);
 		UDPPacks::SendBytes(UDPPacks::ReceiveAdress, true);
 		return false; 
 	}
 }
 
 bool SessionStateMachine::IsNewConnection() {
 	for (size_t i = 0; i < UDPPacks::ConnectedAddresses.size(); i++)
 	{
 		if (UDPPacks::ReceiveAdress == UDPPacks::ConnectedAddresses[i]) {
 			return false;
 		}
 	}
 	return true;
 }
 
 void SessionStateMachine::SendSessionAdresses(MessageType message_type)
 {
 	std::cout << "\n- Players in session count = " << (int)JoinedCount << " out of " << (int)SessionSize << "\n";

	UDPPacks::CreateEchoMessage(message_type, MessageType::EchoRequest, UDPPacks::SendID,                                
                                JoinedCount,
                                SessionSize);  	

    for (auto Chamber = SessionAddrMap.begin(); Chamber != SessionAddrMap.end(); Chamber++ ) {
  
        uint32_t    LocalIP    = Chamber->second.LocalAddress.HostIP();
 		uint32_t	PublicIP   = Chamber->second.PublicAddress.HostIP();
 		uint16_t	PublicPort = Chamber->second.PublicAddress.HostPort();
 		std::string PublicName = Chamber->second.PublicAddress.GetAddrName();
 
        Chamber->second.PublicAddress.PrintAdress();
        Chamber->second.LocalAddress.PrintAdress();
        UDPPacks::AddMessageData(LocalIP,PublicIP,PublicPort,PublicName);
 	}

	for (auto Chamber = SessionAddrMap.begin(); Chamber != SessionAddrMap.end(); Chamber++ ) {

        UDPPacks::SendBytes(Chamber->second.PublicAddress, true);
	}

 }

