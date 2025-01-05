#pragma once
#include "NetClientStates.h"
#include "NetClientStateMachine.h"

NetClientStateMachine::NetClientStateMachine() :
	StateArray
	(
		{
			new Unconnected_Client(this),
			new ConnectedToSession_NetClientState(this),
			new ConnectedToPlayers_NetClientState(this)
		}
	)
{
	SetState(ENetClientStates::Unconnected);
}

NetClientStateMachine::~NetClientStateMachine() {
	for (size_t i = 0; i < StateArray.size(); i++) {
		delete StateArray[i];
	}
}

void NetClientStateMachine::SetState(ENetClientStates NextState) {
	if (CurrentState) { CurrentState->OnExit(); }

	CurrentState = StateArray.at((size_t)NextState);

	if (CurrentState) { 
		CurrentState->OnEnter();	
	}
}

void NetClientStateMachine::OnTick() {
	if (CurrentState){
		CurrentState->OnTick();
	}
}




