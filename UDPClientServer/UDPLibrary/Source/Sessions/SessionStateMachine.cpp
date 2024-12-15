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
	AdressArray.reserve(SessionSize);
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

bool SessionStateMachine::JoinSession(AdressCtr adress_ctr) {
	UDPPacks::SendBytePack.Clear(20, 3);
	UDPPacks::SendBytePack.AddBytes(MessageType::JoinApproval);

	if (SessionSize >= JoinedCount + 1) {
		JoinedCount++;
		AdressArray.push_back(adress_ctr);
		UDPPacks::SendBytePack.AddBytes(true);
		UDPPacks::SendBytePack.AddBytes(JoinedCount);		

		for (size_t i = 0; i < AdressArray.size(); i++)	{
			std::string NameInArray = AdressArray[i].GetAddrName();
			UDPPacks::SendBytePack.AddBytes(NameInArray);
		}
		
		UDPPacks::SendBytes(UDPPacks::ReceiveAdress);
		return true;
	}
	else {
		UDPPacks::SendBytePack.AddBytes(false);
		UDPPacks::SendBytes(UDPPacks::ReceiveAdress, true);
		return false;
	}
}

