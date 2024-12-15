#pragma once
#include "SessionStates.h"
#include "SessionStateMachine.h"

SessionStateMachine::SessionStateMachine(std::string session_name, uint8_t session_size) :
SessionName(session_name),
AdressArray( new AdressCtr[session_size]),
StateArray
(
	{
		new Initialize_SessionState(this),
		new SessionInProgress_SessionState(this),
		new DestroyingSession_SessionState(this)
	}
),
	CurrentState(StateArray[0])
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

