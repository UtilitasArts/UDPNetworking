#pragma once
#include "SessionStates.h"
#include "SessionStateMachine.h"

SessionStateMachine::SessionStateMachine(uint8_t session_size) :
AdressArray( new AdressCtr[session_size]),
StateArray
(
	{
		new Initialize_SessionState(this),
		new SessionInProgress_SessionState(this),
		new DestroyingSession_SessionState(this)
	}
),
	CurrentState(StateArray[0]),
	SendBytePack(200, 10),
	RecvBytePack(200, 10),
	Server(80, 61, 175, 45, 7000, "Server", false)
{ }

SessionStateMachine::~SessionStateMachine() {
	for (size_t i = 0; i < StateArray.size(); i++) {
		delete StateArray[i];		
	}
	delete[] AdressArray;
}

void SessionStateMachine::SetState(ESessionStates NextState) {
	if (CurrentState) {	CurrentState->OnExit();	}
	CurrentState = StateArray.at((size_t)NextState);
	if (CurrentState) {	CurrentState->OnEnter();}
}

