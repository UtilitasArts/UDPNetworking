#pragma once
#include "SessionStateMachine.h"
#include "SessionStates.h"

//BaseState
BaseSessionState::BaseSessionState(SessionStateMachine* sm) :
StateMachine(sm)
{}
void BaseSessionState::InitState(){
	UDPPacks::SendBytePack.Clear(200,20);
	UDPPacks::SendBytePack.Clear(200,20);
	StateEnum = ESessionStates::InitializeSession;
}
void BaseSessionState::OnEnter() {
	InitState();
	std::cout << "State=" << ESessionStateString(StateEnum) << "\n";
}
void BaseSessionState::OnExit(){
	//std::cout << "Exit State " << ENetServerStateString(StateEnum) << "\n";
}


//Initialize_Session
void Initialize_SessionState::InitState(){
	BaseSessionState::InitState();
	StateEnum = ESessionStates::InitializeSession;
}
void Initialize_SessionState::OnEnter() {
	BaseSessionState::OnEnter();

    UDPSetup::UDPInit();
	UDPPacks::SendBytePack.AddBytes(MessageType::ConnectRequest);
	UDPPacks::SendBytePack.AddBytes(UDPSetup::MyName);
 	UDPPacks::SendBytes(UDPPacks::ServerAdress, true);

	while (true)
	{
		UDPPacks::RecvBytes(true);

		switch (UDPPacks::RecvMT) {
		case MessageType::ConnectRequest:

			break;
		case MessageType::CreateRequest:

			break;
		case MessageType::JoinRequest:

			break;
		}
	}
}

void Initialize_SessionState::OnExit(){
	BaseSessionState::OnExit();
}

//SessionInProgress
void SessionInProgress_SessionState::InitState(){
	BaseSessionState::InitState();
	StateEnum = ESessionStates::SessionInProgress;
}
void SessionInProgress_SessionState::OnEnter(){
	BaseSessionState::OnEnter();
}
void SessionInProgress_SessionState::OnExit(){
	BaseSessionState::OnExit();
}


//SessionInProgress
void DestroyingSession_SessionState::InitState(){
	BaseSessionState::InitState();
	StateEnum = ESessionStates::DestroyingSession;
}

void DestroyingSession_SessionState::OnEnter()
{
}

void DestroyingSession_SessionState::OnExit()
{
}