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
	SessionAdresses.reserve(SessionSize);
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

	UDPPacks::SendBytePack.Clear(20, 3);
	UDPPacks::SendBytePack.AddBytes(MessageType::JoinApproval);

	if (SessionSize >= JoinedCount + 1 && IsNewConnection()) {
		JoinedCount++;
		SessionAdresses.push_back(UDPPacks::ReceiveAdress);
		UDPPacks::ConnectedAdresses.push_back(UDPPacks::ReceiveAdress);

		UDPPacks::SendBytePack.AddBytes(true);		
		UDPPacks::SendBytes(UDPPacks::ReceiveAdress, true);

		NotifyAllOnJoin();

		return true;
	}
	else {
		UDPPacks::SendBytePack.AddBytes(false);
		UDPPacks::SendBytes(UDPPacks::ReceiveAdress, true);
		return false;
	}
}

bool SessionStateMachine::IsNewConnection()
{
	for (size_t i = 0; i < UDPPacks::ConnectedAdresses.size(); i++)
	{
		if (UDPPacks::ReceiveAdress == UDPPacks::ConnectedAdresses[i]) {
			return false;
		}
	}
	return true;
}

void SessionStateMachine::NotifyAllOnJoin()
{
	UDPPacks::SendBytePack.Clear(30, 10);
	UDPPacks::SendBytePack.AddBytes(MessageType::JoinNotify);

	for (size_t i = 0; i < SessionAdresses.size(); i++) {
		std::string NameInArray = SessionAdresses[i].GetAddrName();
		uint32_t PublicIP		= SessionAdresses[i].HostIP();
		uint16_t PublicPort		= SessionAdresses[i].HostPort();

		SessionAdresses[i].PrintAdress();

		UDPPacks::SendBytePack.AddBytes(NameInArray);
		UDPPacks::SendBytePack.AddBytes(PublicIP);
		UDPPacks::SendBytePack.AddBytes(PublicPort);
	}

	UDPPacks::SendBytes(UDPPacks::ReceiveAdress, true);
}

